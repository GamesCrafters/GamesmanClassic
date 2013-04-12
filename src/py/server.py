#!/usr/bin/env python2.7
from __future__ import print_function

import BaseHTTPServer
import Queue
import asynchat
import asyncore
import cStringIO
import collections
import fcntl
import imp
import json
import logging
import logging.handlers
import os
import os.path
import socket
import subprocess
import sys
import threading
import time
import urllib
import urlparse

import game
Game = game.Game

bytes_per_mb = 1024 ** 2

max_log_size = 10 * bytes_per_mb

port = 8081
root_game_directory = './bin/'
log_filename = 'server.log'

game_script_directory = './src/py/games/'

indent_response = True
close_on_timeout = False

# Seconds to wait for a response from the process before sending timeout_msg
subprocess_reponse_timeout = 0.2  # Seconds

# Number of seconds to wait without a request before shutting down
# process
subprocess_idle_timeout = 600  # Seconds

log_to_file = True
log_to_stdout = True
log_to_stderr = False

could_not_parse_msg = '{"status":"error", "reason":"Could not parse request."}'
could_not_start_msg = '{"status":"error", "reason":"Could not start game."}'
timeout_msg = '{"status":"error", "reason":"Subprocess timed out."}'
crash_msg = '{"status":"error", "reason":"Subprocess crashed."}'
closed_msg = '{"status":"error", "reason":"Subprocess was closed."}'


class GameRequestHandler(asynchat.async_chat,
                         BaseHTTPServer.BaseHTTPRequestHandler):

    def __init__(self, sock, address, server):
        self.client_address = address
        self.connection = sock
        asynchat.async_chat.__init__(self, sock=sock)
        self.server = server

        self.set_terminator('\r\n\r\n')

        self.in_buffer = []

    def collect_incoming_data(self, data):
        self.in_buffer.append(data)

    def found_terminator(self):
        self.rfile = cStringIO.StringIO(''.join(self.in_buffer))
        self.rfile.seek(0)
        self.wfile = cStringIO.StringIO()
        self.raw_requestline = self.rfile.readline()
        self.parse_request()
        if self.command == 'GET':
            self.do_GET()

    def do_GET(self):
        unquoted = urllib.unquote(self.path)
        parsed = urlparse.urlparse(unquoted)
        self.parsed_path = parsed
        path = parsed.path.split('/')
        command = path[-1]
        if command == 'favicon.ico':
            return
        game_name = path[-2]

        # Can't use urlparse.parse_qs because of equal signs in board string
        query = collections.defaultdict(str)
        query_lst = parsed.query.split('&')
        for t in query_lst:
            if t != '':
                p = t.split('=', 1)
                if len(p) == 2:
                    query[p[0]] = p[1]
                else:
                    query[p[0]] = ''

        self.server.log.info('GET: {}'.format(unquoted))

        game = self.server.get_game(game_name)
        try:
            c_command = {
                'getStart':
                'start_response',

                'getNextMoveValues':
                'next_move_values_response {}'.format(query['board']),

                'getMoveValue':
                'move_value_response {}'.format(query['board'])
            }[command]
        except KeyError:
            self.respond(could_not_parse_msg)
        else:
            game.push_request(GameRequest(self, query, c_command))

    def respond(self, response):
        try:
            self.server.lock.acquire()
            self.send_header('Content-Length', len(response))
            self.send_header('Content-Type', 'text/plain')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.push(response)
            self.close()
        except Exception:
            pass
        finally:
            self.server.lock.release()
        self.server.log.info('Sent response: {}'.format(response))


class GameRequest(object):

    def __init__(self, handler, query, command):
        self.handler = handler
        self.query = query
        self.command = command

    def respond(self, response):
        self.handler.respond(response)


class GameProcess(object):

    def __init__(self, server, game, bin_path, option_num=None):
        self.server = server
        self.game = game
        self.queue = Queue.Queue()
        self.option_num = option_num
        self.req_timeout = subprocess_idle_timeout
        self.read_timeout = subprocess_reponse_timeout

        # Note that arguments to GamesmanClassic must be given in the right
        # order (this one, to be precise).
        arg_list = [bin_path]
        if option_num is not None:
            arg_list.append('--option')
            arg_list.append(str(option_num))
        arg_list.append('--interact')

        # Open a subprocess, connecting all of its file descriptors to pipes,
        # and set it to line buffer mode.
        self.process = subprocess.Popen(arg_list, stdin=subprocess.PIPE,
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.STDOUT, 
                                        bufsize=1,
                                        close_fds=True)
        self.setup_subprocess_pipe(self.process.stdout)

        self.timeout_msg = timeout_msg
        self.crash_msg = crash_msg
        self.closed_msg = closed_msg

        self.thread = threading.Thread(target=self.request_loop)
        self.thread.daemon = True
        self.thread.start()

    def setup_subprocess_pipe(self, pipe):
        descriptor = pipe.fileno()
        flags = fcntl.fcntl(descriptor, fcntl.F_GETFL)
        fcntl.fcntl(descriptor, fcntl.F_SETFL, flags | os.O_NONBLOCK)

    def push_request(self, request):
        self.queue.put(request)

    @property
    def alive(self):
        return self.thread.is_alive()

    def close(self):
        while not self.queue.empty():
            self.queue.get().respond(self.closed_msg)
        self.server.log.info('Closing {}.'.format(self.game.name))
        self.game.remove_process(self)
        try:
            self.process.terminate()
            time.sleep(1)
            self.process.kill()
        except OSError as e:
            pass

    def parse_response(self, response):
        if 'result' not in response:
            return None
        lines = response.split('\n')
        for line in lines:
            if line.startswith('result'):
                try:
                    result = line.split('=>>')[1].strip()
                    parsed = json.loads(result)
                except Exception as e:
                    # Catches problems with split, indexing, and non json
                    # together
                    return None
                formatted = json.dumps(parsed, indent=indent_response)
                return formatted

    def handle_timeout(self, request, response):
        # Did not receive a response from the subprocess, so
        # exit and kill it.
        self.server.log.error('Did not receive result '
                              'from {}!'.format(self.game.name))
        self.server.log.error('Process sent output:\n'
                              '{}'.format(response))
        if self.process.poll() is not None:
            # If the process is no longer running
            self.server.log.error('{} crashed!'.format(self.game.name))
            request.respond(self.crash_msg)
            return False
        else:
            self.server.log.error('{} timed out!'.format(self.game.name))
            request.respond(self.timeout_msg)
            return not close_on_timeout

    def wait_for_ready(self, timeout):
        last_output = ''
        read_timeout = 0.00001
        total_time = read_timeout
        while total_time < timeout:
            total_time += read_timeout
            time.sleep(read_timeout)
            try:
                last_output = self.process.stdout.read()
            except IOError:
                # Probably no data to read.
                # Subprocess could have could have crashed, but we'll timeout
                # anyways.
                continue
            self.server.log.debug('{} read:\n'
                                  '{}'.format(self.game.name, last_output))
            if ' ready =>>' in last_output:
                self.server.log.debug('Found ready prompt in {} '
                                      'seconds.'.format(total_time))
                return timeout - total_time
        self.server.log.error('Could not find ready prompt for '
                '{}'.format(self.game.name))
        return timeout - total_time

    def handle(self, request):
        time_remaining = self.wait_for_ready(self.read_timeout)
        if time_remaining <= 0:
            return self.handle_timeout(request, '')
        try:
            self.server.log.debug('Sent command {}'.format(request.command))
            self.process.stdin.write(request.command + '\n')
        except IOError as e:
            # This case can be hit if the subprocess crashes between requests
            # (known to occasionally happen).
            self.server.log.error('{} crashed on write!'.format(self.game.name))
            request.respond(self.crash_msg)
            return False
        response = ''
        timeout = 0.01
        parsed = None
        while not parsed:
            if time_remaining <= 0:
                self.server.log.debug('timeout')
                return self.handle_timeout(request, response)
            time_remaining -= timeout
            time.sleep(timeout)
            try:
                to_add = self.process.stdout.readline()
            except IOError:
                continue
            self.server.log.debug('subprocess sent output ' + to_add)
            response += to_add
            parsed = self.parse_response(response)
        request.respond(parsed)
        return True

    def request_loop(self):
        live = True
        while live:
            try:
                request = self.queue.get(block=True, timeout=self.req_timeout)
            except Queue.Empty as e:
                self.server.log.error(
                    '{} closed from lack of use.'.format(self.game.name))
                live = False
            else:
                live = self.handle(request)
        self.close()


class GameRequestServer(asyncore.dispatcher):

    could_not_start_msg = could_not_parse_msg
    root_game_directory = root_game_directory
    GameProcess = GameProcess

    def __init__(self, address, handler, log=logging.getLogger("server")):
        self.ip, self.port = address
        self.log = log
        self.handler = handler
        self.log.info('Starting server on port {}.'.format(self.port))
        self._game_table = {}
        asyncore.dispatcher.__init__(self)

        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)

        self.set_reuse_addr()

        self.bind(address)

        self.listen(4096)

        self.lock = threading.Lock()

    def get_game(self, name):
        try:
            return self._game_table[name]
        except KeyError:
            return self.load_game(name)

    def load_game(self, name):
        script_name = name + '.py'
        rel_path = os.path.join(game_script_directory, script_name)
        game_class = None
        game = None
        try:
            for s in os.listdir(game_script_directory):
                if s == script_name:
                    with open(rel_path) as script:
                        module = imp.load_module(name, script, '',
                                                 ('py', 'r', imp.PY_SOURCE))
                        game_class = module.__dict__[name]
                        self.log.debug('Loaded script for {}'.format(name))
        except Exception as e:
            self.log.error('Could not load script for {}'.format(name,
                                                                 e.message))
        if not game_class:
            self.log.debug('Could not find script for {}'.format(name))
            game_class = Game
        game = game_class(self, name)
        self._game_table[name] = game
        return game

    def serve_forever(self):
        asyncore.loop(0.01)

    def handle_accept(self):
        try:
            connection, address = self.accept()
        except socket.error:
            self.log.error('Server accept() failed.')
        else:
            self.handler(connection, address, self)


def get_log():
    log = logging.getLogger('server')

    if log_to_file:
        file_logger = \
            logging.handlers.RotatingFileHandler(log_filename,
                                                 maxBytes=max_log_size)
        file_logger.setLevel(logging.DEBUG)
        log.addHandler(file_logger)

    if log_to_stdout:
        stdout_logger = logging.StreamHandler(sys.stdout)
        stdout_logger.setLevel(logging.DEBUG)
        log.addHandler(stdout_logger)

    if log_to_stderr:
        stderr_logger = logging.StreamHandler(sys.stderr)
        stderr_logger.setLevel(logging.ERROR)
        log.addHandler(stderr_logger)

    log.setLevel(logging.DEBUG)
    return log


def main():
    httpd = GameRequestServer(('', port), GameRequestHandler, log=get_log())
    httpd.serve_forever()


if __name__ == '__main__':
    main()
