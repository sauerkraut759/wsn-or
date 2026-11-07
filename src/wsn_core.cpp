#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>
#include <utility>
#include <cstddef>

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

// PYBIND MACRO
PYBIND11_MODULE(wsn_core, m) {
	m.doc() = "Modulo HPC para ruteo WSN";

	m.def("compute_feasible_edges", &compute_feasible_edges, "Testing");
}
