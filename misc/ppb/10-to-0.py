import ppb

STARTING_NUMBER = 10

CACHE = [-1] * (STARTING_NUMBER + 1)
REMOTE_CACHE = [-1] * (STARTING_NUMBER + 1)

def game_over(p):
	return p == 0


def generate_moves(p):
	if p == 0:
		return []
	elif p == 1:
		return [-1]
	else:
		return [-1, -2]


def do_move(p, m):
	return p + m

def solve(p):
	# already exists in cache
	if CACHE[p] != -1:
		return (CACHE[p], REMOTE_CACHE[p])

	# if current position is game over state
	if game_over(p):
		CACHE[p] = ppb.LOSE
		REMOTE_CACHE[p] = 0
		return (ppb.LOSE, 0)

	# figure out outcomes of next possible moves
	possible_m = generate_moves(p)
	new_p = [do_move(p, m) for m in possible_m]
	outcomes = [solve(np) for np in new_p]
	outcomes_w = [x for x in outcomes if x[0] == 0]
	outcomes_l = [x for x in outcomes if x[0] == 1]

	# if losing state exists for opposing player
	if outcomes_l:
		CACHE[p] = ppb.WIN
		REMOTE_CACHE[p] = 1 + min(outcomes_l, key=lambda x: x[1])[1]
		return (CACHE[p], REMOTE_CACHE[p])

	CACHE[p] = ppb.LOSE
	REMOTE_CACHE[p] = 1 + max(outcomes_w, key=lambda x: x[1])[1]
	return (CACHE[p], REMOTE_CACHE[p])


@ppb.start
def start():
	return str(STARTING_NUMBER)


@ppb.stat
def stat(position):
	current_position = int(position)
	return ppb.make_stat(position, REMOTE_CACHE[current_position], CACHE[current_position])

@ppb.next_stats
def next_stats(position):
	next_moves = generate_moves(int(position))
	next_positions = [do_move(int(position), nm) for nm in next_moves]
	return [ppb.make_next_stat(str(next_moves[i]), str(next_positions[i]), REMOTE_CACHE[next_positions[i]], CACHE[next_positions[i]], False) for i in range(len(next_moves))]


solve(STARTING_NUMBER)

# Create an infinite while loop that waits for user input
ppb.serve()
