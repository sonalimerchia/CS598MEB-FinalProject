import sys 
import os
import seaborn as sns 
import numpy as np
from parse import parse

from utils import read_dist_matrix

import matplotlib.pyplot as plt

data = {}
directory = sys.argv[1]

for filename in os.listdir(directory): 
    if not filename.endswith("DIST"): 
        continue

    with open(directory + "/" + filename, 'r') as file:
        file_content = file.read()

    matrix = read_dist_matrix(directory, filename)
    params = parse('n{num_cells}_l{loci}_s{seed}-{dist_type}-DIST', filename)
    
    params = params.named
    if params['dist_type'] == "CN3": 
        continue

    key = params['num_cells'] + "-" + params['loci']

    if key not in data: 
        data[key] = {params['dist_type']: matrix}
    else: 
        data[key][params['dist_type']] = matrix

med_errs = np.array([], dtype=np.float64)
mean_errs = np.array([], dtype=np.float64)
loci_med = []
loci_mean = []
key_med = []
key_mean = []

omitted_med = {
    "1000": 0, 
    "2000": 0, 
    "3000": 0,
    "4000": 0
}
omitted_mean = {
    "1000": 0, 
    "2000": 0, 
    "3000": 0,
    "4000": 0
}
total = {
    "1000": 0, 
    "2000": 0, 
    "3000": 0,
    "4000": 0
}

for cells_loci in data: 
    keys = data[cells_loci].keys()
    if len(keys) != 2: 
        continue

    loci = parse('{}-{loci}', cells_loci)
    loci = loci.named["loci"]

    med = np.minimum(data[cells_loci]["CNT"], data[cells_loci]["CNT"].T)
    mean = (data[cells_loci]["CNT"] + data[cells_loci]["CNT"].T) / 2

    med_mask = np.where(med != np.inf)
    mean_mask = np.where(mean != np.inf)

    diff_med = np.absolute(med - data[cells_loci]["ZCNT"])
    sum_med = med + data[cells_loci]["ZCNT"]
    diff_mean = np.absolute(mean - data[cells_loci]["ZCNT"])
    sum_mean = mean + data[cells_loci]["ZCNT"]

    sum_med[sum_med == 0.0] = 1
    sum_mean[sum_mean == 0.0] = 1

    med_err = diff_med[med_mask] / sum_med[med_mask]
    mean_err = diff_mean[mean_mask] / sum_mean[mean_mask]

    omitted_med[loci] += np.count_nonzero(med == np.inf)
    omitted_mean[loci] += np.count_nonzero(mean == np.inf)
    total[loci] += med.size

    med_errs = np.hstack((med_errs, med_err))
    mean_errs = np.hstack((mean_errs, mean_err))
    key_med = key_med + (["Median Distance-ZCNT"] * len(med_err))
    key_mean = key_mean + (["Mean Correction-ZCNT"] * len(mean_err))
    loci_med = loci_med + ([loci] * len(med_err))
    loci_mean = loci_mean + ([loci] * len(mean_err))

all_errs = np.hstack((med_errs, mean_errs))
all_keys = key_med + key_mean
all_loci = loci_med + loci_mean

print(np.unique(all_keys))
boxplot = sns.histplot(x=all_errs, hue=all_keys, stat="percent", element="step", bins=30, common_norm=False)
boxplot.set(
    xlabel='Relative Error', 
    ylabel='Percent',
    title='Symmetric Measure Comparisions'
)
fig = boxplot.get_figure()
fig.savefig("sym_fig_hist.png") 
plt.clf()

boxplot = sns.boxplot(y=all_errs, x=all_loci, hue=all_keys, showfliers=False,  order=["1000", "2000", "3000", "4000"])
boxplot.set(
    xlabel='Loci', 
    ylabel='Relative Error',
    title='Symmetric Algorithm Comparision (Outliers Excluded)'
)
# boxplot.legend().set_title("Methods")
fig = boxplot.get_figure()
fig.savefig("sym_fig_boxplot.png") 

print("ommitted_mean", omitted_mean)
print("ommitted_med", omitted_med)
print("total", total)

