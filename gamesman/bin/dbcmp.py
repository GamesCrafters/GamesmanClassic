#!/usr/bin/env python
# Filename: dbcmp
# Author: GamesCrafters - Roger Tu
# Purpose: Comparing Databases

# Instructions:
# place script in gamesman/bin/
# call with game name and filetype (optional, defaults to .bpdb.dat.gz)
# results will be printed out, and also placed into a file gameName.txt
# in data/dmcmp

import os
import sys
import filecmp
import subprocess

if len(sys.argv) < 2:
    print './dbcmp.py gName [filetype]'
    print '\tfiletype defaults to bpdb.dat.gz'
    print '\tstores results in gamesman/bin/data/dbcmp'
else:
    gName = sys.argv[1]
    if len(gName) > 4 and gName[-4:] == '.exe':
        gName = gName[:-4]
    command = './' + gName + ' --numoptions'
    nVarsString = subprocess.Popen(command, shell=True, stderr=subprocess.PIPE).stderr.readlines()
    nVars = int(nVarsString[0][:-1])
    os.chdir('data')
    if len(sys.argv) > 2:
        filetype = sys.argv[2]
    else:
        filetype = 'bpdb.dat.gz'
    allok = True
    listofsames = []
    uniques = []
    if not os.path.exists('dbcmp'):
        os.mkdir('dbcmp')
    os.chdir('dbcmp')
    file = open(gName + '.txt', 'w')
    print '\nSaving results in: data/dbcmp/' + gName + '.txt\n'
    os.chdir('..')
    file.write(gName + ' variants 1 through ' + filetype)
    file.write(' (' + filetype + ')\n\n')
    for i in range(1, nVars):
        file1 = gName + '_' + str(i) + '_' + filetype
        sames = [i]
        for j in range(i + 1, nVars + 1):
            file2 = gName + '_' + str(j) + '_' + filetype
            test = filecmp.cmp(file1, file2)
            if test:
                allok = False
                sames.append(j)
        bool = False
        for comp in listofsames:
            if sames[0] in comp:
                bool = True
                break
        if bool or len(sames) == 1:
            uniques.append(sames[0])
        else:
            listofsames.append(sames)
    print 'DBs are unique for:'
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