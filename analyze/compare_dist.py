import sys 
import os
import seaborn as sns 
import numpy as np
from parse import parse

from utils import read_dist_matrix

import matplotlib.pyplot as plt

data = {}
directory = sys.argv[1]

data['c1'] = {}
data['c2'] = {}
data['c3'] = {}

for filename in os.listdir(directory): 
    if not filename.endswith("DIST"): 
        continue

    with open(directory + "/" + filename, 'r') as file:
        file_content = file.read()

    matrix = read_dist_matrix(directory, filename)
    params = parse('n{num_cells}_l{loci}_s{seed}-{dist_type}-DIST', filename)
    
    params = params.named

    key = params['num_cells'] + "-" + params['loci']

    if key not in data: 
        data[key] = {params['dist_type']: matrix}
    else: 
        data[key][params['dist_type']] = matrix

errs = np.array([], dtype=np.float64) # {}
key_m = []

for cells_loci in data: 
    keys = data[cells_loci].keys()
    if 'CN3' not in data[cells_loci]: 
        continue

    for di in keys:
        for dj in keys: 
            if di <= dj: 
                continue 

            key = di + "-" + dj
                
            diff = np.absolute(data[cells_loci][di] - data[cells_loci][dj])
            sum = data[cells_loci][di] + data[cells_loci][dj]
            sum[sum == 0.0] = 1
                
            mask = np.where(sum != np.inf)
            err = diff[mask] / sum[mask]

                # if key not in errs: 
                #     errs[key] = np.array(err, dtype=np.float64)
                # else: 
                #     errs[key] = np.hstack((errs[key], err))
            errs = np.hstack((errs, err))
            key_m = key_m + ([key] * len(err))


# data = []
# labels = []
# for key in errs: 
#     data.append(errs[key])
#     labels.append(key)

# plt.hist(data, label=labels, density=True)
# plt.legend(loc='upper right')
# plt.ylabel("Density")
# plt.xlabel("Relative Error")
# plt.savefig("dist_fig.png")

print(np.unique(key_m))
boxplot = sns.histplot(x=errs, hue=key_m, stat="density", element="step", bins=30)
boxplot.set(
    xlabel='Error', 
    ylabel='Density',
    title='Distance Algorithm Comparision'
)
# boxplot.legend().set_title("Methods")
fig = boxplot.get_figure()
fig.savefig("dist_fig.png") 