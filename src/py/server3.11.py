import sys
import game
import http.server
import logging
from logging.handlers import RotatingFileHandler
from typing_extensions import TypeVar

Game: game.Game = game.Game

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
    server = None
    def __init__(self, server_address: tuple[str, int], RequestHandlerClass: TypeVar('GameRequestHandler', bound=http.server.BaseHTTPRequestHandler), log: logging.Logger):
        super().__init__(server_address, RequestHandlerClass)
        self.server: GameRequestServer = self # Getters and setters, make sure static works
        self.server_name: str = server_address[0]
        self.server_port: int = server_address[1]
        self.HandlerRequestClass: TypeVar('GameRequestHandler', bound=http.server.SimpleHTTPRequestHandler) = RequestHandlerClass
        self.log = log



class GameRequestHandler(http.server.BaseHTTPRequestHandler):

    def __init__(self, request, client_address, server: GameRequestServer):
        super().__init__(request, client_address, server)
        self.server: GameRequestServer = server # Not necessary, helps with type inference
        self.protocol_version = 'HTTP/1.1'
    

    def do_GET(self) -> None:
        response = "testing"
        self.respond(response)

    def respond(self, response: str) -> None:
        self.send_response(200)
        self.send_header('Content-Type', 'text/plain')
        self.send_header('Content-Length', len(response))
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