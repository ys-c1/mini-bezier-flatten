#ifndef MINI_BEZIER_FLATTEN_H
#define MINI_BEZIER_FLATTEN_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Flatten a 2D cubic Bezier curve into a polyline.
 *
 * Points are pairs of floats: {x, y}.  The output starts exactly at p0 and
 * ends exactly at p1.  Interior vertices are chosen adaptively so that the
 * polyline stays within tolerance of the curve.
 *
 * Returns 0 on success, -1 on invalid input or insufficient capacity.
 * On failure *out_count is set to 0.
 */
int flatten_cubic(const float *p0, const float *c0, const float *c1,
                  const float *p1, float tolerance, float *out,
                  int out_cap, int *out_count);

/*
 * Flatten a 2D quadratic Bezier curve into a polyline.
 * Same point layout and return conventions as flatten_cubic().
 */
int flatten_quad(const float *p0, const float *c0, const float *p1,
                 float tolerance, float *out, int out_cap, int *out_count);

#ifdef __cplusplus
}
#endif

#endif /* MINI_BEZIER_FLATTEN_H */
