#!/usr/bin/python

import sys
import re

def hash(boardpos):
	h = 0
	fact = 1
	seq = []
	for i in range(0,9):
	    seq.append(int(boardpos[i]))
	indices = []
	for num in range(0, 9):
		indices.append(seq.index(num))
	for i in range(1, 9):
		fact *= i
		sum = 0
		for j in range(0, i):
			sum += int(indices[j] > indices[i])
		h += fact * sum
	return h

print "Content-type: text/html\n\n";

FILENAME = "savetest.txt"
f = open(FILENAME, 'r')
if len(sys.argv) > 1:
    print "boardpos: " + sys.argv[1] + "<br>"
    try:
        hashnum = str(hash(sys.argv[1]))
    except Error:
        print "Error in hash function. <br>"
    print "hashnum: " + hashnum + "<br>"
    data = f.read()
    mat = re.search("\n" + hashnum + ":(?P<level>[0-9]+),(?P<move>[a-zA-Z0-9]+)\n", data)
    if mat:
        print "level: " + mat.group('level') + "<br>"
        seq = []
        boardpos = sys.argv[1]
    	for i in range(0,9):
    	    seq.append(boardpos[i])
    	zeroIndex = seq.index('0');
        direction = ""
        number = ''
        if mat.group('move') == 'u':
            direction = "down"
            number = seq[zeroIndex - 3]
        if mat.group('move') == 'd':
            direction = "up"
            number = seq[zeroIndex + 3]
        if mat.group('move') == 'l':
            direction = "right"
            number = seq[zeroIndex - 1]
        if mat.group('move') == 'r':
            direction = "left" 
            number = seq[zeroIndex + 1]   
        print "hint: move \"" + number + "\" " + direction + "."  + "<br>"
    else:
        if sys.argv[1] == "123456780":
            print "Goal state."
        else:
            print "Error: position not found in database."
else:
    print "Error: a script need an argument."
f.close()