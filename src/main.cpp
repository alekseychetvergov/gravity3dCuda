#include <iostream>
#include <vector>
#include <iomanip>
#include <random>
#include <chrono>
#include "gravity3dCuda.hpp" 

int main() {
    int N = COUNT_BODIES;

    // Три чистых вектора STL для хоста
    std::vector<double> posX(N);
    std::vector<double> posY(N);
    std::vector<double> posZ(N);

    std::mt19937 prng(1337);
    std::uniform_real_distribution<double> dist(-100.0, -50.0);

    for (int k = 0; k < N; ++k) {
        double randC = dist(prng);
        posX[k] = randC + 0.2 * k;
        posY[k] = randC - 0.3 * k;
        posZ[k] = randC + 0.4 * k;
    }

    std::cout << "Starting simulation for " << N << " bodies on GPU (Insiders Isolated Mode)...\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    // Передаем указатели на сырые массивы векторов
    int status = runNBodySimulationRaw(posX.data(), posY.data(), posZ.data(), N, STEPS);

    if (status != 0) {
        std::cerr << "Error during simulation! Code: " << status << std::endl;
        return 1;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "Simulation completed in: " << elapsed.count() << " seconds\n\n";

    std::cout << std::fixed << std::setprecision(5);
    std::cout << "First 5 bodies positions:\n";
    for (int i = 0; i < 5; ++i) {
        std::cout << "Body " << i << ": [" << posX[i] << ", " << posY[i] << ", " << posZ[i] << "]\n";
    }

    return 0;
}
