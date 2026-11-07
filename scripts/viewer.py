import sys
import os

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'build')))

import wsn_core

nodos = wsn_core.init_nodes(15)
print(f"Nodos inicializados: {nodos}")
