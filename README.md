# mini-bezier-flatten

Small C11 library that flattens 2D quadratic and cubic Bezier curves into
polylines with adaptive vertex spacing.

## API

Points are pairs of `float` values (`x`, `y`). The public API is declared in
`src/flatten.h`:

```c
int flatten_cubic(const float *p0, const float *c0, const float *c1,
                  const float *p1, float tolerance, float *out,
                  int out_cap, int *out_count);
int flatten_quad(const float *p0, const float *c0, const float *p1,
                 float tolerance, float *out, int out_cap, int *out_count);
```

The output always starts at `p0` and ends at `p1`. `out_cap` is the number of
2D points that fit in `out`; the buffer must therefore hold
`out_cap * 2` floats. A return value of `0` means success. Invalid arguments,
`tolerance <= 0`, or insufficient capacity produce `-1`, and `*out_count` is
set to `0`. No write is allowed beyond `out_cap` points.

The implementation recursively subdivides each curve with de Casteljau's
algorithm. A segment is accepted when both control points lie within
`tolerance` of its chord. This gives two vertices for a collinear curve and
more vertices as the tolerance gets tighter.

## Build and test

```sh
make test
```

`make test` compiles `src/flatten.c` with the C11 test program and runs it.
The test executable is written to `build/test_flatten`. Use `make clean` to
remove build output.

## Files

- `src/flatten.h` - public declarations and ownership conventions
- `src/flatten.c` - adaptive quadratic/cubic flattening
- `tests/test_flatten.c` - assertion-based test suite
- `Makefile` - build and test targets

## Intentional bugs in the current HEAD

This repository is a two-commit exercise. The first commit contains the
correct implementation and passes all 42 checks. The current HEAD
intentionally ships three defects in `src/flatten.c`; the test suite is not
modified between the two versions.

1. **B1** - Cubic and quadratic flatness use half of the sum of control-point
   distances, and the tolerance comparison is reversed (`>=`). As a result,
   making the tolerance smaller produces fewer, not more, vertices.
2. **B2** - After flattening, the top-level API drops the last vertex, so the
   reported polyline does not end at `p1`.
3. **B3** - `append_point()` writes a vertex before checking capacity. With a
   one-point buffer the endpoint write lands one point past the end, which is
   detected by the test's guard-byte sentinels.

Current buggy result: `checks=42 failures=15` (all assertion failures, no
crash). Correct baseline result: `checks=42 failures=0`.
