from __future__ import annotations
from collections import defaultdict
import fcntl
import json
import os
from queue import Queue, Empty
import subprocess
import threading
import time
import typing
import sys
import http.server
import logging
from logging.handlers import RotatingFileHandler
import urllib.parse
import game as game

bytes_per_mb: int = 1024 ** 2

max_log_size: int = 10 * bytes_per_mb

max_memory_percent_per_process: float = 5.0

server_address: str = '' # Localhost
port: int = 8083
root_game_directory: str = "./bin/"
log_filename: str = "server.log"

close_on_timeout: bool = False

# Seconds to wait for a response from the process before sending timeout_msh
subprocess_response_timeout: int = 300

# Seconds to wait without a request before shutting down process
subprocess_idle_timeout: int = 600

log_to_file: bool = False
log_to_stdout: bool = True
log_to_stderr: bool = False
# Choose between logging.DEBUG, logging.INFO, logging.ERROR
log_level: int = logging.INFO

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

class GameRequestServer(http.server.ThreadingHTTPServer):
    
    could_not_start_msg = could_not_parse_msg # TODO
    root_game_directory = root_game_directory
    
    def __init__(self, 
                 server_address: tuple[str, int], 
                 RequestHandlerClass: type[GameRequestHandler], 
                 log: logging.Logger):
        super().__init__(server_address, RequestHandlerClass)
        self.server_name: str = server_address[0]
        self.server_port: int = server_address[1]
        self.HandlerRequestClass = RequestHandlerClass
        self.log = log
        self._game_table: dict[str, game.Game] = {}

    def get_game(self, name: str) -> game.Game:
        if name not in self._game_table:
            self._game_table[name] = start_game(name, self)
        return self._game_table[name]

def start_game(name, server):
    return game.Game(server, name)

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

class GameRequest():

    def __init__(self, handler: GameRequestHandler, query: dict[str, str], command: str):
        self.handler = handler
        self.query = query
        self.command = command

    def respond(self, response: str):
        print(f"[DEBUG] [respond] sending back to client: {response!r}")
        self.handler.respond(response)

class GameRequestHandler(http.server.BaseHTTPRequestHandler):#

    def __init__(self, request, client_address, server: GameRequestServer):
        super().__init__(request, client_address, server)
        self.server: GameRequestServer = server # Not necessary but helps with type inference
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
        if 'favicon.ico' in path:
            return
        
        if len(path) < 4 or path[1] == '' or path[2] == '' or path[3] == '':
            self.respond(could_not_parse_msg)
            return
        
        game_id = path[1]
        variant_id = path[2]
        command = path[3]
        print(f"[DEBUG] Raw path: {self.path}")
        print(f"[DEBUG] command: {command}")

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
        
        query['number'] = variant_id

        self.server.log.info(f"GET: {unquoted}")

        game = self.server.get_game(game_id)
        
        # Board should start and end in quotes
        if 'p' in query and query['p'][0] != '"':
            query['p'] = f'"{query["p"]}"'

        commandToGamesmanCommand = {
                'start':
                'start_response',

                'positions':
                f'position_response {query["p"]}',
            }
        
        if command in commandToGamesmanCommand:
            c_command = commandToGamesmanCommand[command]
            # Dispatches responsibility to the game object to respond
            print(f"[DEBUG] dispatching GameRequest: game_id={game_id}, variant_id={variant_id}")
            print(f"[DEBUG] dispatching command string to subprocess: {c_command}")
            game.push_request(GameRequest(self, query, c_command))
        else:
            try:
                game.respond_to_unknown_request(GameRequest(self, query, command))
            except NotImplementedError:
                self.respond(could_not_parse_msg)
        
    # Send a response to the http request assigned to this handler
    def respond(self, response: str) -> None:
        self.close_connection = True
        self.send_response(200)
        self.send_header('Content-Type', 'text/plain')
        self.send_header('Content-Length', str(len(response)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(response.encode('utf-8'))

        self.server.log.debug(f"Sent headers {str(self.headers)}.")
        self.server.log.debug(f"Sent response {response}")        

# Represents a classic instance of GamesmanClassic running in interact mode
# Responsible for receiving requests, and responding to them 
class GameProcess():
    def __init__(self, server: GameRequestServer, 
                 game: game.Game, 
                 bin_path: str, 
                 game_option: int | None = None):

        self.server = server 
        self.game = game
        self.request_queue: Queue[GameRequest] = Queue()
        self.game_option = game_option
        # How long with no activity in process until it's closed
        self.req_timeout = subprocess_idle_timeout
        
        # Time to wait polling response queue
        self.req_timeout_step = 0.1
        
        # Time to wait for ready message when handling a request. If time
        # waiting exceeds this, then crash
        self.read_timeout = subprocess_response_timeout

        # Note that arguments to GamesmanClassic must be given in the right
        # order (this one, to be precise).
        arg_list = [bin_path]
        if game_option is not None:
            arg_list.append('--option')
            arg_list.append(str(game_option))
        arg_list.append('--interact')

        # Open GamesmanClassic in interact mode, connecting all of its 
        # file descriptors to pipes.
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
        
        self.timeout_msg = timeout_msg
        self.crash_msg = crash_msg
        self.closed_msg = closed_msg

        # Start looping request_loop until shutdown. See request_loop comment for more
        self.thread = threading.Thread(target=self.request_loop)
        self.thread.daemon = True
        self.thread.start()

    def setup_subprocess_pipe(self, pipe: typing.IO[bytes]):
        descriptor = pipe.fileno()
        # fcntl is not available on windows. Use WSL and run a remote
        # vscode session to use it

        # Add a flag to the file descriptor allowing non-blocking reads / writes
        # to the file
        flags = fcntl.fcntl(descriptor, fcntl.F_GETFL)
        fcntl.fcntl(descriptor, fcntl.F_SETFL, flags | os.O_NONBLOCK)
   
    # Request will be handled in request_loop. Also requests will be responded
    # to eventually.
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
            except Empty as e:
                # Waited self.req_timeout_step time with no requests arriving
                total_idle_time += self.req_timeout_step
                if total_idle_time > self.req_timeout:
                    self.server.log.error(
                        f"{self.game.name} closed from lack of use.")
                    live = False
            else:
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
        
    # Respond to a request by sending the appropriate command to the process,
    # parse the output, and send the response.
    #
    # Returns whether the process should continue
    def handle(self, request: GameRequest) -> bool:
        time_remaining = self.wait_for_ready(self.read_timeout)
        if time_remaining <= 0:
            return self.handle_timeout(request, process_output='')
        self.server.log.debug('Sent command {}.'.format(request.command))

        print(f"[DEBUG] [GameProcess.handle] sending to subprocess stdin: {request.command!r}")
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
        # A newline signifies the end of a response, so we parse the response and respond
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
        print(f"[DEBUG] [GameProcess.handle] raw line from subprocess: {response!r}")
        parsed = self.parse_response(response)
        print(f"[DEBUG] [GameProcess.handle] parsed response (after format_parsed): {parsed!r}")
        request.respond(parsed)
        return True
    
    # Respond with timeout message and returns whether the process should continue
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
        # read_timeout: float = 0.001
        # total_time = read_timeout
        # ready: bool = False # True iff ready string found in output
        # while total_time < timeout and not ready:
        #     total_time += read_timeout
        #     time.sleep(read_timeout)
        #     last_output: bytes | None = self.stdout.read()
        #     if last_output is None:
        #         # None if output stream was empty
        #         last_output = b""
        #     last_output_str = last_output.decode()
        #     self.server.log.debug(f"{self.game.name} read:\n"
        #                           f"{last_output}")

        #     if ' ready =>>' in last_output_str:
        #         self.server.log.debug(f'Found ready prompt in {total_time} seconds.')
        #         ready = True
        # if not ready:
        #     self.server.log.error(f"Could not find ready prompt for {self.game.name}.")
        # return timeout - total_time
        print(f"[DEBUG] skipping wait_for_ready, assuming {self.game.name} is ready.")
        return timeout

    # Returns the percentage of on device memory this process is using
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

    # Close the program by all means necessary. First, send the exit command to the
    # program. If that does not work, then forcefully kill the process.
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
            self.server.log.error(f"{self.game.name} normal exit BrokenPipeError." + 
                                  "The process was likely terminated already.")
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

    # Parsing a result that looks like
    # result =>> {result}
    def parse_response(self, response: str) -> str:
        self.server.log.debug(f"[DEBUG] raw subprocess response before parse: {response!r}")

        if '=>>' not in response:
            self.server.log.error(
                f"[ERROR] Subprocess output not in expected format: {response!r}"
            )
            fallback = {
                "status": "error",
                "reason": "Unexpected subprocess output (no '=>>' found)",
                "rawOutput": response,
            }
            # Send back pretty JSON so frontend can read it
            return json.dumps(fallback, indent=2)

        result = response.split('=>>')[1].strip()
        self.server.log.debug(f'Split off "result =>>"')
        parsed = json.loads(result)
        self.server.log.debug('Parsed json from {!r}'.format(result))
        return self.game.format_parsed(parsed)
        
    @property
    def alive(self):
        return self.thread.is_alive()
        
def main():
    server: GameRequestServer = GameRequestServer((server_address, port), GameRequestHandler, get_log())
    # Argument below is the polling interval
    thread = threading.Thread(target=server.serve_forever, args=[0.01])
    thread.start()
if __name__ == "__main__":
    main()