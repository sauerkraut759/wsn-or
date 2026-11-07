# Optimización de Energía en Redes WSN (IoT)

Este proyecto implementa un modelo de Programación Lineal Entera Mixta (MILP) para maximizar la vida útil de una Red de Sensores Inalámbricos (WSN) basada en hardware Texas Instruments CC2652R. 

El sistema utiliza una arquitectura híbrida de alto rendimiento:
* **Backend (C++ / OR-Tools / SCIP):** Construcción del grafo de ruteo y resolución matemática del modelo Min-Max de consumo energético, garantizando la optimalidad y evitando cuellos de botella computacionales. Expuesto mediante `pybind11`.
* **Frontend (Python):** Interfaz para generación de parámetros, manejo de simulaciones y visualización topológica (NetworkX / Matplotlib).

## Requisitos Previos

Independientemente del sistema operativo, necesitas tener instalado:
* **Python 3.8+**
* **CMake** (v3.15+)
* Compilador de C++:
  * **Linux:** `gcc` o `clang` (se recomienda usar `ninja` para compilar más rápido).
  * **Windows:** MSVC (Visual Studio Build Tools con carga de trabajo "Desarrollo para el escritorio con C++").

## Instrucciones de Compilación (Motor C++)

Clona este repositorio y navega a la carpeta raíz del proyecto.

### En Linux (Arch/Ubuntu/etc.)
```bash
mkdir build
cd build
cmake -GNinja ..
ninja
cd ..
```
### En Windows
```
mkdir build
cd build
cmake ..
cmake --build . --config Release
cd ..
```

