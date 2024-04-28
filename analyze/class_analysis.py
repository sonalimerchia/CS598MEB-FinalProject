
import sys 
import os
import seaborn as sns 
import numpy as np
from parse import parse

from utils import read_dist_matrix

directory = sys.argv[1]

matching_files = {} # mf[loci][dist_type] = filename
for filename in os.listdir(directory): 
    if not filename.endswith("DIST"): 
        continue

    params = parse('n{num_cells}_l{loci}_s{}-{dist_type}-DIST', filename)
    params = params.named
    if params['loci'] not in matching_files: 
        matching_files[params['loci']] = {params['dist_type']: filename}
    else: 
        matching_files[params['loci']][params['dist_type']] = filename

C1 = {}
C2 = {}
C3 = {}

for l in matching_files: 
    if "CNT" not in matching_files[l]: 
        continue 
    
    cnt_dist = read_dist_matrix(directory, matching_files[l]["CNT"])
    is_reachable = cnt_dist != np.inf 
    
    C1_mask = np.logical_and(is_reachable, is_reachable.T)
    C2_mask = np.logical_and(is_reachable, ~is_reachable.T)
    C3_mask = np.logical_and(~is_reachable, ~is_reachable.T)

    for dt in matching_files[l]: 
        if dt not in C1: 
            C1[dt] = np.array([], dtype=np.float64)
            C2[dt] = np.array([], dtype=np.float64)
            C3[dt] = np.array([], dtype=np.float64)

        mat = read_dist_matrix(directory, matching_files[l][dt])
        C1[dt] = np.hstack((C1[dt], mat[C1_mask]))
        C2[dt] = np.hstack((C2[dt], mat[C2_mask]))
        C3[dt] = np.hstack((C3[dt], mat[C3_mask]))


dists = np.hstack((C1['ZCNT'], C2['ZCNT'], C3['ZCNT'])).astype(np.int64)
labels = (["Class 1"] * len(C1['ZCNT'])) + (["Class 2"] * len(C2['ZCNT'])) + (["Class 3"] * len(C3['ZCNT']))
histplot = sns.histplot(x=dists, hue=labels, stat='density', kde=True, bins=25)
histplot.set(
    xlabel='Distance', 
    ylabel='Percent',
    title='Class Distance Comparision'
)
fig = histplot.get_figure()
fig.savefig("classes.png") 