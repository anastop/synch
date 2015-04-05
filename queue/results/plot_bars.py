#!/usr/bin/env python

import sys
import matplotlib
matplotlib.use('PDF')
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.cm as c
import matplotlib.cm as cm
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

# Returns a cartesian product of 2 string lists. 
# Strings from the first list are concatenated with
# strings from the second list in an all-to-all combination,
# and separated with the specified delimiter 
def string_product(list1, list2, delim):
    list = []
    for s1 in list1:
        for s2 in list2:
            list.append(s1 + delim + s2)
    return list


infile = sys.argv[1]
f1 = open(infile, 'r')
csvfile = infile + '.csv'
f2 = open(csvfile, 'w')

linearray = []
queues = []
qsizes = []
local_works = []
for line in f1.readlines():
    if 'Queue:' in line:
        linearray.append(line.strip())
        cur_q = get_val(line,'Queue:')
        if cur_q not in queues:
            queues.append(cur_q)
        cur_qsize = get_val(line,'queue_size:')
        if cur_qsize not in qsizes:
            qsizes.append(cur_qsize)
        cur_lwork = get_val(line, 'nsecs_to_spin:')
        if cur_lwork not in local_works:
            local_works.append(cur_lwork)

# Sort in place 'qsizes' and 'local_works' arrays 
qsizes.sort(lambda a,b: cmp(int(a), int(b))) 
local_works.sort(lambda a,b: cmp(int(a), int(b)))

# Column headers format: <queue_implementation>+<local_work_nsecs>
cols = string_product(queues, local_works, '+')

# print header
f2.write('id    Queue_size ' + ' '.join(cols) + " \n")

# print column values
i = 1
for qs in qsizes:
    f2.write(str(i) + " " + str(qs) + " ")
    for c in cols:
        tokens=c.split('+')
        for line in linearray:
            if ('queue_size:' + str(qs) + ' ') in line \
            and ('Queue:' + tokens[0] + ' ') in line \
            and ('nsecs_to_spin:' + tokens[1] + ' ') in line:
                f2.write(get_val(line,'cycles_per_iter_wo_delay:') + ' ')
    f2.write('\n')
    i = i + 1

f2.close()

#########################################################################          
a = mlab.csv2rec(csvfile, delimiter=' ')

# Data, info and preferences about a specific plot
class PlotData:
    def __init__(self, columns, colormap, title, outfile):
        self.columns = columns 
        self.colormap = colormap
        self.title = title
        self.outfile = outfile

# plots list
plots = [
       PlotData(
            string_product(['stage_lam'],
                           ['1','10','100','200','500','1000','2000'],''),
            'Blues',
            'Lamport queue',
            'lam.pdf'),
        
        PlotData(
            # the verbose way
            # selected_cols = ['stage_ff1', 'stage_ff10', 
            #                  'stage_ff50', 'stage_ff500',
            #                  'stage_ff1000', 'stage_ff2000']
            string_product(['stage_ff'],
                           ['1','10','100','200','500','1000','2000'],''),
            'Oranges',
            'Fast-forward queue',
            'ff.pdf'),
  ]

# bar width
bw = 0.1
# line width
lw = 0.5

maxy = 0
count = 1
for p in plots:
    #New figure for each plot
    plt.figure(count)
    count = count+1

    # number of bars per cluster
    nbars = len(p.columns)

    # Get colors from colormap
    colors = []
    num_colors = nbars
    cmap = cm.get_cmap(p.colormap)
    for i in range(num_colors):
        colors.append(cmap(1.*i/num_colors))

    # Plot bars
    boxgroups = []
    # initial offset of 1st bar
    offset = -nbars/2*bw
    i = 0
    # keep first rectangle of each bar group to plot legend later
    rects = []
    for col in p.columns:
        bgroup = plt.bar(a.id + offset, a[col], bw, 
                        color=colors[i], linewidth=lw)
        rects.append(bgroup[0])
        offset = offset + bw
        i = i + 1
        boxgroups.append(bgroup)

    #plot legend
    plt.legend( rects, map(str.upper, p.columns), loc='best' )
    leg = plt.gca().get_legend()
    ltext = leg.get_texts()
    llines = leg.get_lines()
    plt.setp(ltext, fontsize='small')
    plt.setp(llines, linewidth=2)

    r = plt.axis()
    curry = r[3]
    if curry > maxy:
        maxy = curry

    plt.axis([0.5, len(qsizes)+0.5, 0, maxy])
    plt.xticks(a.id, a.queue_size)
    plt.grid(True)
    plt.ylabel('Avg. cycles per enque/deque pair')
    plt.xlabel('Queue size')
    plt.title(p.title)

    #plt.show()
    plt.savefig(p.outfile)
