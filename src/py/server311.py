from __future__ import annotations
import sys
import http.server
import logging
from logging.handlers import RotatingFileHandler
import urllib.parse
from game311 import Game

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
        self.server: GameRequestServer = self # Getters and setters, make sure static works
        self.server_name: str = server_address[0]
        self.server_port: int = server_address[1]
        self.HandlerRequestClass = RequestHandlerClass
        self.log = log

    def get_game(self, name: str) -> Game:
        # TODO
       return Game()

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
        query: dict[str, str] = {}
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
        pass
    
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


def main():
    server: GameRequestServer = GameRequestServer((server_address, port), GameRequestHandler, get_log())
    server.serve_forever()

if __name__ == "__main__":
    main()