import os 
import sys 

import networkx as nx

directory = sys.argv[1]
external_data = "lazac-copy-number/data/simulations/"
CNT_SUFFIX = "-CNT-DIST"

distances = []
edgelists = []

for filename in os.listdir(directory): 
    if not filename.endswith(CNT_SUFFIX): 
        continue

    distances.append(filename)
    edgelists.append(external_data + filename[:-len(CNT_SUFFIX)] + "_edgelist.csv")

for d, e in zip(distances, edgelists):
    H = nx.read_edgelist(e, create_using=nx.DiGraph)


    # TODO: Remove Break
    break