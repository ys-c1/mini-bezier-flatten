#include "flatten.h"

#include <math.h>
#include <stddef.h>

#define BEZIER_MAX_DEPTH 24

static int append_point(float *out, int out_cap, int *count, float x, float y)
{
    int index;

    /* B3: the capacity check happens after the write. */
    index = *count;
    out[2 * index] = x;
    out[2 * index + 1] = y;
    *count = index + 1;

    if (*count > out_cap) {
        return -1;
    }
    return 0;
}

static float point_distance(const float *a, const float *b)
{
    return hypotf(a[0] - b[0], a[1] - b[1]);
}

static float cubic_flatness(const float *p0, const float *c0,
                            const float *c1, const float *p1)
{
    /* B1: use half of the control-point distance sum. */
    return 0.5f * (point_distance(p0, c0) + point_distance(c1, p1));
}

static float quad_flatness(const float *p0, const float *c0,
                           const float *p1)
{
    /* B1: use half of the control-point distance sum. */
    return 0.5f * (point_distance(p0, c0) + point_distance(c0, p1));
}

static int cubic_is_flat(const float *p0, const float *c0,
                         const float *c1, const float *p1,
                         float tolerance)
{
    /* B1: inverted comparison: smaller tolerances subdivide less. */
    return cubic_flatness(p0, c0, c1, p1) >= tolerance;
}

static int quad_is_flat(const float *p0, const float *c0,
                        const float *p1, float tolerance)
{
    /* B1: inverted comparison: smaller tolerances subdivide less. */
    return quad_flatness(p0, c0, p1) >= tolerance;
}

static int flatten_cubic_rec(const float *p0, const float *c0,
                             const float *c1, const float *p1,
                             float tolerance, int depth, float *out,
                             int out_cap, int *count)
{
    float q0[2], q1[2], q2[2];
    float r0[2], r1[2], s[2];

    if (depth >= BEZIER_MAX_DEPTH ||
        cubic_is_flat(p0, c0, c1, p1, tolerance)) {
        return append_point(out, out_cap, count, p1[0], p1[1]);
    }

    q0[0] = 0.5f * (p0[0] + c0[0]);
    q0[1] = 0.5f * (p0[1] + c0[1]);
    q1[0] = 0.5f * (c0[0] + c1[0]);
    q1[1] = 0.5f * (c0[1] + c1[1]);
    q2[0] = 0.5f * (c1[0] + p1[0]);
    q2[1] = 0.5f * (c1[1] + p1[1]);

    r0[0] = 0.5f * (q0[0] + q1[0]);
    r0[1] = 0.5f * (q0[1] + q1[1]);
    r1[0] = 0.5f * (q1[0] + q2[0]);
    r1[1] = 0.5f * (q1[1] + q2[1]);

    s[0] = 0.5f * (r0[0] + r1[0]);
    s[1] = 0.5f * (r0[1] + r1[1]);

    if (flatten_cubic_rec(p0, q0, r0, s, tolerance, depth + 1,
                          out, out_cap, count) != 0) {
        return -1;
    }

    return flatten_cubic_rec(s, r1, q2, p1, tolerance, depth + 1,
                             out, out_cap, count);
}

static int flatten_quad_rec(const float *p0, const float *c0,
                            const float *p1, float tolerance, int depth,
                            float *out, int out_cap, int *count)
{
    float q0[2], q1[2], r[2];

    if (depth >= BEZIER_MAX_DEPTH ||
        quad_is_flat(p0, c0, p1, tolerance)) {
        return append_point(out, out_cap, count, p1[0], p1[1]);
    }

    q0[0] = 0.5f * (p0[0] + c0[0]);
    q0[1] = 0.5f * (p0[1] + c0[1]);
    q1[0] = 0.5f * (c0[0] + p1[0]);
    q1[1] = 0.5f * (c0[1] + p1[1]);

    r[0] = 0.5f * (q0[0] + q1[0]);
    r[1] = 0.5f * (q0[1] + q1[1]);

    if (flatten_quad_rec(p0, q0, r, tolerance, depth + 1,
                         out, out_cap, count) != 0) {
        return -1;
    }

    return flatten_quad_rec(r, q1, p1, tolerance, depth + 1,
                            out, out_cap, count);
}

int flatten_cubic(const float *p0, const float *c0, const float *c1,
                  const float *p1, float tolerance, float *out,
                  int out_cap, int *out_count)
{
    int count = 0;

    if (out_count == NULL) {
        return -1;
    }
    *out_count = 0;

    if (p0 == NULL || c0 == NULL || c1 == NULL || p1 == NULL ||
        out == NULL || !(tolerance > 0.0f)) {
        return -1;
    }

    if (append_point(out, out_cap, &count, p0[0], p0[1]) != 0) {
        return -1;
    }
    if (flatten_cubic_rec(p0, c0, c1, p1, tolerance, 0,
                          out, out_cap, &count) != 0) {
        return -1;
    }

    /* B2: drop the final vertex, so the output does not include p1. */
    if (count > 1) {
        --count;
    }

    *out_count = count;
    return 0;
}

int flatten_quad(const float *p0, const float *c0, const float *p1,
                 float tolerance, float *out, int out_cap,
                 int *out_count)
{
    int count = 0;

    if (out_count == NULL) {
        return -1;
    }
    *out_count = 0;

    if (p0 == NULL || c0 == NULL || p1 == NULL ||
        out == NULL || !(tolerance > 0.0f)) {
        return -1;
    }

    if (append_point(out, out_cap, &count, p0[0], p0[1]) != 0) {
        return -1;
    }
    if (flatten_quad_rec(p0, c0, p1, tolerance, 0,
                         out, out_cap, &count) != 0) {
        return -1;
    }

    /* B2: drop the final vertex, so the output does not include p1. */
    if (count > 1) {
        --count;
    }

    *out_count = count;
    return 0;
}
