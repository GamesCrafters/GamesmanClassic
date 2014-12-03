from ai import AiRequest
class PerfectAi(AiRequest):
    def play(self, moves):
        sort_by_value_dict = {
            'lose' : -1,
            'tie' : 0,
            'win' : 1,
        }
        moves = sorted(moves, key = lambda move: -move['remoteness'])
        moves = sorted(moves, key = lambda move: sort_by_value_dict[move['value']])
        return (moves[0], "")
