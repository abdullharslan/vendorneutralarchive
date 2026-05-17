# Project Brief

## Purpose

VNA is a C++17 Vendor Neutral Archive learning and implementation project.
The long-term goal is to implement core DICOM archive workflows similar in
scope to a small subset of DCM4CHEE: receive, store, index, query, and retrieve
DICOM studies over standard DICOM networking.

The project is intentionally built in small milestones so that each layer can
be reviewed, tested, and kept independent from unrelated infrastructure.

## Problem Being Solved

Medical imaging systems exchange studies through DICOM services. A clean archive
node needs to:

- accept DICOM associations from remote nodes;
- validate and store incoming instances;
- extract searchable metadata;
- answer query requests;
- retrieve or forward stored instances through standard DICOM networking.

This repository is building those capabilities from the foundation upward while
keeping DCMTK isolated from pure domain and configuration code.

## MVP Scope

The MVP target is a small DICOM archive node that can:

- receive DICOM instances through C-STORE SCP;
- persist received instances to local filesystem storage;
- index core instance metadata;
- query indexed metadata through C-FIND SCP;
- retrieve stored instances through C-MOVE SCP;
- send local files through C-STORE SCU for validation and interoperability
  testing;
- provide clear command-line entrypoints for receiver and sender workflows.

The MVP is not intended to be a production PACS replacement. It is a clean,
reviewable, protocol-focused implementation of the core archive path.

## Current Implemented Scope

The current codebase implements milestones 00 through 04.

Implemented:

- CMake project skeleton with C++17 and two CLI binaries:
  - `vna-send`
  - `vna-receiver`
- Pure domain layer under `src/domain`:
  - AE title validation;
  - DICOM node value object;
  - DICOM file descriptor;
  - instance metadata value object;
  - DIMSE status categorization;
  - C-ECHO and C-STORE result value objects.
- Configuration layer under `src/config`:
  - CLI parsing for `vna-send`;
  - CLI parsing for `vna-receiver`;
  - validation for ports, AE titles, required arguments, file/storage paths,
    and timeout values;
  - `ConfigError` for user-facing configuration failures.
- DICOM file reader under `src/dicom`:
  - local Part 10 DICOM file parsing with DCMTK `dcmdata`;
  - extraction of SOP Instance UID, SOP Class UID, Study Instance UID,
    Series Instance UID, Patient ID, and Modality;
  - conversion to domain objects;
  - reader-level failures reported as `DicomFileReadError`.
- C-ECHO SCU under `src/dicom`:
  - real DCMTK `dcmnet` association setup;
  - Verification SOP Class presentation context negotiation;
  - real `DIMSE_echoUser` call;
  - explicit release, abort, destroy, and network cleanup paths;
  - `vna-send --echo-only` integration.
- Unit tests for:
  - DICOM reader invalid input cases;
  - DICOM reader positive fixture case;
  - send CLI timeout validation.

Not implemented yet:

- C-STORE SCP receiving;
- C-STORE SCU file sending;
- local storage implementation;
- metadata index;
- C-FIND SCP;
- C-MOVE SCP;
- application orchestration layer;
- logging helpers;
- production deployment concerns.

## Planned Scope

The roadmap currently lists the following planned milestones after C-ECHO:

- Milestone 05: C-STORE SCP receiver;
- Milestone 06: local file storage;
- Milestone 07: metadata index;
- Milestone 08: C-STORE SCU sender;
- Milestone 09: C-FIND SCP;
- Milestone 10: C-MOVE SCP.

Advanced features are listed as future work only:

- DICOMweb;
- Storage Commitment;
- MWL / HL7;
- MinIO;
- PostgreSQL;
- production deployment.

## Explicit Non-Goals

The following must not be added before their own milestone explicitly requires
them:

- UI;
- Redis;
- Celery;
- MinIO;
- PostgreSQL;
- DICOMweb;
- HL7;
- MWL;
- Storage Commitment;
- production authentication or authorization;
- cloud deployment automation;
- fake DICOM success responses.

The project must not claim success for a DICOM operation unless the real DICOM
protocol exchange produced that result.

## Main Modules

- `apps/`: CLI entrypoints. These parse configuration, call the appropriate
  service, and translate outcomes into process output and exit codes.
- `src/domain/`: pure value objects and result types. This layer has no DCMTK,
  CLI, filesystem, networking, storage, or database dependency.
- `src/config/`: CLI parsing and validation. This layer may depend on domain
  validators but must not include DCMTK.
- `src/dicom/`: DCMTK-based DICOM file parsing and networking. DCMTK headers
  are kept in implementation files where possible; public headers expose domain
  types and project error types.
- `src/application/`: planned orchestration layer. Currently empty.
- `src/storage/`: planned storage abstractions and implementations. Currently
  empty.
- `src/index/`: planned metadata indexing abstractions and implementations.
  Currently empty.
- `src/logging/`: planned logging helpers. Currently empty.
- `tests/unit/`: current automated tests.
- `tests/integration/`: reserved for future real networking/storage workflows.
- `docs/`: architecture, roadmap, ADRs, and milestone notes.

## Build And Test Workflow

Configure and build:

```bash
cmake -S . -B build
cmake --build build
```

Run tests:

```bash
ctest --test-dir build --output-on-failure
```

Run a strict warning build:

```bash
cmake -S . -B build-strict -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Werror"
cmake --build build-strict
ctest --test-dir build-strict --output-on-failure
```

The positive DICOM reader test requires the DCMTK CLI tool `dump2dcm` to be
available on `PATH`, because CMake generates a real Part 10 fixture from a
textual dump during the build.

## MVP Success Criteria

The MVP is successful when:

- `vna-receiver` can accept a real DICOM C-STORE request from DCMTK `storescu`;
- received instances are persisted through a storage abstraction;
- core metadata is extracted and indexed;
- C-FIND can query indexed study/series/instance information;
- C-MOVE can retrieve stored instances through standard DICOM networking;
- `vna-send` can validate connectivity with C-ECHO and send local files through
  C-STORE SCU;
- all protocol success and failure responses are real DICOM responses, not log
  messages or simulated success;
- domain and configuration layers remain DCMTK-free;
- automated tests and documented manual validation cover the core workflows.
