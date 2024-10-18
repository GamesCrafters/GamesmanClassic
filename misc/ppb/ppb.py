'''
ppb.py
~~~
Python bridge. 
'''

import re

# Possible position values. Please use these values when implementing the games. Use ppb.WIN... etc
WIN = 0
LOSE = 1
TIE = 2
DRAW = 3

global_start_func = None
global_stat_func = None
global_next_stats_func = None

#~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Decorator functions. 
def start(func):
	global global_start_func
	global_start_func = func

	return func


def stat(func):
	global global_stat_func
	global_stat_func = func

	return func


def next_stats(func):
	global global_next_stats_func
	global_next_stats_func = func

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
		command = input(">>>")
		if command == "start()":
			print(global_start_func())
		elif re.search("^stat\((.+)\)", command):
			reg = re.search("^stat\((.+)\)", command)
			queried_position = reg.groups()[0]
			try:
				print(global_stat_func(queried_position))
			except Exception:
				print("Invalid command")
		elif re.search("^next_stats\((.+)\)", command):
			reg = re.search("^next_stats\((.+)\)", command)
			queried_position = reg.groups()[0]
			print(queried_position)
			try:
				next_positions = global_next_stats_func(queried_position)
				for p in next_positions:
					print(p)

			except Exception as e:
				# traceback.print_exc()

				# # Get traceback as a string and do something with it
				# error = traceback.format_exc()
				# print(error.upper())
				print("Invalid command")
		else:
			print("Invalid command")