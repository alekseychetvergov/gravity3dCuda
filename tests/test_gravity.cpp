#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include <numeric>
#include "gravity3dCuda.hpp"

// 1. Тест третьего закона Ньютона (симметричное притяжение двух тел)
TEST(GravityPhysicsTest, TwoBodiesSymmetricAttraction) {
    int N = 2;
    int steps = 1;

    std::vector<double> host_x = { 0.0, 10.0 };
    std::vector<double> host_y = { 0.0, 0.0 };
    std::vector<double> host_z = { 0.0, 0.0 };

    int status = runNBodySimulationRaw(host_x.data(), host_y.data(), host_z.data(), N, steps);
    ASSERT_EQ(status, 0);

    // Тела должны сдвинуться навстречу друг другу
    EXPECT_GT(host_x[0], 0.0);
    EXPECT_LT(host_x[1], 10.0);

    // Смещения должны быть равны по модулю
    double delta_body1 = host_x[0] - 0.0;
    double delta_body2 = 10.0 - host_x[1];
    EXPECT_NEAR(delta_body1, delta_body2, 1e-7);
}

// 2. Тест закона сохранения центра масс (импульса) для системы многих тел
TEST(GravityPhysicsTest, CenterOfMassConservation) {
    int N = 4;
    int steps = 5;

    // Задаем несимметричную систему в 3D-пространстве
    std::vector<double> host_x = { -5.0,  2.0,  10.0, -7.0 };
    std::vector<double> host_y = { 3.0, -8.0,   4.0,  1.0 };
    std::vector<double> host_z = { 0.0,  6.0, -12.0,  6.0 };

    // Начальный центр масс равен (0, 0, 0), так как сумма координат по каждой оси = 0
    int status = runNBodySimulationRaw(host_x.data(), host_y.data(), host_z.data(), N, steps);
    ASSERT_EQ(status, 0);

    // В замкнутой системе без внешних сил центр масс должен остаться неподвижным
    double sum_x = std::accumulate(host_x.begin(), host_x.end(), 0.0);
    double sum_y = std::accumulate(host_y.begin(), host_y.end(), 0.0);
    double sum_z = std::accumulate(host_z.begin(), host_z.end(), 0.0);

    EXPECT_NEAR(sum_x / N, 0.0, 1e-6);
    EXPECT_NEAR(sum_y / N, 0.0, 1e-6);
    EXPECT_NEAR(sum_z / N, 0.0, 1e-6);
}

// 3. Тест суперпозиции сил в 3D (тело в центре симметрии не должно двигаться)
TEST(GravityPhysicsTest, EquilibriumAtCenterOfSymmetry) {
    int N = 7;
    int steps = 3;

    // Тело 0 в центре (0,0,0), остальные 6 образуют правильный октаэдр вокруг него
    std::vector<double> host_x = { 0.0,  5.0, -5.0,  0.0,  0.0,  0.0,  0.0 };
    std::vector<double> host_y = { 0.0,  0.0,  0.0,  5.0, -5.0,  0.0,  0.0 };
    std::vector<double> host_z = { 0.0,  0.0,  0.0,  0.0,  0.0,  5.0, -5.0 };

    int status = runNBodySimulationRaw(host_x.data(), host_y.data(), host_z.data(), N, steps);
    ASSERT_EQ(status, 0);

    // Силы со всех 6 сторон компенсируют друг друга, центральное тело остается на месте
    EXPECT_NEAR(host_x[0], 0.0, 1e-7);
    EXPECT_NEAR(host_y[0], 0.0, 1e-7);
    EXPECT_NEAR(host_z[0], 0.0, 1e-7);
}

// 4. Тест одиночного тела (отсутствие самодействия в цикле j != i)
TEST(GravityEdgeCasesTest, SingleBodyRemainsStationary) {
    int N = 1;
    int steps = 10;

    std::vector<double> host_x = { 42.5 };
    std::vector<double> host_y = { -13.1 };
    std::vector<double> host_z = { 7.8 };

    int status = runNBodySimulationRaw(host_x.data(), host_y.data(), host_z.data(), N, steps);
    ASSERT_EQ(status, 0);

    // Одиночное тело в пустом пространстве не должно менять координаты
    EXPECT_DOUBLE_EQ(host_x[0], 42.5);
    EXPECT_DOUBLE_EQ(host_y[0], -13.1);
    EXPECT_DOUBLE_EQ(host_z[0], 7.8);
}

// 5. Тест нулевого количества шагов симуляции
TEST(GravityEdgeCasesTest, ZeroStepsNoMovement) {
    int N = 2;
    int steps = 0;

    std::vector<double> host_x = { 1.0, 2.0 };
    std::vector<double> host_y = { 3.0, 4.0 };
    std::vector<double> host_z = { 5.0, 6.0 };

    int status = runNBodySimulationRaw(host_x.data(), host_y.data(), host_z.data(), N, steps);
    ASSERT_EQ(status, 0);

    // При 0 шагах координаты должны остаться строго исходными
    EXPECT_DOUBLE_EQ(host_x[0], 1.0);
    EXPECT_DOUBLE_EQ(host_x[1], 2.0);
}

// 6. Тест численной стабильности при совпадении координат (проверка softening factor 1e-9)
TEST(GravityStabilityTest, CoincidentBodiesNoNaN) {
    int N = 2;
    int steps = 2;

    // Два тела помещены в абсолютно одну точку (0, 0, 0)
    std::vector<double> host_x = { 0.0, 0.0 };
    std::vector<double> host_y = { 0.0, 0.0 };
    std::vector<double> host_z = { 0.0, 0.0 };

    int status = runNBodySimulationRaw(host_x.data(), host_y.data(), host_z.data(), N, steps);
    ASSERT_EQ(status, 0);

    // Благодаря фактору сглаживания (1e-9) не должно возникнуть деления на ноль (NaN или Inf)
    EXPECT_FALSE(std::isnan(host_x[0]));
    EXPECT_FALSE(std::isinf(host_x[0]));
    EXPECT_DOUBLE_EQ(host_x[0], 0.0);
}

// 7. Тест на корректность работы с несколькими блоками потоков (N > 256, не кратное 256)
TEST(GravityCudaArchitectureTest, MultiBlockBoundaryCheck) {
    int N = 300; // Больше одного блока (256), но не кратно ему
    int steps = 2;

    std::vector<double> host_x(N, 0.0);
    std::vector<double> host_y(N, 0.0);
    std::vector<double> host_z(N, 0.0);

    // Размещаем тела вдоль оси X на равном расстоянии
    for (int i = 0; i < N; ++i) {
        host_x[i] = static_cast<double>(i);
    }

    int status = runNBodySimulationRaw(host_x.data(), host_y.data(), host_z.data(), N, steps);
    ASSERT_EQ(status, 0);

    // Проверяем крайние тела (индекс 0 в первом блоке и индекс 299 во втором блоке)
    // Они должны сдвинуться строго симметрично навстречу центру массива
    double shift_first = host_x[0] - 0.0;
    double shift_last = 299.0 - host_x[299];

    EXPECT_GT(shift_first, 0.0);
    EXPECT_NEAR(shift_first, shift_last, 1e-6);
}

// 8. Тест проверки закона обратных квадратов (при увеличении дистанции в 2 раза сила падает в 4 раза)
TEST(GravityPhysicsTest, InverseSquareLawValidation) {
    int N = 2;
    int steps = 1;

    // Случай А: расстояние между телами = 10.0
    std::vector<double> x1 = { 0.0, 10.0 }, y1 = { 0.0, 0.0 }, z1 = { 0.0, 0.0 };
    ASSERT_EQ(runNBodySimulationRaw(x1.data(), y1.data(), z1.data(), N, steps), 0);
    double shift_dist_10 = x1[0];

    // Случай Б: расстояние между телами = 20.0 (в 2 раза больше)
    std::vector<double> x2 = { 0.0, 20.0 }, y2 = { 0.0, 0.0 }, z2 = { 0.0, 0.0 };
    ASSERT_EQ(runNBodySimulationRaw(x2.data(), y2.data(), z2.data(), N, steps), 0);
    double shift_dist_20 = x2[0];

    // Отношение смещений должно быть равно 4.0 (с учетом пренебрежимо малого softening factor)
    EXPECT_NEAR(shift_dist_10 / shift_dist_20, 4.0, 1e-4);
}

// 9. Тест детерминированности (отсутствие гонок данных / race conditions на GPU)
TEST(GravityCudaArchitectureTest, DeterministicResults) {
    int N = 512;
    int steps = 5;

    std::vector<double> x_run1(N), y_run1(N), z_run1(N);
    for (int i = 0; i < N; ++i) {
        x_run1[i] = i * 1.5;
        y_run1[i] = -i * 0.5;
        z_run1[i] = i * 0.25;
    }

    // Делаем точную копию начальных данных для второго запуска
    std::vector<double> x_run2 = x_run1;
    std::vector<double> y_run2 = y_run1;
    std::vector<double> z_run2 = z_run1;

    ASSERT_EQ(runNBodySimulationRaw(x_run1.data(), y_run1.data(), z_run1.data(), N, steps), 0);
    ASSERT_EQ(runNBodySimulationRaw(x_run2.data(), y_run2.data(), z_run2.data(), N, steps), 0);

    // Результаты обоих запусков должны совпадать побитово (EXPECT_DOUBLE_EQ)
    for (int i = 0; i < N; ++i) {
        EXPECT_DOUBLE_EQ(x_run1[i], x_run2[i]);
        EXPECT_DOUBLE_EQ(y_run1[i], y_run2[i]);
        EXPECT_DOUBLE_EQ(z_run1[i], z_run2[i]);
    }
}
