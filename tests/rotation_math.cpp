#include <gtest/gtest.h>

#include <numbers>

#include "glvm/glvm.hpp"

TEST(RotationMathTests, YawZeroIsIdentity) {
    const auto matrix = rotation_yaw_matrix(0.0f);
    for (i32 i = 0; i < 4; ++i) {
        for (i32 j = 0; j < 4; ++j) {
            if (i == j) {
                EXPECT_FLOAT_EQ(matrix[i][j], 1.0f);
            } else {
                EXPECT_FLOAT_EQ(matrix[i][j], 0.0f);
            }
        }
    }
}

TEST(RotationMathTests, YawTurnsRestForwardTowardMovement) {
    // Facing scenario from third-person movement: rest forward (0, 0, -1)
    // rotated by -pi/2 must face (1, 0, 0). Applied column-vector style,
    // like the renderer shader does with model matrices.
    const auto matrix =
        rotation_yaw_matrix(-std::numbers::pi_v<f32> / 2.0f);
    const auto x =
        matrix[0][0] * 0.0f + matrix[1][0] * 0.0f + matrix[2][0] * -1.0f;
    const auto y =
        matrix[0][1] * 0.0f + matrix[1][1] * 0.0f + matrix[2][1] * -1.0f;
    const auto z =
        matrix[0][2] * 0.0f + matrix[1][2] * 0.0f + matrix[2][2] * -1.0f;
    EXPECT_NEAR(x, 1.0f, 1e-6f);
    EXPECT_NEAR(y, 0.0f, 1e-6f);
    EXPECT_NEAR(z, 0.0f, 1e-6f);
}

TEST(RotationMathTests, Cross2DSign) {
    const auto sign = cross(
        Vector<f32, 2>(1.0f, 0.0f),
        Vector<f32, 2>(0.0f, -1.0f)
    );
    EXPECT_FLOAT_EQ(sign, -1.0f);
}
