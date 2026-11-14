#pragma once

constexpr int COUNT_BODIES = 8192;
constexpr int STEPS = 300;

// Передаем координаты тремя раздельными массивами double
extern "C" int runNBodySimulationRaw(double* host_x, double* host_y, double* host_z, int N, int steps);

