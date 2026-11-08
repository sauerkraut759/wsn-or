#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>
#include <string>
#include <map>
#include <tuple>
#include <utility>
#include <cstddef>
#include <stdexcept>
#include <memory>

#include "ortools/linear_solver/linear_solver.h"

namespace py = pybind11;
namespace ort = operations_research;

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
	using ort::MPSolver;
	using ort::MPVariable;

	std::unique_ptr<MPSolver> solver(MPSolver::CreateSolver("SCIP"));
	if (!solver) {
		throw std::runtime_error("Error al instanciar solver SCIP");
	}

	const double inf = solver->infinity();

	std::vector<MPVariable*> x(E.size());
	std::vector<MPVariable*> f(E.size());

	for (std::size_t k = 0; k < E.size(); ++k) {
		int i = E[k].first;
		int j = E[k].second;

		// Nombres
		std::string x_name = "x_" + std::to_string(i) + "_" + std::to_string(j);
		std::string f_name = "f_" + std::to_string(i) + "_" + std::to_string(j);

		x[k] = solver->MakeBoolVar(x_name);
		f[k] = solver->MakeNumVar(0.0, inf, f_name);
	}

	// Variable objetivo
	MPVariable* E_max = solver->MakeNumVar(0.0, inf, "E_max");
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

	long long M = (N - 1) * g; // Big-M
	EdgeList E = compute_feasible_edges(coords, D_max); // enlaces posibles
}


// PYBIND MACRO
PYBIND11_MODULE(wsn_core, m) {
	m.doc() = "Modulo HPC para ruteo WSN";

	m.def("optimize_wsn_routing", &optimize_wsn_routing, "Resolver topologia WSN");
}
