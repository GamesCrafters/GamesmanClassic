import json
from server311 import GameProcess, GameRequest


class Game:
    
    indent_response = True

    def __init__(self):
        self.name: str = ''

    def push_request(self, request: GameRequest) -> None:
        pass
    
    def format_parsed(self, parsed: str) -> str:
        return json.dumps(parsed, indent=self.indent_response)

    def remove_process(self, process: GameProcess) -> None:
        pass