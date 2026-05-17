# Coding Rules

## Language And Standard

- Use C++17.
- CMake must require C++17 and disable compiler extensions.
- All code, comments, logs, filenames, class names, function names, test names,
  commit messages, and documentation must be written in English.
- Prefer the C++ standard library before adding a dependency.
- Keep changes small and milestone-scoped.

## Style Rules

- Follow the existing namespace structure:
  - `vna::domain`
  - `vna::config`
  - `vna::config::internal`
  - `vna::dicom`
- Use `PascalCase` for classes, structs, and enum classes.
- Use `camelCase` for functions, variables, and data members.
- Private data members use a trailing underscore, for example `path_`.
- Use descriptive names that reflect DICOM meaning, such as `sopInstanceUid`,
  `calledAe`, and `timeoutSeconds`.
- Prefer small functions with one clear responsibility.
- Avoid hidden global state.
- Avoid fake success paths. A DICOM operation succeeds only when the real
  protocol result says it succeeded.

## Header And Source Organization

- Public headers must start with `#pragma once`.
- Headers should include only what they use.
- Implementation details should live in `.cpp` files or unnamed namespaces.
- Use project includes in the existing form:

```cpp
#include "domain/DicomNode.h"
#include "config/SendConfig.h"
#include "dicom/EchoScu.h"
```

- Public headers in `src/domain`, `src/config`, and `src/dicom` must not expose
  raw DCMTK types.
- DCMTK headers should be included in `.cpp` files in `src/dicom` unless a
  future milestone provides a strong reason to do otherwise.

## Error Handling

- Domain validation failures throw `std::invalid_argument`.
- CLI parsing and configuration failures throw `vna::config::ConfigError`.
- DICOM file reader failures throw `vna::dicom::DicomFileReadError`.
- C-ECHO association and DIMSE failures throw `vna::dicom::EchoScuError`.
- CLI applications translate `ConfigError` into exit code `2`.
- `vna-send --echo-only` returns:
  - `0` on real C-ECHO success;
  - non-zero on DICOM or network failure.
- Do not swallow errors and print success.
- Do not replace real DICOM responses with log messages.

## Layering Rules

The intended dependency direction is from outer layers toward inner or lower
level abstractions.

- `src/domain` depends on no other project layer.
- `src/config` may depend on `src/domain`.
- `src/dicom` may depend on `src/domain` and DCMTK.
- `src/application` is planned for use-case orchestration and may depend on
  domain and abstractions from infrastructure layers.
- `src/storage`, `src/index`, and `src/logging` are planned infrastructure
  layers.
- `apps/*` may depend on config, application services, logging, and concrete
  infrastructure wiring.

Forbidden:

- `src/domain` including DCMTK, config, storage, index, logging, or app code.
- `src/config` including DCMTK.
- public headers exposing `T_ASC_*`, `DcmDataset`, `DcmFileFormat`,
  `OFCondition`, or other raw DCMTK types.
- DICOM networking code in CLI entrypoints.
- storage or indexing logic inside DCMTK protocol classes unless a milestone
  explicitly defines a narrow adapter boundary.

## Dependency Rules

- DCMTK is allowed only for DICOM-specific implementation under `src/dicom`.
- Current `vna_dicom` links DCMTK privately.
- Domain and config libraries must stay usable without including DCMTK headers.
- Tests may link project libraries, but test source files should not include
  DCMTK directly unless a future integration milestone explicitly requires it.
- Do not add Redis, Celery, MinIO, PostgreSQL, DICOMweb, HL7, UI frameworks, or
  other infrastructure before the relevant milestone.

## Testing Expectations

- Add tests when behavior is added or when a review fix closes a bug.
- Prefer focused unit tests for domain, config, parsing, and file-reader
  validation.
- Use integration tests for real DICOM networking once SCP/SCU workflows need
  more than a simple manual validation.
- Tests must fail on fake success assumptions.
- Test fixtures should be small, deterministic, and stored as source-friendly
  inputs where possible.
- Generated test outputs belong in the build directory, not in the repository.
- Run at least:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

- For milestone completion or review hardening, also run a strict warning
  build with `-Wall -Wextra -Wpedantic -Werror`.

## Rules For AI Coding Agents

- Read `CLAUDE.md`, `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, and the active
  milestone file before making changes.
- Implement only the requested milestone or task.
- Do not silently implement future roadmap items.
- Do not modify generated build folders.
- Keep all generated code and documentation in English.
- Keep completion notes factual: separate implemented work from planned work.
- When adding DCMTK usage, prove that it does not leak into domain, config, or
  public headers.
- When adding a DICOM operation, validate it against a real DCMTK peer or add
  a test that exercises the real code path.
- Update milestone completion notes after implementation.
- If a milestone has an ordering conflict with existing architecture, document
  the uncertainty before inventing a design.
