from __future__ import annotations
from collections import defaultdict
import fcntl
import json
import os
from queue import Queue
import queue
import subprocess
import threading
import time
import typing
import sys
import http.server
import logging
from logging.handlers import RotatingFileHandler
import urllib.parse
import game311

bytes_per_mb: int = 1024 ** 2

max_log_size: int = 10 * bytes_per_mb

max_memory_percent_per_process: float = 5.0

server_address: str = '' # Local
port: int = 8083
root_game_directory: str = "./bin/"
log_filename: str = "server.log"

game_script_directory: str = "./src/py/games"

close_on_timeout: bool = False

# Seconds to wait for a response from the process before sending timeout_msh
subprocess_response_timeout: int = 5

# Seconds to wait without a request before shutting down process
subprocess_idle_timeout: int = 600

log_to_file: bool = True
log_to_stdout: bool = True
log_to_stderr: bool = False
log_level: int = logging.DEBUG

could_not_parse_msg: str = ('{' +
                            '\n "status":"error",' +
                            '\n "reason":"Could not parse request."' +
                            '\n}')
could_not_start_msg: str = ('{' +
                            '\n "status":"error",' +
                            '\n "reason":"Could not start game."' +
                            '\n}')
timeout_msg: str = ('{' +
                    '\n "status":"error",' +
                    '\n "reason":"Subprocess timed out."' +
                    '\n}')
crash_msg: str = ('{' +
                  '\n "status":"error",' +
                  '\n "reason":"Subprocess crashed."' +
                  '\n}')
closed_msg: str = ('{' +
                   '\n "status":"error",' +
                   '\n "reason":"Subprocess was closed."' +
                   '\n}')

class GameRequestServer(http.server.HTTPServer):
    def __init__(self, server_address: tuple[str, int], RequestHandlerClass: type[GameRequestHandler], log: logging.Logger):
        super().__init__(server_address, RequestHandlerClass)
        self.server_name: str = server_address[0]
        self.server_port: int = server_address[1]
        self.HandlerRequestClass = RequestHandlerClass
        self.log = log

    def get_game(self, name: str) -> game311.Game:
        # TODO
       return game311.Game()

class GameRequest():
    def __init__(self, handler: GameRequestHandler, query: dict[str, str], c_command: str):
        self.handler = handler
        self.query = query
        self.command = c_command

    def respond(self, response: str):
        self.handler.respond(response)


class GameRequestHandler(http.server.BaseHTTPRequestHandler):

    def __init__(self, request, client_address, server: GameRequestServer):
        super().__init__(request, client_address, server)
        self.server: GameRequestServer = server # Not necessary, helps with type inference
        self.protocol_version = 'HTTP/1.1'
        
    

    def do_GET(self) -> None:
        # Prevent the connection from closes when this function returns
        # Allows us to relegate responding to another thread
        # However, we must actually respond to each 
        # request, otherwise the server will hang
        self.close_connection = False
                
        unquoted = urllib.parse.unquote(self.path)
        parsed = urllib.parse.urlparse(unquoted)
        path = parsed.path.split('/')
        httpCommand = path[-1]
        # No favorite icon for the classic server
        if httpCommand == 'favicon.ico':
            self.respond("Not supported")
        if httpCommand == 'getGames':
            # TODO
            return
        game_name = path[-2]
        
        # Can't use urlparse.parse_qs because of equal signs in board string
        query: defaultdict[str, str] = defaultdict(str)
        keyValPairs = parsed.query.split('&')
        for keyValPair in keyValPairs:
            if keyValPair != '':
                splitKeyVal = keyValPair.split('=', 1)
                if len(splitKeyVal) == 2:
                    # Both key and value present
                    query[splitKeyVal[0]] = splitKeyVal[1]
                else:
                    # Only key present
                    query[splitKeyVal[0]] = ''
                    
        self.server.log.info(f"GET: {unquoted}")

        game = self.server.get_game(game_name)
        
        # Board should start and end in quotes
        if 'board' in query and query['board'][0] != '"':
            query['board'] = f'"{query["board"]}"'

        httpCommandToGamesmanCommand = {
                'getStart':
                'start_response',

                'getEnd':
                f'end_response {query["board"]}',

                'getNextMoveValues':
                f'detailed_position_response {query["board"]}',

                'getMoveValue':
                f'move_value_response {query["board"]}'
            }
        
        if httpCommand not in httpCommandToGamesmanCommand:
            self.respond(could_not_parse_msg)
            return
        c_command = httpCommandToGamesmanCommand[httpCommand]
        # Dispatches responsibility to the game object to respond
        game.push_request(GameRequest(self, query, c_command))
        
        # TODO
        for f in os.listdir(root_game_directory):
            if f == "mttt":
                bin_path = os.path.join(os.path.curdir, "mttt")
        process = GameProcess(self.server, game, bin_path)
        process.push_request(GameRequest(self, query, c_command))

    def respond(self, response: str) -> None:
        self.server
        self.send_response(200)
        self.send_header('Content-Type', 'text/plain')
        self.send_header('Content-Length', str(len(response)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(response.encode('utf-8'))

        self.server.log.debug(f"Sent headers {str(self.headers)}.")
        self.server.log.info(f"Sent response {response}")        
    
def get_log():
    log = logging.getLogger('server')

    if log_to_file:
        file_logger = RotatingFileHandler(log_filename, maxBytes=max_log_size)
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

    log.setLevel(log_level)
    return log

class GameProcess():

    def __init__(self, server: GameRequestServer, game: game311.Game, bin_path: str, game_option_id: typing.Optional[int] = None):
        self.server = server 
        self.game = game
        self.request_queue: Queue[GameRequest] = Queue()
        self.game_option_id = game_option_id
        # How long with no activity in process until it's closed
        self.req_timeout = subprocess_idle_timeout
        
        # Time to wait polling response queue
        self.req_timeout_step = 0.1
        
        # Time to TODO
        self.read_timeout = subprocess_response_timeout

        # Note that arguments to GamesmanClassic must be given in the right
        # order (this one, to be precise).
        arg_list = [bin_path]
        if game_option_id is not None:
            arg_list.append('--option')
            arg_list.append(str(game_option_id))
        arg_list.append('--interact')

        # Open a subprocess, connecting all of its file descriptors to pipes,
        # and set it to line buffer mode.
        self.process = subprocess.Popen(arg_list, stdin=subprocess.PIPE,
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.STDOUT,
                                        cwd=root_game_directory, 
                                        close_fds=True)
        
        # Casting does nothing in runtime but makes the linter happy, since
        # we know self.process.stdout will not be None, but the linter does not. 
        self.stdin = typing.cast(typing.IO[bytes], self.process.stdin)
        self.stdout = typing.cast(typing.IO[bytes], self.process.stdout)
        
        self.setup_subprocess_pipe(self.stdout)
        
        self.close()
        
        self.timeout_msg = timeout_msg
        self.crash_msg = crash_msg
        self.closed_msg = closed_msg

        # Start looping request_loop until shutdown. See request_loop comment for more
        self.thread = threading.Thread(target=self.request_loop)
        self.thread.daemon = True
        self.thread.start()
        
    def push_request(self, request: GameRequest):
        self.request_queue.put(request)
        
    # Periodically checks the request queue for this process. If enough time has been spent
    # idle (no requests seen), or memory usage is too high, then close the process.
    def request_loop(self):
        live = True
        total_idle_time = 0.0 # Time spent with no request
        while live:
            self.server.log.debug(
                'In request loop for {}.'.format(self.game.name))
            try:
                # Wait for request for self.req_timeout_step time
                request = self.request_queue.get(block=True,
                                         timeout=self.req_timeout_step)
            except queue.Empty as e:
                # Waited self.req_timeout_step time with no requests arriving
                total_idle_time += self.req_timeout_step
                if total_idle_time > self.req_timeout:
                    self.server.log.error(
                        f"{self.game.name} closed from lack of use.")
                    live = False
            else:
                # Found a request in the queue, so reset idle time and handle request
                total_idle_time = 0.0
                live = self.handle(request)
            self.server.log.debug('{} is using {}% of memory.'
                                  .format(self.game.name,
                                          self.memory_percent_usage()))
            if self.memory_percent_usage() > max_memory_percent_per_process:
                self.server.log.error('{} used too much memory!'
                                      .format(self.game.name))
                live = False
        self.close()
        
    def handle(self, request: GameRequest) -> bool:
        time_remaining = self.wait_for_ready(self.read_timeout)
        if time_remaining <= 0:
            return self.handle_timeout(request, process_output='')
        self.server.log.debug('Sent command {}.'.format(request.command))
           
        try:
            self.stdin.write((request.command + '\n').encode())
            self.stdin.flush()
        except BrokenPipeError:
            # This case can be hit if the subprocess crashes between requests
            # (known to occasionally happen).
            self.server.log.error('{} crashed on write!'
                                  .format(self.game.name))
            request.respond(self.crash_msg)
            return False
        
        response: str = ''
        timeout = 0.01
        parsed = None
        # Construct response by reading character by character.
        # Whenever a newline is reached, try to parse the response
        # If it is parsable (thus complete), we respond with the parsed request
        # Otherwise, we repeat
        while not parsed:
            while (next_char := self.stdout.read(1)) != b'\n':
                if time_remaining <= 0:
                    self.server.log.debug('timeout')
                    return self.handle_timeout(request, response)
                if next_char is None:
                    # Nothing to read, waiting on output, so wait and try again 
                    time.sleep(timeout)
                    time_remaining -= timeout
                else:
                    response += next_char.decode()
            parsed = self.parse_response(response)
        request.respond(parsed)
        return True
    
    def handle_timeout(self, request: GameRequest, process_output: str) -> bool:
        # Did not receive a response from the subprocess, so
        # exit and kill it.
        self.server.log.error('Did not receive result '
                              'from {}!'.format(self.game.name))
        self.server.log.error('Process sent output:\n'
                              '{}'.format(process_output))
        if self.process.poll() is not None:
            # If the process is no longer running
            self.server.log.error(f"{self.game.name} crashed!")
            response = self.crash_msg
            program_should_live = False
        else:
            self.server.log.error('{} timed out!'.format(self.game.name))
            response = self.timeout_msg
            program_should_live = not close_on_timeout
        request.respond(response)
        return program_should_live

    # Wait until we see ready string in process output to indicate we
    # can send input. Returns the amount of time until a timeout
    # based on the timeout argument.
    def wait_for_ready(self, timeout: float) -> float:
        read_timeout: float = 0.001
        total_time = read_timeout
        ready: bool = False # True iff ready string found in output
        while total_time < timeout and not ready:
            total_time += read_timeout
            last_output: bytes | None = self.stdout.read()
            if last_output is None:
                # None if output stream was empty
                last_output = b""
            last_output_str = last_output.decode()
            self.server.log.debug(f"{self.game.name} read:\n"
                                  f"{last_output}")
            if ' ready =>>' in last_output_str:
                self.server.log.debug(f'Found ready prompt in {total_time} seconds.')
                ready = True
        if not ready:
            self.server.log.error(f"Could not find ready prompt for {self.game.name}.")
        return timeout - total_time

    def memory_percent_usage(self) -> float:
        try:
            # "ps -o pmem pid returns %MEM \n mem_percentage for process with id pid"
            ps_output = subprocess.check_output(
                'ps -o pmem'.split() + [str(self.process.pid)]).decode()
            percent = float(ps_output.split('\n')[1].strip())
            #self.server.log.debug('{} is using {}% of memory.'
            #                      .format(self.game.name, percent))
        except Exception as e:
            self.server.log.error('Could not get memory use of {}, '
                                  'because of error: {}.'
                                  .format(self.game.name, e))
            percent = 0.0
        return percent

    def close(self) -> None:
        while not self.request_queue.empty():
            self.request_queue.get().respond(self.closed_msg)
        self.server.log.info('Closing {}.'.format(self.game.name))
        self.game.remove_process(self)
        self.server.log.debug('Sending exit command to {}.'.format(self.game.name))
        self.stdin.write(b'exit\n')
        try:
            self.stdin.flush()
        except BrokenPipeError: # TODO
            self.server.log.error(f"{self.game.name} normal exit BrokenPipeError. 
                                  The process was likely terminated already.")
        normal_exit_timeout = 60
        normal_exit_timeout_step = 5
        while self.process.poll() is None and normal_exit_timeout > 0:
            time.sleep(normal_exit_timeout_step)
            normal_exit_timeout -= normal_exit_timeout_step
        if self.process.poll() is not None:
            # Program successfully terminated
            return
        self.server.log.error(f"{self.game.name} normal exit timeout!")
        try:
            self.process.terminate()
            time.sleep(1)
            self.process.kill()
        except OSError:
            pass

    def parse_response(self, response: str) -> str | None:
    # TODO
        if 'result' not in response:
            self.server.log.debug('"result" not in string to parse: {!r}'
                                 .format(response))
            return None
        lines = response.split('\n')
        for line in lines:
            if line.startswith('result'):
                self.server.log.debug('Parsing line starting with result.')
                try:
                    result = line.split('=>>')[1].strip()
                    self.server.log.debug('Split off "result =>>"')
                    parsed = json.loads(result)
                    self.server.log.debug('Parsed json from {!r}'.format(result))
                except Exception as e:
                    self.server.log.debug('Could not parse: {!r}'.format(result))
                    # Catches problems with split, indexing, and non json
                    # together
                    return None
                return self.game.format_parsed(parsed)

        
    @property
    def alive(self):
        return self.thread.is_alive()
        
        
    def setup_subprocess_pipe(self, pipe: typing.IO[bytes]):
        descriptor = pipe.fileno()
        # fcntl is not available on windows. Use WSL and run a remote
        # vscode session to use it

        # Add a flag to the file descriptor allowing non-blocking reads / writes
        # to the file
        flags = fcntl.fcntl(descriptor, fcntl.F_GETFL)
        fcntl.fcntl(descriptor, fcntl.F_SETFL, flags | os.O_NONBLOCK)

def main():
    server: GameRequestServer = GameRequestServer((server_address, port), GameRequestHandler, get_log())
    server.serve_forever()

if __name__ == "__main__":
    main()