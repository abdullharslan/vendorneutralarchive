# Architecture

## Overview

VNA is a C++17 DICOM archive node built on top of DCMTK. The project aims at
core DICOM archive workflows (receive, store, index, query, retrieve) over
standard DICOM networking, comparable in spirit to DCM4CHEE.

The codebase follows a layered architecture. Each layer has a single
responsibility, depends only on layers below it, and is built up over the
roadmap milestones. The current implementation includes the foundation,
domain models, configuration parsing, DICOM file reading, and C-ECHO SCU.
Archive storage, indexing, query, and retrieve workflows are still planned.

## Layers

```
+---------------------------------------------------------------+
|  apps/        CLI and service entrypoints                     |
|  - vna-receiver, vna-send                                     |
+---------------------------------------------------------------+
|  src/application   use cases, orchestration                   |
+---------------------------------------------------------------+
|  src/dicom    DCMTK-based DICOM networking and file parsing   |
|  src/storage  storage abstractions and implementations        |
|  src/index    metadata indexing abstractions and impls        |
|  src/config   CLI and environment configuration               |
|  src/logging  logging helpers                                 |
+---------------------------------------------------------------+
|  src/domain   pure domain models, no DCMTK dependency         |
+---------------------------------------------------------------+
```

### apps

CLI and service entrypoints. Wire configuration, logging, and one or more use
cases into a runnable binary. Contain no business logic.

- `apps/vna-receiver`: planned C-STORE SCP service entrypoint. Currently
  parses configuration and does not listen for DICOM associations.
- `apps/vna-send`: C-ECHO SCU and planned C-STORE SCU entrypoint. Current
  `--echo-only` mode performs a real C-ECHO; non-echo mode does not send
  C-STORE yet.

### src/domain

Pure domain models (Patient, Study, Series, Instance, identifiers, value
objects). No DCMTK dependency, no I/O, no networking. Other layers depend on
domain, not the other way around.

### src/application

Use cases and orchestration. Defines workflows (e.g. "receive and persist a
study") in terms of abstractions from `src/dicom`, `src/storage`, `src/index`.
No DCMTK types are leaked outside this layer.

### src/dicom

DCMTK-based DICOM networking and file parsing. Hosts SCP/SCU implementations
and file readers. This is the only layer that links against DCMTK.

### src/storage

Storage abstractions and implementations. Initially local filesystem; future
milestones may add object storage (MinIO, etc.). The abstraction is defined
here and reused by the application layer.

### src/index

Metadata indexing abstractions and implementations. Used for Patient/Study/
Series/Instance lookup. Initially in-memory; future milestones may add
relational backends (e.g. PostgreSQL).

### src/config

CLI argument parsing and environment configuration. Produces strongly typed
configuration values consumed by application and infrastructure layers.

### src/logging

Thin logging helpers. Keeps logging concerns out of business code and makes
log formatting consistent across binaries.

### tests

`tests/unit` for fast, isolated tests of domain and application logic.
`tests/integration` for tests that exercise real DCMTK networking, storage,
and indexing components together.

## Dependency rules

- `src/domain` depends on nothing else inside this project.
- `src/dicom`, `src/storage`, `src/index`, `src/config`, `src/logging` may
  depend on `src/domain`.
- `src/application` may depend on `src/domain` and on the abstractions
  declared in the infrastructure layers above.
- `apps/*` may depend on `src/application`, `src/config`, `src/logging`, and
  pick concrete infrastructure implementations.
- No layer is allowed to introduce a dependency that points "upward" in this
  diagram.

## Out of scope until their milestone

Storage, indexing, UI, Redis, Celery, MinIO, PostgreSQL, DICOMweb, HL7,
C-STORE, C-FIND, C-MOVE, C-GET, MPPS, and Storage Commitment are explicitly
out of scope until the milestone that introduces them. C-ECHO SCU is the only
implemented DICOM networking operation at this point.

The external RTM/SKS documents describe the broader final target system. They
are tracked as final-scope planning inputs in `docs/RTM_SCOPE_ASSESSMENT.md`
and `docs/COMPLIANCE_SCOPE.md`; they do not change the current implemented
scope until corresponding code and tests exist.
