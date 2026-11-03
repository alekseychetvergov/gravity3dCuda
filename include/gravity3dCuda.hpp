#pragma once

constexpr int COUNT_BODIES = 2048;
constexpr int STEPS = 10;

// Передаем координаты тремя раздельными массивами double
extern "C" int runNBodySimulationRaw(double* host_x, double* host_y, double* host_z, int N, int steps);

