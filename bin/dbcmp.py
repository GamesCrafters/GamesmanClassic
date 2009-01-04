#!/usr/bin/env python
# Filename: dbcmp
# Author: GamesCrafters - Roger Tu
# Purpose: Comparing Databases

# Instructions:
# place script in /gamesman/bin
# call with game name and filetype (optional, defaults to .bpdb.dat.gz)
# results will be printed out, and also placed into a file gameName.txt
# in data/dmcmp

# Notes:
# For comparing tierdb, please use tierdbcmp.py because of the
# irregular naming conventions for both directories and files.
# tierdbcmp.py can easily be merged with this script, but naming
# conventions need to be cleaned up first.
# =====
# This script concatenates results to avoid redundancy and for
# ease of reading.

'''./dbcmp.py gameName [filetype]
\tFiletype defaults to bpdb.dat.gz
\tStores results in /gamesman/bin/data/dbcmp as *.txt'''

import os
import sys
import filecmp
import subprocess

if len(sys.argv) < 2: # Check to make sure there are enough arguments
    print 'Usage:\t./dbcmp.py gameName[.exe] [filetype]'
    print '\tFiletype defaults to bpdb.dat.gz'
    print '\tStores results in /gamesman/bin/data/dbcmp'
else: # Enough arguments are given
    gName = sys.argv[1] # gamename is first argument
    if len(gName) > 4 and gName[-4:] == '.exe':
        gName = gName[:-4]
    command = './' + gName + ' --numoptions'
    nVarsString = subprocess.Popen(command, shell=True, stderr=subprocess.PIPE).stderr.readlines()
    nVars = int(nVarsString[0][:-1])
    os.chdir('data')
    if len(sys.argv) > 2: # filetype is second argument
        filetype = sys.argv[2]
    else: # filetype defaults to bpdb.dat.gz
        filetype = 'bpdb.dat.gz'
    if filetype == 'tierdb':
        print '\nPlease use tierdbcmp.py'
    else:
        allok = True
        listofsames = []
        uniques = []
        if not os.path.exists('dbcmp'): # creates dbcmp dir if nonexistant
            os.mkdir('dbcmp')
        os.chdir('dbcmp')
        file = open(gName + '.txt', 'w')
        print '\nSaving results in: data/dbcmp/' + gName + '.txt\n'
        os.chdir('..')
        file.write(gName + ' variants 1 through ' + str(nVars))
        file.write(' (' + filetype + ')\n\n')
        for i in range(1, nVars): # foreach variant: compare files
            file1 = gName + '_' + str(i) + '_' + filetype
            sames = [i] # sames holds list of DBs that are the same
            for j in range(i + 1, nVars + 1):
                file2 = gName + '_' + str(j) + '_' + filetype
                test = filecmp.cmp(file1, file2)
                if test:
                    allok = False
                    sames.append(j)
            bool = False # bool is a boolean to see uniqueness
            for comp in listofsames: # foreach previous list
                if sames[0] in comp:
                    bool = True
                    break
            if not bool and len(sames) == 1:
                uniques.append(sames[0])
            elif not bool:
                listofsames.append(sames)
        print 'DBs are unique for:' # everything below here is format and printing
        file.write('DBs are unique for:\n')
        if len(uniques) == 0:
            print 'There are no unique DBs.\n\n'
            file.write('There are no unique DBs.\n\n')
        else:
            counter = 0
            for u in range(0, len(uniques)):
                print uniques[u],
                file.write(str(uniques[u]))
                if counter == 7:
                    print ''
                    file.write('\n')
                    counter = 0
                else:
                    print '\t',
                    file.write('\t')
                    counter += 1
        print '\n\nDBs not unique:'
        file.write('\n\nDBs not unique:')
        if len(listofsames) == 0:
            print '\nAll %s DBs, variants 1 ~ %d are unique' % (gName, nVars)
            file.write('All %s DBs, variants 1 ~ %d are unique'% (gName, nVars))
            file.close()
        else:
            for comp in listofsames:
                print 'DBs for ',
                file.write('\nDBs for ')
                for num in comp:
                    print str(num) + ' ',
                    file.write(str(num) + ' ')
                print 'are the same.'
                file.write('are the same.')
            file.close()