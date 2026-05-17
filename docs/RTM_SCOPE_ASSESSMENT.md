# RTM Scope Assessment

## Purpose

This document compares the current VNA C++ repository against the externally
provided RTM/SKS target documents:

- `RTM.md`
- `RTM.xlsx`
- `SKS_TITCK_Uyumluluk.md`

The attached documents describe the final VNA/PACS Engine target scope. They
must be treated as final-scope requirements and compliance evidence planning,
not as evidence that those features already exist in this repository.

## Current Repository Baseline

Current implemented scope in this repository:

- C++17 CMake project structure.
- `vna-send` and `vna-receiver` CLI entrypoints.
- Pure domain value objects and validation.
- CLI configuration parsing and validation.
- DCMTK-based DICOM Part 10 file reader.
- DCMTK-based C-ECHO SCU integrated into `vna-send --echo-only`.
- CTest-based unit tests for DICOM file reading and CLI timeout validation.

Current explicitly missing scope:

- C-ECHO SCP.
- C-STORE SCP.
- C-STORE SCU.
- Local archive storage.
- Metadata index.
- C-FIND SCP.
- C-MOVE SCP.
- C-GET.
- MPPS.
- Storage Commitment.
- DICOMweb.
- Worklist / HL7 / FHIR.
- Admin panel.
- Internal HTTP API.
- Audit trail.
- Authentication / authorization.
- DICOM TLS.
- MinIO, Redis, PostgreSQL, Celery, Docker deployment.
- Performance/load testing.

## Important Interpretation Rule

The RTM files contain many rows marked as complete for a different or future
VNA/PACS Engine scope. Those completion statuses do not match the current
source tree. For this repository, requirement status must be re-baselined from
actual code and tests.

Future AI agents must not copy the external RTM status values into project
completion notes unless the corresponding implementation exists in this
repository.

## Current Coverage Against RTM Categories

| RTM Category | Current Repository Coverage | Gap |
|---|---|---|
| DICOM DIMSE | C-ECHO SCU only. | C-ECHO SCP, C-STORE SCP/SCU, C-FIND, C-MOVE, C-GET, MPPS, Storage Commitment are missing. |
| Transfer Syntax | C-ECHO proposes Explicit VR Little Endian, Big Endian Explicit, and Implicit VR Little Endian for Verification SOP Class. | Storage transfer syntax negotiation, JPEG/JPEG-LS/JPEG 2000, and configurable ordering are missing. |
| SOP Classes | Verification SOP Class is used for C-ECHO. The DICOM reader can parse a Secondary Capture test fixture. | Storage SOP Class negotiation for CT/MR/CR/DX/US/PET/SR/etc. is missing. |
| DICOMweb | None. | QIDO-RS, WADO-URI, WADO-RS, STOW-RS, DICOM JSON, multipart handling, and HTTP service are missing. |
| Security | AE title validation and CLI input validation exist. | AE whitelist, RBAC, auth, account lockout, TLS, DICOM TLS, secret handling, and security audit are missing. |
| Audit Trail | None. | ATNA-style audit model, audit persistence, DICOM event logging, user event logging, export, and retention policy are missing. |
| Storage | `DicomFile` value object exists; no archive storage exists. | Local storage, object layout, readback, checksums, MinIO/S3, retention, and cleanup are missing. |
| Database / Index | None. | Study/series/instance index, query model, migrations, tenant/device tables, and PostgreSQL are missing. |
| Worklist / HL7 / FHIR | None. | MWL, HL7 MLLP, ORM/ORU handling, MPPS integration, REST worklist API, and FHIR ImagingStudy are missing. |
| Admin Panel | None. | Dashboard, studies/devices/logs/audit/settings pages, RBAC UI, and device management are missing. |
| Internal API | None. | Health/status/studies/devices/logs/settings/audit endpoints are missing. |
| Monitoring / DevOps | CMake build exists. Git repository exists. | Docker, health checks, watchdog, alarms, deployment config, and service orchestration are missing. |
| Performance | No performance targets measured. | Load tests, concurrency tests, capacity tests, and performance budgets are missing. |
| Compliance | Basic project docs and ADRs exist. | DICOM Conformance Statement, formal RTM, risk management, IEC 62304 evidence, SKS/TITCK evidence, and release validation are missing. |
| Test | CTest unit tests exist. | GoogleTest is not used. Integration, load, regression, DICOM conformance, DICOMweb, panel, and end-to-end tests are missing. |

## Re-Baselined Requirement Highlights

The current repository can only claim meaningful coverage for a small subset of
the external requirements:

| External Requirement Area | Current Status In This Repository |
|---|---|
| C-ECHO SCU | Implemented for `vna-send --echo-only`. |
| C-ECHO result reporting | Partially implemented: success/failure message exists; duration reporting is not implemented. |
| C-ECHO SCP | Not implemented. |
| C-STORE SCP/SCU | Not implemented. |
| DICOM file parsing | Implemented for local Part 10 files and core metadata extraction. |
| CMake build | Implemented. |
| Unit test infrastructure | Partially implemented through plain CTest executables, not GoogleTest. |
| DICOM Conformance Statement | Not implemented. Existing docs are project architecture docs, not a formal PS3.2 conformance statement. |
| SKS/TITCK readiness | Not implemented. Current documentation is not certification evidence. |

## Missing Work Organized By Implementation Stage

### Stage A: Core Archive MVP

Required before the project can behave like a minimal DICOM archive:

- local storage abstraction and deterministic local file layout;
- C-STORE SCP receiver;
- DICOM storage SOP Class presentation context negotiation;
- received file persistence;
- metadata extraction after receiving;
- metadata index;
- C-FIND SCP backed by the index;
- C-MOVE SCP backed by storage and index;
- C-STORE SCU sender for outbound validation.

### Stage B: Protocol Hardening

Required before broader interoperability claims:

- C-ECHO SCP;
- DICOM association policy and limits;
- configurable transfer syntax ordering;
- common radiology storage SOP Class coverage;
- failure-status mapping for C-STORE, C-FIND, and C-MOVE;
- DICOM Conformance Statement;
- integration tests against DCMTK/dcm4che/Orthanc where appropriate.

### Stage C: Security And Audit

Required before SKS/KVKK-style operational claims:

- AE title whitelist;
- DICOM TLS;
- audit event model;
- audit persistence;
- audit logging for DICOM operations;
- immutable audit retention rules;
- security-focused tests.

### Stage D: Storage, Index, And Infrastructure Expansion

Part of the final product scope after the local archive MVP is stable:

- PostgreSQL metadata backend;
- MinIO/S3 storage backend;
- Redis/Celery-style asynchronous processing;
- Docker deployment;
- watchdog and alerting.

### Stage E: Application And Web Interfaces

Part of the final product scope after core archive workflows are stable:

- internal HTTP API;
- admin panel;
- DICOMweb;
- Worklist / HL7 / FHIR.

### Stage F: Compliance And Certification Evidence

Documentation and process work needed for formal readiness:

- formal RTM maintained from this repository's real status;
- SKS/TITCK gap analysis;
- IEC 62304 software lifecycle evidence;
- ISO 14971 risk management file;
- DICOM Conformance Statement;
- release validation protocol;
- performance and load test reports;
- security test evidence.

## Documentation Policy

When updating project documentation:

- use "implemented" only for behavior present in the current codebase;
- use "planned" for roadmap items;
- use "target" for requirements from the external RTM/SKS files;
- do not mark a requirement complete from the external RTM unless current code,
  tests, and milestone completion notes support it;
- keep regulatory/compliance text framed as engineering planning, not legal or
  certification advice.
