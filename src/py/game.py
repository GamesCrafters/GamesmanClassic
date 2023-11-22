from collections import defaultdict
import json
import os
from py.server import GameProcess, GameRequest, GameRequestServer

# Represents one game and its variants. Keeps track of the running processes for that game,
# and is responsible for accepting requests for that game.
class Game:
    
    indent_response = True

    def __init__(self, server: GameRequestServer, name: str):
        self.server = server
        self.name = name
        self.root_game_dir = server.root_game_directory
        self.option_to_processes: defaultdict[int | None, list[GameProcess]] = defaultdict(list)
        self.process_class = GameProcess
        
    def start_process(self, option: int | None) -> GameProcess | None:
        bin_name = 'm' + self.name
        for f in os.listdir(self.root_game_dir):
            if f == bin_name:
                self.server.log.info(f"Starting {self.name}.")
                bin_path = os.path.join(os.path.curdir, bin_name)
                gp = None
                try:
                    gp = self.process_class(self.server, self, bin_path, option)
                except OSError:
                    self.server.log.error('Ran out of file descriptors!')
                else:
                    self.option_to_processes[option].append(gp)
                return gp
        
    def get_process(self, query: dict[str, str]) -> GameProcess | None:
        self.delete_closed_processes()
        option_str = query.get('number', '')
        if not option_str:
            option = self.get_option(query)
        else:
            option = int(option_str)
        if processes := self.option_to_processes[option]:
            process = processes[0]
        else:
            process = self.start_process(option)
        return process

    # Close and remove any dead processes (thread is dead) that were not properly
    # closed (and thus removed from the process list in the first place)
    # This should ideally never do anything, but accounts for threads
    # dying and any uncaught exceptions in the process request loop 
    # causing the process to stay alive while the thread is dead
    def delete_closed_processes(self) -> None:
        for _, processes in self.option_to_processes.items():
            for proc in processes:
                if not proc.alive:
                        proc.close()
        
    def get_option(self, query: dict[str, str]) -> int | None:
        return None

    # Gets the appropritate process and pushes the request to the process
    # All requests sent to processes are guaranteed to be responded to
    def push_request(self, request: GameRequest) -> None:
        proc = self.get_process(request.query)
        if proc:
            proc.push_request(request)
        else:
            # Handles game not being present
            request.respond(self.server.could_not_start_msg)
        
    # Takes in an object as returned from json.loads and formats
    # the object into the required response output
    def format_parsed(self, parsed_object) -> str:
        return json.dumps(parsed_object, indent=self.indent_response)

    def remove_process(self, process: GameProcess) -> None:
        self.server.log.info('Removing {}.'.format(self.name))
        try:
            self.option_to_processes[process.game_option].remove(process)
        except ValueError as e:
            self.server.log.error('Trying to remove subprocess of {} failed \
            because it could not be found.'.format(self.name))

    # Raises not implemented to game does not know how to respond
    # to unknown request
    def respond_to_unknown_request(self, request: GameRequest):
        raise NotImplementedError()