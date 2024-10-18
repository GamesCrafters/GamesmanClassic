import collections
import os
import os.path
import json
import time

class Game(object):
    indent_response = True

    def __init__(self, server, name):
        self.server = server
        self.name = name
        self.root_dir = server.root_game_directory
        self.processes = collections.defaultdict(list)
        self.process_class = server.GameProcess
        self._working = 'maybe'

    def get_process(self, query):
        self.delete_closed_processes()
        opt = query.get('number', None)
        if opt is None:
            opt = self.get_option(query)

        # Check that there is a non-empty list of processes for this option
        if self.processes[opt]:
            return self.processes[opt][0]
        else:
            return self.start_process(query, opt)

    def format_parsed(self, parsed):
        return json.dumps(parsed, indent=self.indent_response)

    @property
    def has_script(self):
        return type(self) != Game

    @property
    def working(self):
        if self._working == 'maybe':
            self._working = self.check_if_working()
        return self._working

    def check_if_working(self):
        proc = self.get_process(collections.defaultdict(str))
        time_remaining = 600
        time_step = 0.01
        class TestRequest(object):

            def __init__(self, game, proc):
                self.query = collections.defaultdict(str)
                self.command = 'start_response'
                self.game = game
                self.proc = proc

            def respond(self, response):
                try:
                    parsed = json.loads(response)
                    self.game._working = parsed[u'response'] != u'not implemented'
                except Exception:
                    pass
                finally:
                    self.proc.close()
        try:
            proc.push_request(TestRequest(self, proc))
        except AttributeError:
            return False
        while proc.alive and time_remaining > 0:
            time_remaining -= time_step
            time.sleep(time_step)
            if self._working != 'maybe':
                return self._working
        if self._working == 'maybe':
            return False
        else:
            return self._working
        

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
                bin_path = os.path.join(os.path.curdir, bin_name)
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

    def respond_to_unknown_request(self, req):
        raise Exception("Cannot handle query.")

    def remove_process(self, process):
        self.server.log.info('Removing {}.'.format(self.name))
        try:
            self.processes[process.option_num].remove(process)
        except ValueError as e:
            self.server.log.error('Trying to remove subprocess of {} failed \
            because it could not be found.'.format(self.name))
