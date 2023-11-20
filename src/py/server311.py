from __future__ import annotations
from collections import defaultdict
import fcntl
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
log_to_stdout: bool = False
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
        print("handling")
        unquoted = urllib.parse.unquote(self.path)
        parsed = urllib.parse.urlparse(unquoted)
        path = parsed.path.split('/')
        httpCommand = path[-1]
        if httpCommand == 'favicon.ico':
            return
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

    def respond(self, response: str) -> None:
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
                                        bufsize=1,
                                        close_fds=True)
        
        # Casting does nothing in runtime but makes the linter happy, since
        # we know self.process.stdout will not be None, but the linter does not. 
        self.stdout = typing.cast(typing.IO[bytes], self.process.stdout)
        
        self.setup_subprocess_pipe(self.stdout)
        
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
            self.wait_for_ready(1000)
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
        return True # TODO
        
    def wait_for_ready(self, timeout):
        read_timeout: float = 0.001
        total_time = read_timeout
        ready: bool = False # True iff ready string found in output
        while total_time < timeout and not ready:
            total_time += read_timeout
            time.sleep(read_timeout)
            last_output: bytes | None = self.stdout.read()
            if last_output is None:
                # None if output stream was empty
                last_output = b""
            last_output_str = last_output.decode()
            self.server.log.debug(f"{self.game.name} read:\n"
                                  f"{last_output_str}")
            if ' ready =>>' in last_output.decode():
                self.server.log.debug('Found ready prompt in {} seconds.'
                                      .format(total_time))
                ready = True
        if not ready:
            self.server.log.error(f"Could not find ready prompt for {self.game.name}.")
        return timeout - total_time

    def memory_percent_usage(self) -> float:
        return 0.0 # TODO
    
    def close(self) -> None:
        pass # TODO
        
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