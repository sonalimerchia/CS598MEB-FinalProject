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
    params = parse('n{num_cells}_l{loci}_s{}-{dist_type}-DIST', filename)
    
    params = params.named
    params['num_cells'] *= 2 

    if params['loci'] not in data: 
        data[params['loci']] = {params['dist_type']: matrix}
    else: 
        data[params['loci']][params['dist_type']] = matrix

errs = {}

for locus in data: 
    keys = data[locus].keys()
    for i in keys:
        for j in keys: 
            if i > j: 
                key = i + "-" + j
                
                diff = np.absolute(data[locus][i] - data[locus][j])
                sum = data[locus][i] + data[locus][j]
                sum[sum == 0.0] = 1
                
                mask = np.where(sum != np.inf)
                
                err = diff[mask] / sum[mask]

                if key not in errs: 
                    errs[key] = np.array(err, dtype=np.float64)
                else: 
                    errs[key] = np.hstack((errs[key], err))
                # errs = np.hstack((errs, err))
                # key_m = key_m + ([key] * len(err))


data = []
labels = []
for key in errs: 
    data.append(errs[key])
    labels.append(key)

plt.hist(data, label=labels, density=True)
plt.legend(loc='upper right')
plt.ylabel("Density")
plt.xlabel("Relative Error")
plt.savefig("dist_fig.png")


# boxplot = sns.histplot(x=errs, hue=key_m, stat="percent")
# boxplot.set(
#     xlabel='Error', 
#     ylabel='Percent',
#     title='Distance Algorithm Comparision'
# )
# boxplot.get_legend().set_title("Methods")
# fig = boxplot.get_figure()
# fig.savefig("dist_fig.png") 