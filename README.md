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
