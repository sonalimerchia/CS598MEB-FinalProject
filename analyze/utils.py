import numpy as np

def read_dist_matrix(directory, filename): 
    with open(directory + "/" + filename, 'r') as file:
        file_content = file.read()

    rows = file_content.split("\n")
    matrix = [r.split(',') for r in rows if r != '']
    matrix = np.array([[int(x) for x in row if x != ''] for row in matrix], dtype=np.float64)
    matrix[matrix == -1] = np.inf

    return matrix