# PBKit Baseline

The `123_nehe_pb_*` through `134_nehe_pb_*` lessons are retained as native PBKit
reference ports for lessons 1 through 12.

They are not intended to grow into a separate demo framework. Their purpose is
to answer one question quickly: does a visual issue appear only in the NXGL path,
or does it also appear when the same lesson data is drawn directly through the
native renderer helpers?

## Comparison Workflow

Build matching lesson numbers from both families:

```sh
make -C 115_nehe_nxgl_05_3d_shapes
make -C 127_nehe_pb_05_3d_shapes
```

Capture both in xemu at comparable frame times, then compare them with the
Windows reference capture described in `REFERENCE_CAPTURE.md`.

## Interpretation

- If Windows and PBKit match but NXGL differs, prioritize NXGL state,
  conversion, or draw submission.
- If PBKit and NXGL match but Windows differs, inspect shared lesson data,
  texture assets, timing, or projection setup.
- If all three differ, capture more frames before making a rendering change.
