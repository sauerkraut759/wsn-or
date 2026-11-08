import sys
import os
import numpy as np

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'build')))

import wsn_core

coords = np.random.rand(20, 2) * 200
coords[0] = [100.0, 100.0]

E = wsn_core.compute_feasible_edges(coords, 40.0)

print(f"Enlaces Factibles: {E}")
