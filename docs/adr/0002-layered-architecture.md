# ADR 0002: Layered Architecture

## Status

Accepted.

## Context

VNA is intended to grow from a small C++17 skeleton into a DICOM archive node.
The project needs to add protocol handling, storage, indexing, query, and
retrieve behavior without turning every module into a dependency on DCMTK or a
future database.

The codebase is also being developed milestone by milestone with AI assistance,
so the architecture must be explicit enough to prevent accidental implementation
of future features or cross-layer coupling.

## Decision

Use a layered architecture with clear dependency direction and small libraries.

Current and planned layers:

- `apps`
- `src/application`
- `src/config`
- `src/dicom`
- `src/storage`
- `src/index`
- `src/logging`
- `src/domain`
- `tests`
- `docs`

The domain layer is the stable inner layer. Infrastructure layers may depend on
domain. Application orchestration will connect abstractions. CLI apps will wire
configuration and concrete implementations together.

## Layer Responsibilities

### apps

CLI and service entrypoints.

Responsibilities:

- parse command-line input through `src/config`;
- call application services or narrow infrastructure services;
- print user-facing output;
- return meaningful process exit codes.

Current apps:

- `vna-send`: supports CLI parsing and real C-ECHO through `--echo-only`;
  non-echo mode does not send C-STORE yet.
- `vna-receiver`: parses receiver config and prints that DICOM networking is
  not implemented yet.

### src/domain

Pure domain model.

Responsibilities:

- value objects;
- validation that belongs to DICOM/archive concepts;
- result types;
- DIMSE status categorization independent of DCMTK.

No I/O, CLI parsing, DCMTK, storage, database, or logging dependency belongs
here.

### src/config

Configuration and CLI parsing.

Responsibilities:

- turn `argv` into typed config structs;
- validate required flags and user-facing ranges;
- reuse domain validators where appropriate;
- report `ConfigError`.

### src/dicom

DICOM implementation boundary.

Responsibilities:

- use DCMTK for file parsing and networking;
- translate DCMTK failures into VNA exceptions or domain results;
- keep raw DCMTK types out of public headers;
- implement DICOM protocol behavior without owning storage or index policy.

### src/application

Planned use-case orchestration layer.

Responsibilities:

- coordinate DICOM services, storage, and index abstractions;
- keep workflows such as "receive, store, and index instance" out of CLI and
  low-level protocol classes.

This layer is currently empty.

### src/storage

Planned storage layer.

Responsibilities:

- define storage abstractions;
- implement local filesystem storage first;
- hide storage layout from DICOM networking code.

This layer is currently empty.

### src/index

Planned metadata indexing layer.

Responsibilities:

- store searchable Patient/Study/Series/Instance metadata;
- support future C-FIND and C-MOVE workflows.

This layer is currently empty.

### src/logging

Planned logging helper layer.

Responsibilities:

- provide consistent process logging if needed;
- avoid scattering logging policy through domain and protocol code.

This layer is currently empty.

### tests

Automated verification.

Responsibilities:

- unit tests for deterministic behavior;
- future integration tests for real DICOM networking and storage workflows;
- fixture generation in build directories.

## Allowed Dependencies

- `src/domain` depends only on the C++ standard library.
- `src/config` may depend on `src/domain`.
- `src/dicom` may depend on `src/domain` and DCMTK.
- `src/application` may depend on `src/domain` and project abstractions from
  DICOM, storage, index, and logging layers.
- `apps/*` may depend on config, application services, logging, and concrete
  infrastructure implementations.
- Tests may depend on the project library they are testing.

## Forbidden Dependencies

- `src/domain` must not depend on config, DICOM, storage, index, logging, apps,
  or DCMTK.
- `src/config` must not depend on DCMTK, storage, index, or apps.
- `src/dicom` public headers must not expose raw DCMTK types.
- `apps/*` must not directly implement DICOM association handling.
- Storage implementations must not own DICOM protocol negotiation.
- Metadata indexing must not require DCMTK dataset objects as its public input.
- Future database or object-storage dependencies must not leak into domain.

## Consequences

Positive:

- Domain rules remain easy to test.
- DCMTK usage is contained.
- Future storage and index implementations can change without rewriting the
  DICOM protocol boundary.
- AI agents have concrete rules for what belongs in each milestone.

Tradeoffs:

- More small libraries and files are needed than in a single-binary prototype.
- Some simple workflows may require extra wiring through abstractions.
- Milestone order matters: C-STORE SCP, storage, and indexing must be designed
  together carefully so protocol code does not become the storage layer.

## Planning Note

The roadmap keeps C-STORE SCP as Milestone 05 and local storage hardening as
Milestone 06. To avoid protocol code becoming storage code, Milestone 05 is
allowed to introduce only the minimal storage abstraction required for C-STORE
SCP persistence. Milestone 06 then completes and tests the local storage layer.

No MinIO, PostgreSQL, Redis, audit, DICOMweb, or UI dependency should be added
as part of that handoff.
