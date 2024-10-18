import game
import server

class quickchess(game.Game):

    class QuickchessProcess(server.GameProcess):

        def __init__(self, *args, **kwargs):
            server.GameProcess.__init__(self, *args, **kwargs)
            # Set timeout to a year
            self.req_timeout = 365 * 24 * 60 * 60

    def __init__(self, server, name):
        game.Game.__init__(self, server, name)
        self.process_class = self.QuickchessProcess

    def get_option(self, query):
        if query['misere'] == 'yes':
            return 2

