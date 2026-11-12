#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <cstdint>
#include <stdfloat>
#include <vector>
#include <string>
#include <map>
#include <tuple>
#include <utility>
#include <cstddef>
#include <stdexcept>
#include <memory>

#include "ortools/linear_solver/linear_solver.h"

// Tamaños fijos para compatibilidad ILP64 LLP64 etc
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using f32 = std::float32_t;
using f64 = std::float64_t;

namespace py = pybind11;
namespace ort = operations_research;

using EdgeList = std::vector<std::pair<i32, i32>>;
using Node = std::pair<f64, f64>;

struct MilpContext {
	std::unique_ptr<ort::MPSolver> solver;
	std::vector<ort::MPVariable*> x;
	std::vector<ort::MPVariable*> f;
	ort::MPVariable* E_max = nullptr;
};

inline f64 dist_sq(const Node& A, const Node& B) noexcept
{
	f64 dx = B.first - A.first;
	f64 dy = B.second - B.second;
	return (dx*dx + dy*dy);
}

EdgeList compute_feasible_edges(const std::vector<Node>& coords, f64 D_max)
{
	EdgeList E;
	const std::size_t N = coords.size();

	if (N <= 1) return E;

	E.reserve(N * (N - 1) / 2); //Memoria aproximada para evitar realloc

	const f64 D_max_sq = D_max * D_max;

	for (std::size_t i = 1; i < N; i++) {
		for (std::size_t j = 0; j < N; j++) {
			if (i == j || dist_sq(coords[i], coords[j]) > D_max_sq) continue;

			E.emplace_back(static_cast<i32>(i), static_cast<i32>(j));
		}
	}

	return E;
}

MilpContext build_milp_model(std::size_t N, const EdgeList& E, i32 g, f64 e_tx, f64 e_rx)
{
	using ort::MPSolver;
	using ort::MPVariable;
	using ort::MPConstraint;

	std::unique_ptr<MPSolver> solver(MPSolver::CreateSolver("SCIP"));
	if (!solver) {
		throw std::runtime_error("Error al instanciar solver SCIP");
	}

	const f64 inf = solver->infinity();
	const f64 M = (N - 1) * g; // Big-M 

	/* ----- Variables ----- */

	std::vector<MPVariable*> x(E.size());
	std::vector<MPVariable*> f(E.size());

	// Direccion del flujo de cada nodo
	std::vector<std::vector<std::size_t>> incoming(N);
	std::vector<std::vector<std::size_t>> outgoing(N);

	for (std::size_t k = 0; k < E.size(); ++k) {
		i32 i = E[k].first;
		i32 j = E[k].second;

		// Nombres
		std::string x_name = "x_" + std::to_string(i) + "_" + std::to_string(j);
		std::string f_name = "f_" + std::to_string(i) + "_" + std::to_string(j);

		x[k] = solver->MakeBoolVar(x_name);
		f[k] = solver->MakeNumVar(0.0, inf, f_name);

		outgoing[E[k].first].push_back(k);
		incoming[E[k].second].push_back(k);
	}

	// Variable objetivo
	MPVariable* E_max = solver->MakeNumVar(0.0, inf, "E_max");

	/* ----- Restricciones ----- */

	// Iterar sobre conjunto S = V\{0}
	for (std::size_t i = 1; i < N; ++i) {
		MPConstraint* c_assign  = solver->MakeRowConstraint(1.0, 1.0, "assign_node_" + std::to_string(i));
		MPConstraint* c_balance = solver->MakeRowConstraint(static_cast<f64>(-g), static_cast<f64>(-g), "balance_node_" + std::to_string(i));
		MPConstraint* c_energy  = solver->MakeRowConstraint(-inf, 0.0, "energy_node_" + std::to_string(i));

		c_energy->SetCoefficient(E_max, -1.0);

		// Asigna coeficientes solo en los indices k donde el nodo i transmite datos.
		for (std::size_t k_out : outgoing[i]) {
			c_assign->SetCoefficient(x[k_out], 1.0);
			c_balance->SetCoefficient(f[k_out], -1.0);
			c_energy->SetCoefficient(f[k_out], e_tx);
		}
		// Asigna coeficientes solo en los indices k donde el nodo i recibe datos.
		for (std::size_t k_in : incoming[i]) {
			c_balance->SetCoefficient(f[k_in], 1.0);
			c_energy->SetCoefficient(f[k_in], e_rx);
		}
	}

	// Iterar sobre enlaces E
	for (std::size_t k = 0; k < E.size(); ++k) {
		MPConstraint* c_bigm = solver->MakeRowConstraint(-inf, 0.0, "bigm_edge_" + std::to_string(k));
		c_bigm->SetCoefficient(f[k], 1.0);
		c_bigm->SetCoefficient(x[k], -M);
	}

	/* ----- Configurar funcion objetivo ----- */

	ort::MPObjective* const objective = solver->MutableObjective();
	objective->SetCoefficient(E_max, 1.0);
	objective->SetMinimization();

	/* ----- Retorno ----- */

	/*
	 * Aunque retornar por valor aprovecharia NRVO, usar variables locales
	 * evita el overhead generado por tener que recargar los registros cada vez
	 * que se llama a funciones opacas de ortools.
	 *
	 * Además es más simple para el compilador con -O3 optimizar este codigo
	 *
	 * Probablemente al final solo me estoy ahorrando un par de ciclos de CPU
	 */
	 return {
		.solver = std::move(solver),
		.x      = std::move(x),
		.f      = std::move(f),
		.E_max  = E_max
	};
}

std::tuple<f64, EdgeList> optimize_wsn_routing(std::size_t N, std::vector<Node> coords, f64 D_max, i32 g, f64 e_tx, f64 e_rx)
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

	i64 M = (N - 1) * g; // Big-M
	EdgeList E = compute_feasible_edges(coords, D_max); // enlaces posibles
}


// PYBIND MACRO
PYBIND11_MODULE(wsn_core, m) {
	m.doc() = "Modulo HPC para ruteo WSN";

	m.def("optimize_wsn_routing", &optimize_wsn_routing, "Resolver topologia WSN");
}
