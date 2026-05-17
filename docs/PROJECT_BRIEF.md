# Project Brief

## Purpose

VNA is a C++17 Vendor Neutral Archive / PACS Engine project.
The final product target is a full archive platform that can receive, store,
index, query, retrieve, expose, audit, secure, monitor, and operate DICOM
studies across standard DICOM, DICOMweb, storage, database, worklist, and
deployment workflows.

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

## Final Product Scope

The final product scope includes the complete long-term target described by the
RTM/SKS documents. These items are part of the intended end state even when they
are not implemented yet.

Final DICOM DIMSE scope:

- C-ECHO SCU and SCP;
- C-STORE SCP receiver;
- C-STORE SCU sender;
- C-FIND SCP;
- C-MOVE SCP;
- C-GET SCP if prioritized after the MVP retrieve workflow;
- MPPS N-CREATE and N-SET;
- Storage Commitment N-ACTION and N-EVENT-REPORT;
- transfer syntax negotiation, including common uncompressed and compressed
  radiology transfer syntaxes;
- common radiology Storage SOP Classes, including CT, MR, CR, DX, US, PET,
  Secondary Capture, Structured Report, and Presentation State as the supported
  SOP Class list is expanded.

Final archive platform scope:

- local filesystem storage;
- MinIO/S3 object storage;
- deterministic object layout;
- metadata indexing;
- PostgreSQL persistence;
- Redis/Celery-style asynchronous processing where needed;
- study, series, and instance query/retrieve workflows;
- audit trail for DICOM, system, and user events;
- AE title whitelist and security policy enforcement;
- DICOM TLS;
- DICOMweb: QIDO-RS, WADO-URI, WADO-RS, and STOW-RS;
- MWL / HL7 workflow integration;
- FHIR ImagingStudy if prioritized in the final integration scope;
- internal HTTP API;
- admin panel;
- RBAC and authentication;
- monitoring/watchdog/alerting;
- Docker/production deployment;
- performance/load/regression test suite;
- DICOM Conformance Statement;
- RTM, SKS/TITCK/KVKK/IHE-oriented compliance evidence.

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

The externally provided RTM/SKS documents add the broader final product scope:
security, audit, Storage Commitment, DICOMweb, Worklist/HL7/FHIR, admin UI,
internal APIs, MinIO, PostgreSQL, Redis/Celery-style processing,
infrastructure, performance, and compliance evidence. These items are part of
the final target, but they are not implemented in the current repository yet.

## Milestone-Gated Work

The following are final-scope items, but they must not be added before their own
milestone or explicit implementation task:

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

## True Non-Goals

The following are not acceptable at any stage:

- fake DICOM success responses;
- claiming a DICOM operation succeeded without a real protocol response;
- claiming SKS/TITCK/KVKK/IHE compliance without implementation and evidence;
- copying external RTM completion statuses into this repository without matching
  code and tests.

The current repository must not claim SKS/TITCK/KVKK/IHE compliance. The
attached RTM/SKS documents are useful target inputs, but current status must be
derived from the actual source tree and tests. See `docs/RTM_SCOPE_ASSESSMENT.md`
and `docs/COMPLIANCE_SCOPE.md`.

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
