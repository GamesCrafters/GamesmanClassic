import game
import server


class tt2(game.Game):

    class TT2Process(server.GameProcess):

        def memory_percent_usage(self):
            return 0.0

    def __init__(self, server, name):
        game.Game.__init__(self, server, name)
        self.process_class = self.TT2Process

    def get_option(self, query):
        if query['misere'] == 'yes':
            return 1

    def respond_to_unknown_request(self, req):
        if req.command == 'getOptions':
            options = [{'misere': 'yes',
                        'number': 1,
                        'width': 6,
                        'height': 3},
                       {'misere': 'no',
                        'number': 2,
                        'width': 6,
                        'height': 3},
                       {}]
            req.respond(self.format_parsed(
                {'status': 'ok',
                'response': options}))
        else:
            raise NotImplemented()
