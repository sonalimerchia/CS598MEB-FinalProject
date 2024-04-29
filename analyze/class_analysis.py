
import sys 
import os
import seaborn as sns 
import numpy as np
from parse import parse
from tqdm import tqdm
import pickle

from utils import read_dist_matrix

directory = sys.argv[1]

def plot_data(dists, labels): 
    print("Plotting...")
    histplot = sns.histplot(x=dists, hue=labels, stat='density', bins=20)
    histplot.set(
        xlabel='Distance', 
        ylabel='Density',
        title='Class Distance Comparision'
    )
    print("Plotted")
    fig = histplot.get_figure()
    fig.savefig("classes.png") 

if directory.endswith(".pkl"): 
    data = None
    with open(directory, 'rb') as pickle_file:
        data = pickle.load(pickle_file)
    plot_data(data['distance'], data['labels'])
    exit(0) 

matching_files = {} # mf[loci][dist_type] = filename
for filename in os.listdir(directory): 
    if not filename.endswith("DIST"): 
        continue
    if filename.rfind("CN3") != -1: 
        continue

    params = parse('n{num_cells}_l{loci}_s{seed}-{dist_type}-DIST', filename)
    params = params.named
    k = params['loci'] + "-" + params['seed'] + "-" + params['num_cells']
    if k not in matching_files: 
        matching_files[k] = {params['dist_type']: filename}
    else: 
        matching_files[k][params['dist_type']] = filename

C1 = {}
C2 = {}
C3 = {}

for l in tqdm(matching_files): 
    if "CNT" not in matching_files[l]: 
        continue 
    
    mat = read_dist_matrix(directory, matching_files[l]["CNT"])
    is_reachable = mat != np.inf 
    
    C1_mask = np.logical_and(is_reachable, is_reachable.T)
    C2_mask = np.logical_xor(is_reachable, is_reachable.T)
    C3_mask = np.logical_and(~is_reachable, ~is_reachable.T)

    for dt in matching_files[l]: 
        if dt not in C1: 
            C1[dt] = np.array([], dtype=np.float64)
            C2[dt] = np.array([], dtype=np.float64)
            C3[dt] = np.array([], dtype=np.float64)

        mat = read_dist_matrix(directory, matching_files[l][dt])
        num_elements = mat.shape[0] * mat.shape[1]
        C1[dt] = np.hstack((C1[dt], mat[C1_mask]))
        C2[dt] = np.hstack((C2[dt], mat[C2_mask]))
        C3[dt] = np.hstack((C3[dt], mat[C3_mask]))


dists = np.hstack((C1['ZCNT'], C2['ZCNT'], C3['ZCNT'])).astype(np.int64)
labels = (["Class 1"] * len(C1['ZCNT'])) + (["Class 2"] * len(C2['ZCNT'])) + (["Class 3"] * len(C3['ZCNT']))
data = {
    "distance": dists, 
    "labels": labels
}
with open('saved_class_analysis.pkl', 'wb') as f:
    pickle.dump(data, f)

plot_data(dists, labels)
print("Class 1", len(C1['ZCNT']))
print("Class 2", len(C2['ZCNT']))
print("Class 3", len(C3['ZCNT']))