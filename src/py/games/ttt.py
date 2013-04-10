import game

class ttt(game.Game):

    def get_option(self, query):
        if 'misere' in query:
            return 1
