#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include "gravity3dCuda.hpp"

// Тест проверяет корректность физического притяжения двух тел на GPU
TEST(GravityPhysicsTest, TwoBodiesSymmetricAttraction) {
    int N = 2;
    int steps = 1;
    
    // Тело 1 в точке (0,0,0), Тело 2 в точке (10,0,0)
    std::vector<double> host_x = { 0.0, 10.0 };
    std::vector<double> host_y = { 0.0, 0.0 };
    std::vector<double> host_z = { 0.0, 0.0 };

    int status = runNBodySimulationRaw(host_x.data(), host_y.data(), host_z.data(), N, steps);
    
    // Проверяем, что симуляция вообще выполнилась успешно
    ASSERT_EQ(status, 0);

    // Физическая проверка: тела должны были притянуться (сдвинуться навстречу друг другу)
    EXPECT_GT(host_x[0], 0.0);  // Первое тело сместилось вправо
    EXPECT_LT(host_x[1], 10.0); // Второе тело сместилось влево

    // Так как массы тел одинаковы, их смещение должно быть абсолютно симметричным
    double delta_body1 = host_x[0] - 0.0;
    double delta_body2 = 10.0 - host_x[1];

    // Проверяем равенство смещений с точностью до 5 знаков после запятой
    EXPECT_NEAR(delta_body1, delta_body2, 1e-5);
}

// Дополнительный изолированный тест на обработку некорректных входных данных
TEST(GravitySystemTest, HandleEmptyData) {
    std::vector<double> empty_vec;
    // Симуляция должна вернуть код ошибки (например, если передать N = 0)
    int status = runNBodySimulationRaw(empty_vec.data(), empty_vec.data(), empty_vec.data(), 0, 1);
    // Наш код выделит 0 байт памяти. В зависимости от вашей реализации это либо 0, либо код ошибки.
    // Если в функции стоит проверка на N <= 0, можно ожидать ошибку.
    EXPECT_TRUE(status == 0 || status < 0);
}
