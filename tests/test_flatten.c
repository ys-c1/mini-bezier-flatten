#include "flatten.h"

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define POLY_CAP 4096
#define GUARD_BYTES 64

static int g_checks;
static int g_failures;

#define CHECK(condition)                                                    \
    do {                                                                    \
        ++g_checks;                                                         \
        if (!(condition)) {                                                 \
            ++g_failures;                                                   \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);     \
        }                                                                   \
    } while (0)

static int guard_intact(const unsigned char *memory, size_t size,
                        unsigned char sentinel)
{
    size_t i;

    for (i = 0; i < size; ++i) {
        if (memory[i] != sentinel) {
            return 0;
        }
    }
    return 1;
}

static int all_points_inside(const float *polyline, int count,
                             float min_x, float max_x,
                             float min_y, float max_y, float epsilon)
{
    int i;

    for (i = 0; i < count; ++i) {
        float x = polyline[2 * i];
        float y = polyline[2 * i + 1];

        if (!(x >= min_x - epsilon && x <= max_x + epsilon &&
              y >= min_y - epsilon && y <= max_y + epsilon)) {
            return 0;
        }
    }
    return 1;
}

static void test_cubic_line_is_two_points(void)
{
    const float p0[2] = {0.0f, 0.0f};
    const float c0[2] = {25.0f, 25.0f};
    const float c1[2] = {75.0f, 75.0f};
    const float p1[2] = {100.0f, 100.0f};
    float out[POLY_CAP * 2] = {0.0f};
    int count = -1;
    int rc;

    rc = flatten_cubic(p0, c0, c1, p1, 0.01f, out, POLY_CAP, &count);

    CHECK(rc == 0);
    CHECK(count == 2);
    CHECK(count >= 1 && out[0] == p0[0]);
    CHECK(count >= 1 && out[1] == p0[1]);
    CHECK(count >= 2 && out[2 * (count - 1)] == p1[0]);
    CHECK(count >= 2 && out[2 * (count - 1) + 1] == p1[1]);
}

static void test_quad_line_is_two_points(void)
{
    const float p0[2] = {-40.0f, 10.0f};
    const float c0[2] = {-10.0f, 22.0f};
    const float p1[2] = {60.0f, 50.0f};
    float out[POLY_CAP * 2] = {0.0f};
    int count = -1;
    int rc;

    rc = flatten_quad(p0, c0, p1, 0.02f, out, POLY_CAP, &count);

    CHECK(rc == 0);
    CHECK(count == 2);
    CHECK(count >= 1 && out[0] == p0[0]);
    CHECK(count >= 1 && out[1] == p0[1]);
    CHECK(count >= 2 && out[2 * (count - 1)] == p1[0]);
    CHECK(count >= 2 && out[2 * (count - 1) + 1] == p1[1]);
}

static void test_smaller_tolerance_adds_points(void)
{
    const float p0[2] = {0.0f, 0.0f};
    const float c0[2] = {0.0f, 80.0f};
    const float c1[2] = {100.0f, 80.0f};
    const float p1[2] = {100.0f, 0.0f};
    float coarse[POLY_CAP * 2] = {0.0f};
    float fine[POLY_CAP * 2] = {0.0f};
    int coarse_count = -1;
    int fine_count = -1;
    int rc_coarse;
    int rc_fine;

    rc_coarse = flatten_cubic(p0, c0, c1, p1, 2.0f,
                              coarse, POLY_CAP, &coarse_count);
    rc_fine = flatten_cubic(p0, c0, c1, p1, 0.01f,
                            fine, POLY_CAP, &fine_count);

    CHECK(rc_coarse == 0);
    CHECK(rc_fine == 0);
    CHECK(coarse_count >= 3);
    CHECK(fine_count > coarse_count);
}

static void test_cubic_endpoints_are_exact(void)
{
    const float p0[2] = {3.25f, -7.5f};
    const float c0[2] = {24.0f, 61.0f};
    const float c1[2] = {81.0f, -33.0f};
    const float p1[2] = {107.75f, 19.25f};
    float out[POLY_CAP * 2] = {0.0f};
    int count = -1;
    int rc;

    rc = flatten_cubic(p0, c0, c1, p1, 0.05f, out, POLY_CAP, &count);

    CHECK(rc == 0);
    CHECK(count >= 2);
    CHECK(count >= 1 && out[0] == p0[0]);
    CHECK(count >= 1 && out[1] == p0[1]);
    CHECK(count >= 2 && out[2 * (count - 1)] == p1[0]);
    CHECK(count >= 2 && out[2 * (count - 1) + 1] == p1[1]);
}

static void test_quad_endpoints_are_exact(void)
{
    const float p0[2] = {-31.5f, 4.75f};
    const float c0[2] = {17.0f, 92.0f};
    const float p1[2] = {73.5f, -18.25f};
    float out[POLY_CAP * 2] = {0.0f};
    int count = -1;
    int rc;

    rc = flatten_quad(p0, c0, p1, 0.05f, out, POLY_CAP, &count);

    CHECK(rc == 0);
    CHECK(count >= 2);
    CHECK(count >= 1 && out[0] == p0[0]);
    CHECK(count >= 1 && out[1] == p0[1]);
    CHECK(count >= 2 && out[2 * (count - 1)] == p1[0]);
    CHECK(count >= 2 && out[2 * (count - 1) + 1] == p1[1]);
}

static void test_cubic_samples_stay_in_bounds(void)
{
    const float p0[2] = {0.0f, 0.0f};
    const float c0[2] = {-20.0f, 80.0f};
    const float c1[2] = {120.0f, 60.0f};
    const float p1[2] = {100.0f, 20.0f};
    const float tolerance = 0.05f;
    const float epsilon = tolerance + 0.001f;
    float out[POLY_CAP * 2] = {0.0f};
    int count = -1;
    int rc;

    rc = flatten_cubic(p0, c0, c1, p1, tolerance, out, POLY_CAP, &count);

    CHECK(rc == 0);
    CHECK(count > 0);
    CHECK(all_points_inside(out, count, -20.0f, 120.0f, 0.0f, 80.0f,
                            epsilon));
}

static void test_quad_samples_stay_in_bounds(void)
{
    const float p0[2] = {-10.0f, 10.0f};
    const float c0[2] = {50.0f, 100.0f};
    const float p1[2] = {110.0f, 0.0f};
    const float tolerance = 0.05f;
    const float epsilon = tolerance + 0.001f;
    float out[POLY_CAP * 2] = {0.0f};
    int count = -1;
    int rc;

    rc = flatten_quad(p0, c0, p1, tolerance, out, POLY_CAP, &count);

    CHECK(rc == 0);
    CHECK(count > 0);
    CHECK(all_points_inside(out, count, -10.0f, 110.0f, 0.0f, 100.0f,
                            epsilon));
}

static void test_capacity_failure_does_not_overrun(void)
{
    const float p0[2] = {0.0f, 0.0f};
    const float c0[2] = {20.0f, 20.0f};
    const float c1[2] = {80.0f, 20.0f};
    const float p1[2] = {100.0f, 0.0f};
    const size_t payload_bytes = 2u * sizeof(float);
    _Alignas(16) unsigned char storage[GUARD_BYTES + 2u * sizeof(float) +
                                       GUARD_BYTES];
    float *out = (float *)(void *)(storage + GUARD_BYTES);
    unsigned char *after = storage + GUARD_BYTES + payload_bytes;
    int count = -1;
    int rc;

    memset(storage, 0xA5, sizeof(storage));
    rc = flatten_cubic(p0, c0, c1, p1, 0.01f, out, 1, &count);

    CHECK(rc == -1);
    CHECK(count == 0);
    CHECK(guard_intact(storage, GUARD_BYTES, 0xA5));
    CHECK(guard_intact(after, GUARD_BYTES, 0xA5));
}

static void test_invalid_tolerance(void)
{
    const float p0[2] = {0.0f, 0.0f};
    const float c0[2] = {10.0f, 20.0f};
    const float c1[2] = {30.0f, 20.0f};
    const float p1[2] = {40.0f, 0.0f};
    float out[64] = {0.0f};
    int count = -1;

    CHECK(flatten_cubic(p0, c0, c1, p1, 0.0f, out, 32, &count) == -1);
    CHECK(count == 0);

    count = -1;
    CHECK(flatten_quad(p0, c0, p1, -0.5f, out, 32, &count) == -1);
    CHECK(count == 0);
}

int main(void)
{
    printf("mini-bezier-flatten tests\n");

    test_cubic_line_is_two_points();
    test_quad_line_is_two_points();
    test_smaller_tolerance_adds_points();
    test_cubic_endpoints_are_exact();
    test_quad_endpoints_are_exact();
    test_cubic_samples_stay_in_bounds();
    test_quad_samples_stay_in_bounds();
    test_capacity_failure_does_not_overrun();
    test_invalid_tolerance();

    printf("checks=%d failures=%d\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
