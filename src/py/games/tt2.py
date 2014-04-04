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
