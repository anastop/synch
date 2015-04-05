#!/usr/bin/env python

import sys
import matplotlib
matplotlib.use('PDF')
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import mlab

#In a line containing key:value pairs, extract the value
#for the key given as argument
#E.g. in the string s="nthreads:2 op:ADD cycles:2.0"
#get_val(s,"op:") returns ADD
def get_val(s, key):
    tmp = s[s.find(key) + len(key):]
    x=tmp.find(' ')
    ret=tmp[0: (x if x > 0 else len(tmp))]
    if ret[-1] == '\n':
        return ret[0:-1]
    else: 
        return ret

infile = sys.argv[1]
f1 = open(infile, 'r')
csvfile = infile + '.csv'
f2 = open(csvfile, 'w')

maxthreads = 0
linearray = []
ops = []
for line in f1.readlines():
    if 'nthreads:' in line:
        linearray.append(line.strip())
        maxthreads=max(maxthreads,int(get_val(line,'nthreads:')))
        cur_op = get_val(line,'lock:')
        if cur_op not in ops:
            ops.append(cur_op)

#print header
f2.write('nthreads ' + ' '.join(ops) + " \n")

#print column values
for thread in range(1, maxthreads+1):
    f2.write(str(thread) + " ")
    for op in ops:
        for line in linearray:
            if ('nthreads:' + str(thread) + ' ') in line \
            and ('lock:' + op + ' ') in line:
                f2.write(get_val(line,'cycles:') + ' ')
    f2.write('\n')     

f2.close()
#########################################################################  
a = mlab.csv2rec(csvfile, delimiter=' ')

## SHORT version
selected_cols = (
'spin_lock_aligned',
'spin_lock_aligned_paused',
'spin_lock_ttas',
'spin_lock_ttas_paused',
'pthread_mutex',
)

# Create markers according to column name
def create_marker(column):
    if column.find('lock_aligned_paused') >= 0:
        return '^-'
    elif column.find('lock_aligned') >= 0:
        return 's-'
    elif column.find('lock_ttas_paused') >= 0:
        return 'o-'
    elif column.find('lock_ttas') >= 0:
        return 'v-'
    else:
        return '<-'


################################################################
# New figure for plot
plt.figure(1)

curves = []
for col in selected_cols:
    c = plt.plot(a.nthreads, a[col], create_marker(col),
                 linewidth=2 )
    curves.append(c)

plt.legend( curves, map(str.upper, selected_cols), loc='best' )

leg = plt.gca().get_legend()
ltext = leg.get_texts()
llines = leg.get_lines()
plt.setp(ltext, fontsize='small')
plt.setp(llines, linewidth=2)

r = plt.axis()
plt.axis([0.5, maxthreads+0.5, 0, r[3]])
plt.grid(True)

plt.ylabel('Avg. cycles per operation')
plt.xlabel('#Threads')

plt.savefig('lock_scalability.pdf')

################################################################
# Plot for fist 8 threads

# New figure for plot
plt.figure(2)

curves = []
for col in selected_cols:
    c = plt.plot(a.nthreads[:8], a[col][:8], create_marker(col),
                 linewidth=2 )
    curves.append(c)

plt.legend( curves, map(str.upper, selected_cols), loc='best' )

leg = plt.gca().get_legend()
ltext = leg.get_texts()
llines = leg.get_lines()
plt.setp(ltext, fontsize='small')
plt.setp(llines, linewidth=2)

r = plt.axis()
plt.axis([0.5, 8+0.5, 0, r[3]])
plt.grid(True)

plt.ylabel('Avg. cycles per operation')
plt.xlabel('#Threads')

plt.show()
plt.savefig('lock_scalability_until8.pdf')


