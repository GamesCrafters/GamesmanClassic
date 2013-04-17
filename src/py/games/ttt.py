import game

class ttt(game.Game):

    def get_option(self, query):
        if query['misere'] == 'yes':
            return 1
