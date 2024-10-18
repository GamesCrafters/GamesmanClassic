import game


class ttt(game.Game):

    def get_option(self, query):
        if query['misere'] == 'yes':
            return 1

    def respond_to_unknown_request(self, req):
        if req.command == 'getOptions':
            options = [{'misere': 'yes',
                        'number': 1,
                        'width': 3,
                        'height': 3},
                       {'misere': 'no',
                        'number': 2,
                        'width': 3,
                        'height': 3},
                       {}]
            req.respond(self.format_parsed(
                {'status': 'ok',
                 'response': options}))
        else:
            raise NotImplemented()
