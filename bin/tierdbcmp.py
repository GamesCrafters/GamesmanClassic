#!/usr/bin/env python
# Filename: tierdbcmp
# Author: GamesCrafters - Roger Tu
# Purpose: Comparing Databases

# Instructions:
# place script in /gamesman/bin
# call with game name (as seen in /gamesman/bin/data)

# Notes:
# For comparisons of other types of DBs, please use dbcmp.py
# This can be easily merged with dbcmp.py once tierdb directory and file
# naming conventions have been established.
# =====
# This script does not concatenate results. (see above for reason)

'''Usage:\t./tierdbcmp.py gameName[.exe]
\tStores results in /gamesman/bin/data/dbcmp'''

import os
import sys
import filecmp
import subprocess

if len(sys.argv) < 2: # Check to make sure there are enough arguments
    print 'Usage:\t./tierdbcmp.py gameName[.exe]'
    print '\tStores results in /gamesman/bin/data/dbcmp'
else: # Enough arguments are given
    gName = sys.argv[1] # gamename is first argument
    if len(gName) > 4 and gName[-4:] == '.exe':
        gName = gName[:-4]
    command = 'ls -d ' + gName + '*tierdb'
    os.chdir('data')
    pdirs = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE).stdout.readlines()
    for k in range(0, len(pdirs)):
        pdirs[k] = pdirs[k][:-1]
    if len(pdirs) > 0:
        oks = []
        if not os.path.exists('dbcmp'): # creates dbcmp dir if nonexistant
                os.mkdir('dbcmp')
        os.chdir('dbcmp')
        text = open(gName + '_tier.txt', 'w')
        print '\nSaving results in: data/dbcmp/' + gName + '_tier.txt\n'
        os.chdir('..')
        for dir in pdirs:
            os.chdir(dir)
            dirok = True
            command = 'ls -1'
            pfiles = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE).stdout.readlines()
            for k in range(0, len(pfiles)):
                pfiles[k] = pfiles[k][:-1]
            for i in range(0, len(pfiles) - 1):
                for j in range(i + 1, len(pfiles)):
                    test = filecmp.cmp(pfiles[i], pfiles[j])
                    if test:
                        dirok = False
                        print 'DBs same (%s, %s)' % (pfiles[i], pfiles[j])
                        text.write('DBs same (%s, %s)\n' % (pfiles[i], pfiles[j]))
            if dirok:
                oks.append(dir)
            os.chdir('..')
        if len(oks) == 0:
            print '\n\nSomething may be broken'
            text.write('\n\nSomething may be broken')
        else:
            print '\nEverything different'
            text.write('\nEverything different')