# Contributing

Contributions are welcome, especially visual fixes, lesson parity improvements,
and build cleanup.

## Guidelines

- Keep NXGL implementation changes in the `nxgl` repository.
- Keep this repository focused on NeHe demos, PBKit comparisons, and capture
  tooling.
- Preserve the standalone lesson numbering. The classic NeHe sequence is lessons
  1-48; later experiments should be named and documented as supplemental demos.
- Do not commit generated `.inl`, `.obj`, `.d`, `.exe`, `.xbe`, `.iso`, or
  `dist/` output.
- When changing rendering behavior, compare against Windows reference captures
  and the PBKit lesson where possible.

## Validation

For small source changes:

```sh
make -C 116_nehe_nxgl_06_texture_mapping
make -C 128_nehe_pb_06_texture_mapping
```

For release-facing changes:

```sh
make
./tools/collect_release_isos.sh
```
