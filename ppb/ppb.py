'''
ppb.py
~~~
Python bridge. 
'''

# Possible position values. Please use these values when implementing the games. Use ppb.WIN... etc
WIN = 0
LOSE = 1
TIE = 2
DRAW = 3

global_start_func = None
global_stat = None
global_next_stats = None

#~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Decorator functions. 
def start(func):
	global global_start_func
	start_func = func

	return func


def stat(func):
	global global_stat
	global_stat = func

	return func


def next_stats(func):
	global global_next_stats
	global_next_stats = func

	return func

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



def make_stat(position, remoteness, value):
	return (position, remoteness, value)


def make_next_stat(move, position, remoteness, value, intermediate=False):
	return (move, position, remoteness, value, intermediate)


def serve():
	'''
	Sets up a loop to read user input and serve the correct responses. 
	'''
	while True:
