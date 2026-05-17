# Architecture

## Overview

VNA is a C++17 DICOM archive node built on top of DCMTK. The project aims at
core DICOM archive workflows (receive, store, index, query, retrieve) over
standard DICOM networking, comparable in spirit to DCM4CHEE.

The codebase follows a layered architecture. Each layer has a single
responsibility, depends only on layers below it, and is built up over the
roadmap milestones. Milestone 00 only establishes the empty skeleton; layers
become populated by the milestones that follow.

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

- `apps/vna-receiver`: future C-STORE SCP service entrypoint.
- `apps/vna-send`: future C-ECHO / C-STORE SCU entrypoint.

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

DICOM networking, storage, indexing, UI, Redis, Celery, MinIO, PostgreSQL,
DICOMweb, HL7, C-FIND, C-MOVE, and Storage Commitment are explicitly out of
scope until the milestone that introduces them. Milestone 00 ships only the
folder skeleton, the CMake setup, and two placeholder executables that
acknowledge the absence of DICOM functionality.
