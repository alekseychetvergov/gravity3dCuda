#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <math.h> 
#include <vector> // Добавлено для безопасного буфера на хосте

constexpr const double MASS = 10.0;
constexpr const double GRAVIT_CONST = 0.1;
constexpr const double TIME_STEP = 1.0;
constexpr const int THREADS_PER_BLOCK = 256;

// Кернел для расчета гравитационных сил
__global__ void computeGravityForcesKernel(const double3* __restrict__ positions,
    double3* __restrict__ forces, int N, double gravity)
{
    int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i >= N) return;

    double3 local_force = make_double3(0.0, 0.0, 0.0);
    double3 pos_i = positions[i];

    for (int j = 0; j < N; ++j) {
        if (i == j) continue;

        double3 pos_j = positions[j];
        double dRx = pos_j.x - pos_i.x;
        double dRy = pos_j.y - pos_i.y;
        double dRz = pos_j.z - pos_i.z;

        double distSqr = dRx * dRx + dRy * dRy + dRz * dRz;
        distSqr += 1e-9;

        // rsqrt — максимально производительный CUDA-интринсик вместо 1.0 / sqrt()
        double distInv = rsqrt(distSqr);
        double invDist3 = distInv * distInv * distInv;
        double force_mag = gravity * invDist3;

        local_force.x += dRx * force_mag;
        local_force.y += dRy * force_mag;
        local_force.z += dRz * force_mag;
    }
    forces[i] = local_force;
}

// Кернел для обновления скоростей и координат тел
__global__ void updatePositionsKernel(double3* __restrict__ positions,
    double3* __restrict__ velocities, const double3* __restrict__ forces,
    int N, double mass, double dt)
{
    int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i >= N) return;

    double3 f = forces[i];
    double3 v = velocities[i];
    double3 p = positions[i];

    v.x += (f.x / mass) * dt;
    v.y += (f.y / mass) * dt;
    v.z += (f.z / mass) * dt;

    p.x += v.x * dt;
    p.y += v.y * dt;
    p.z += v.z * dt;

    velocities[i] = v;
    positions[i] = p;
}

// Основная функция запуска симуляции на GPU, которую вызывает main.cpp
extern "C" int runNBodySimulationRaw(double* host_x, double* host_y, double* host_z, int N, int steps) {
    size_t sizeInBytes = N * sizeof(double3);
    double gravity_const_combined = GRAVIT_CONST * MASS * MASS;

    if (cudaSetDevice(0) != cudaSuccess) return -1;

    double3* dev_positions = nullptr;
    double3* dev_velocities = nullptr;
    double3* dev_forces = nullptr;

    if (cudaMalloc(&dev_positions, sizeInBytes) != cudaSuccess ||
        cudaMalloc(&dev_velocities, sizeInBytes) != cudaSuccess ||
        cudaMalloc(&dev_forces, sizeInBytes) != cudaSuccess) {
        cudaFree(dev_positions); cudaFree(dev_velocities); cudaFree(dev_forces);
        return -2;
    }

    // Безопасный RAII-контейнер std::vector вместо сырого указателя и new[]
    std::vector<double3> temp_host(N);
    for (int i = 0; i < N; ++i) {
        temp_host[i] = make_double3(host_x[i], host_y[i], host_z[i]);
    }

    cudaMemcpy(dev_positions, temp_host.data(), sizeInBytes, cudaMemcpyHostToDevice);
    cudaMemset(dev_velocities, 0, sizeInBytes);

    int blocksPerGrid = (N + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    for (int step = 0; step < steps; ++step) {
        cudaMemset(dev_forces, 0, sizeInBytes);

        // Обертка макросом __CUDACC__ скрывает синтаксис <<<>>> от парсера IntelliSense MSVS Insiders
#ifdef __CUDACC__
        computeGravityForcesKernel << <blocksPerGrid, THREADS_PER_BLOCK >> > (dev_positions, dev_forces, N, gravity_const_combined);
#endif
        cudaDeviceSynchronize();

#ifdef __CUDACC__
        updatePositionsKernel << <blocksPerGrid, THREADS_PER_BLOCK >> > (dev_positions, dev_velocities, dev_forces, N, MASS, TIME_STEP);
#endif
        cudaDeviceSynchronize();
    }

    cudaMemcpy(temp_host.data(), dev_positions, sizeInBytes, cudaMemcpyDeviceToHost);

    for (int i = 0; i < N; ++i) {
        host_x[i] = temp_host[i].x;
        host_y[i] = temp_host[i].y;
        host_z[i] = temp_host[i].z;
    }

    cudaFree(dev_positions);
    cudaFree(dev_velocities);
    cudaFree(dev_forces);
    return 0;
}
