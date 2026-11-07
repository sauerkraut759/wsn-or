#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>

namespace py = pybind11;

// Test
std::vector<int> init_nodes(int n_nodes)
{
	std::vector<int> nodes;
	for (auto i = 0; i < n_nodes; i++) {
		nodes.push_back(i);
	}

	return nodes;
}

// PYBIND MACRO
PYBIND11_MODULE(wsn_core, m) {
	m.doc() = "Modulo HPC para ruteo WSN";

	m.def("init_nodes", &init_nodes, "Generar lista de IDs de sensores");
}
