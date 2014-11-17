from ai import AiRequest
class PerfectLoseAi(AiRequest):
    def play(self, moves):
        sort_by_value_dict = {
            'lose': -1,
            'tie': 0,
            'win': 1,
        }
        moves = sorted(moves, lambda move: -move['remoteness'] if move['value'] != 'lose' else move['remoteness'])
        moves = sorted(moves, lambda move: sort_by_value_dict[move['value']])
        return (moves[0], "")
