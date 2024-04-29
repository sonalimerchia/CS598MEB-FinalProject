import sys 
import os
import seaborn as sns 
import numpy as np
from parse import parse

dist = []
loci = []
times = np.array([], dtype=np.float64)

directory = sys.argv[1]

for filename in os.listdir(directory): 
    if not filename.endswith("TIMES"): 
        continue
    with open(directory + "/" + filename, 'r') as file:
        file_content = file.read()

    l = file_content.split(",")
    t = [int(li) for li in l]
    t = np.array(t) / 1000
    t = t[t > 0]
    t = np.log(t)
    params = parse('n{num_cells}_l{loci}_s{}-{dist_type}-TIMES', filename)
    
    params = params.named
    params['num_cells'] *= 2 

    dist = dist + ([params['dist_type']] * len(t))
    loci = loci + ([params['loci']] * len(t))
    times = np.hstack((times, t))
    
boxplot = sns.boxplot(x=loci, y=times, hue=dist, showfliers=False, fill=False, order=["1000", "2000", "3000", "4000"], hue_order=['ZCNT', 'CNT', 'CN3'])
boxplot.set(
    xlabel='Loci', 
    ylabel='Log Runtime (ms)',
    title='Distance Algorithm Runtime (Outliers Excluded)'
)
fig = boxplot.get_figure()
fig.savefig("time_fig.png") 