from ai import AiRequest
class StallingAi(AiRequest):
    def play(self, moves):
        moves = sorted(moves, key = lambda move: -move['remoteness'])
        return (moves[0],"")
