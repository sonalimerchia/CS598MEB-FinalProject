import os 
import sys 
import numpy as np
from parse import parse
import seaborn as sns 
import pickle
import matplotlib.pyplot as plt

import networkx as nx
from utils import read_dist_matrix

directory = sys.argv[1]
external_data = "lazac-copy-number/data/simulations/"
CNT_SUFFIX = "-CNT-DIST"

distances = []
edgelists = []

def find_root(guess: int, T: nx.DiGraph): 
    if guess in T.pred: 
        for p in T.pred[guess]: 
            return find_root(p, T)

    return guess

def classify_edges(T: nx.DiGraph, is_reachable): 
    unreachable = 0
    total = 0 
    for (u, v) in T.edges: 
        total += 1
        unreachable += 1 if not is_reachable[u, v] else 0

    return unreachable > 0, unreachable / total
    
def classify_desc(AD: dict, is_reachable): 
    unreachable = 0
    total = 0 
    for ancestor in AD: 
        for successor in AD[ancestor]: 
            total += 1
            unreachable += 1 if not is_reachable[ancestor, successor] else 0
    
    return unreachable > 0, unreachable / total

def create_desc_map(root: int, T: nx.DiGraph, S: dict): 
    S[root] = []
    for s in T.successors(root): 
        create_desc_map(s, T, S)
        S[root] = S[root] + [s] + S[s]
    
    pass

def plot(): 
    ie = None 
    id = None
    with open(sys.argv[1], 'rb') as pickle_file:
        ie = pickle.load(pickle_file)
    
    with open(sys.argv[2], 'rb') as pickle_file:
        id = pickle.load(pickle_file)

    cells = []
    loci = []
    ie_stack = []
    id_stack = []

    ie_maxes = []
    id_maxes= []

    for c in sorted(ie.keys()): 
        for l in ie[c]:
            # key = format("{} Cells, {} Loci")
            cells = cells + ([c] * len(ie[c][l]))
            loci = loci + ([l] * len(ie[c][l]))
            ie_stack = ie_stack + ie[c][l]
            id_stack = id_stack + id[c][l]

    print("No illegal descendents in: ", len(id_stack) - np.count_nonzero(id_stack), "of", len(id_stack))
    print("No illegal edges in: ", len(ie_stack) - np.count_nonzero(ie_stack), len(ie_stack))

    boxplot = sns.boxplot(x=loci, y=ie_stack, hue=cells, fill=False, order=["1000", "2000", "3000", "4000"])
    boxplot.set(
        xlabel='Loci', 
        ylabel='Percent of Edges',
        title='Impossible Edges'
    )
    plt.legend().set_title("Num Cells")
    fig = boxplot.get_figure()
    fig.savefig("illegal_edges.png") 
    plt.clf()

    boxplot = sns.boxplot(x=loci, y=id_stack, hue=cells, fill=False, order=["1000", "2000", "3000", "4000"])
    boxplot.set(
        xlabel='Loci', 
        ylabel='Percent of AD pairs',
        title='Impossible Ancestor-Descendant Pairs'
    )
    plt.legend().set_title("Num Cells")
    fig = boxplot.get_figure()
    fig.savefig("illegal_desc.png") 
    plt.clf()

    ie_stack = np.sort(ie_stack)
    cumulative_ie = np.arange(1, len(ie_stack) + 1) / len(ie_stack) * 100

    id_stack = np.sort(id_stack)
    cumulative_id = np.arange(1, len(id_stack) + 1) / len(id_stack) * 100

    plt.title("Cumulative Illegality")
    plt.xlabel("Percent of Edges Matching Criteria")
    plt.ylabel("Cumulative Percent of Trees")
    plt.plot(ie_stack, cumulative_ie, label="% of Illegal Edges")
    plt.plot(id_stack, cumulative_id, label="% of Illegal AD Relations")
    plt.legend().set_title("Criteria")
    plt.savefig("cumulative_plot.png")

if directory.endswith('.pkl'): 

    plot()
    exit(0)

for filename in os.listdir(directory): 
    if not filename.endswith(CNT_SUFFIX): 
        continue

    distances.append(filename)
    edgelists.append(external_data + filename[:-len(CNT_SUFFIX)] + "_edgelist.csv")

nt_illegal_desc = 0
nt_illegal_edges = 0
total = 0

ie = {}
id = {}

for d, e in zip(distances, edgelists):
    print(e, d)
    T = nx.read_edgelist(e, create_using=nx.DiGraph, delimiter=",", nodetype=int, comments='src')
    D = read_dist_matrix(directory, d)
    C = np.zeros(D.shape, dtype=np.uint8)

    is_reachable = D != np.inf 

    root = find_root(0, T)
    desc_map = {}
    create_desc_map(root, T, desc_map)
    has_desc, illegal_desc = classify_desc(desc_map, is_reachable)
    has_edges, illegal_edges = classify_edges(T, is_reachable)

    params = parse('n{num_cells}_l{loci}_s{}-{dist_type}-DIST', d)
    params = params.named
    c = params['num_cells']
    l = params['loci']

    if c not in ie: 
        ie[c] = {l: []}
        id[c] = {l: []}
    elif l not in ie[c]: 
        ie[c][l] = []
        id[c][l] = []


    ie[c][l].append(illegal_edges)
    id[c][l].append(illegal_desc)

    total += 1

    if has_desc: 
        nt_illegal_desc += 1
    if has_edges: 
        nt_illegal_edges += 1

    
print('Illegal edge trees: ', nt_illegal_edges, " of ", total)
print('Illegal desc trees: ', nt_illegal_desc, " of ", total)

with open('ie.pkl', 'wb') as f:
    pickle.dump(ie, f)

with open('id.pkl', 'wb') as f: 
    pickle.dump(id, f)