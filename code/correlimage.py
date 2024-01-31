import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

L = 30
rhos = pd.read_csv('../data/hydro_j' + str(L) + 'by' + str(L) + '.dat', header=None, skiprows=1).to_numpy()[:, :-1]
rhos = rhos.reshape((-1, L, L)).astype(int)

result = np.zeros((L, L))
for idx in range(len(rhos)-1):
    x = rhos[idx]
    y = rhos[idx+1]
    for i in range(L):
        for j in range(L):
            result[i,j] += np.average(x[i:,j:] * y[:L-i,:L-j])
result /= len(rhos)-1

plt.imshow(result)
plt.colorbar()
plt.savefig('../img/hydro_jcorrel' + str(L) + 'by' + str(L))
