# Milestone 00: Project Skeleton

## Goal

Create a clean C++17 project skeleton for a DICOM archive node using DCMTK.

## Scope

Create the initial project structure, CMake setup, documentation placeholders, and empty source folders.

## Required Structure

- CMakeLists.txt
- apps/vna-receiver
- apps/vna-send
- src/domain
- src/application
- src/dicom
- src/storage
- src/index
- src/config
- src/logging
- tests/unit
- tests/integration
- docs

## Rules

- Use English only.
- Do not implement DICOM networking yet.
- Do not implement storage logic yet.
- Do not implement database logic yet.
- Do not implement UI, Redis, Celery, MinIO, PostgreSQL, DICOMweb, HL7, C-FIND, C-MOVE, or Storage Commitment.
- Do not create fake success responses.
- Keep the project ready for SOLID-based growth.

## Acceptance Criteria

- The folder structure exists.
- CMake configures successfully.
- The project builds a minimal placeholder executable for `vna-send`.
- The project builds a minimal placeholder executable for `vna-receiver`.
- `docs/architecture.md` or `docs/ARCHITECTURE.md` explains the intended layered architecture.
- No future milestone functionality is implemented.

## Completion Notes

Implemented on 2026-05-15.

### What was created

- Root `CMakeLists.txt` declaring the `vna` project, C++17 standard
  (required, no GNU extensions), and a default Release build type. Binaries
  are emitted to `build/bin/`.
- `apps/CMakeLists.txt` plus per-app `CMakeLists.txt` files for
  `apps/vna-receiver` and `apps/vna-send`.
- Minimal placeholder `main.cpp` for each app. Each prints that it is a
  Milestone 00 skeleton and that DICOM networking is not implemented yet.
  No fake DICOM responses are produced.
- Folder skeleton for the layered architecture:
  - `src/domain`, `src/application`, `src/dicom`, `src/storage`,
    `src/index`, `src/config`, `src/logging`
  - `tests/unit`, `tests/integration`
- `docs/ARCHITECTURE.md` populated with the layered architecture, per-layer
  responsibilities, and dependency rules.

### Verification

- `cmake -S . -B build` configured cleanly with AppleClang 21.0.0 and
  CMake 4.3.2.
- `cmake --build build` produced `build/bin/vna-receiver` and
  `build/bin/vna-send`.
- Running each binary prints the placeholder message and exits with status 0.

### Acceptance criteria check

- Folder structure exists: yes.
- CMake configures successfully: yes.
- Placeholder executable for `vna-send` builds: yes.
- Placeholder executable for `vna-receiver` builds: yes.
- `docs/ARCHITECTURE.md` explains the intended layered architecture: yes.
- No future milestone functionality implemented: yes (DCMTK is not yet a
  dependency; no networking, storage, indexing, configuration, or logging
  code exists; the `src/*` and `tests/*` directories remain empty).

### Notes for future milestones

- DCMTK is not yet integrated. It will be wired in via `src/dicom` when the
  C-ECHO SCU milestone (04) or earlier requires it.
- A test framework has not been chosen yet; `tests/unit` and
  `tests/integration` are empty placeholders and are not yet added via
  `add_subdirectory` in the root `CMakeLists.txt`.
- CMake was installed via Homebrew (`brew install cmake`) on the build host
  as part of this milestone; no other tooling was added.