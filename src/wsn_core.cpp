#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>
#include <map>
#include <tuple>
#include <utility>
#include <cstddef>
#include <stdexcept>

namespace py = pybind11;

using EdgeList = std::vector<std::pair<int, int>>;
using Node = std::pair<double, double>;

inline double dist_sq(const Node& A, const Node& B) noexcept
{
	double dx = B.first - A.first;
	double dy = B.second - B.second;
	return (dx*dx + dy*dy);
}

EdgeList compute_feasible_edges(const std::vector<Node>& coords, double D_max)
{
	EdgeList E;
	const std::size_t N = coords.size();

	if (N <= 1) return E;

	E.reserve(N * (N - 1) / 2); //Memoria aproximada para evitar realloc

	const double D_max_sq = D_max * D_max;

	for (std::size_t i = 1; i < N; i++) {
		for (std::size_t j = 0; j < N; j++) {
			if (i == j || dist_sq(coords[i], coords[j]) > D_max_sq) continue;

			E.emplace_back(static_cast<int>(i), static_cast<int>(j));
		}
	}

	return E;
}

void build_milp_model(MPSolver* solver, int N, const EdgeList& E, double g, double e_tx, double e_rx)
{
	return;
}

std::tuple<double, EdgeList> optimize_wsn_routing(int N, std::vector<Node> coords, double D_max, int g, double e_tx, double e_rx)
{
	if (N <= 0 || coords.size() <= 0) {
		throw std::invalid_argument("La cantidad de nodos no puede ser menor a 1");
	}
	if (N != coords.size()) {
		throw std::invalid_argument("La cantidad de nodos no coincide con la cantidad N declarada");
	}
	if (D_max <= 1) {
		throw std::invalid_argument("Distancia máxima entre nodos (D_max) debe ser al menos 1");
	}
	if (g <= 0) {
		throw std::invalid_argument("La cantidad de bits transmitidos (g) no puede ser menor a 1");
	}
	if (e_tx < 0 || e_rx < 0) {
		throw std::invalid_argument("Los costos energéticos de transmisión o recepión no pueden ser negativos");
	}
}


// PYBIND MACRO
PYBIND11_MODULE(wsn_core, m) {
	m.doc() = "Modulo HPC para ruteo WSN";

	m.def("optimize_wsn_routing", &optimize_wsn_routing, "Resolver topologia WSN");
}
