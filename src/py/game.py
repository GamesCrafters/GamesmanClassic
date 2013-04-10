import collections
import os
import os.path

class Game(object):

    def __init__(self, server, name):
        self.server = server
        self.name = name
        self.root_dir = server.root_game_directory
        self.processes = collections.defaultdict(list)
        self.process_class = server.GameProcess

    def get_process(self, query):
        self.delete_closed_processes()
        opt = self.get_option(query)

        # Check that there is a non-empty list of processes for this option
        if self.processes[opt]:
            return self.processes[opt][0]
        else:
            return self.start_process(query, opt)

    def delete_closed_processes(self):
        for k, ps in self.processes.items():
            for proc in ps:
                if not proc.alive:
                    try:
                        proc.process.kill()
                    except Exception:
                        pass
            self.processes[k] = filter(lambda p: p.alive, ps)

    def start_process(self, query, opt):
        bin_name = 'm' + self.name
        for f in os.listdir(self.root_dir):
            if f == bin_name:
                self.server.log.info('Starting {}.'.format(self.name))
                bin_path = os.path.join(self.root_dir, bin_name)
                gp = None
                try:
                    gp = self.process_class(self.server, self, bin_path, opt)
                except OSError as err:
                    self.server.log.error('Ran out of file descriptors!')
                else:
                    proc_list = self.processes.setdefault(opt, [])
                    proc_list.append(gp)
                return gp

    def push_request(self, request):
        proc = self.get_process(request.query)
        if proc:
            proc.push_request(request)
        else:
            # Handles game not being present
            request.respond(self.server.could_not_start_msg)

    def get_option(self, query):
        return None

    def remove_process(self, process):
        self.server.log.info('Removing {}.'.format(self.name))
        try:
            self.processes[process.option_num].remove(process)
        except ValueError as e:
            self.server.log.error('Trying to remove subprocess of {} failed \
            because it could not be found.'.format(self.name))
