# DICOM Scope

## Current DICOM Functionality In Scope

The current implemented DICOM scope is intentionally small.

Implemented:

- DICOM value modeling in the domain layer:
  - AE title validation;
  - DICOM node identity;
  - DICOM file descriptor;
  - instance metadata;
  - DIMSE status categorization;
  - C-ECHO and C-STORE result value objects.
- Local DICOM Part 10 file reading through DCMTK `dcmdata`:
  - verifies that the path exists and is a non-empty file;
  - parses the file with DCMTK;
  - extracts SOP Instance UID, SOP Class UID, Study Instance UID,
    Series Instance UID, Patient ID, and Modality;
  - reports failures through `DicomFileReadError`.
- C-ECHO SCU through DCMTK `dcmnet`:
  - creates a real association request;
  - negotiates the Verification SOP Class;
  - sends a real C-ECHO request;
  - derives success only from the returned DICOM status.

Current CLI behavior:

- `vna-send --echo-only` performs a real C-ECHO.
- `vna-send` without `--echo-only` does not send C-STORE yet and exits
  non-zero after printing that C-STORE is not implemented.
- `vna-receiver` parses receiver configuration but does not listen for DICOM
  associations yet.

## Planned DICOM Functionality

Planned roadmap items:

- C-ECHO SCP;
- C-STORE SCP receiver;
- local file storage for received instances;
- metadata index;
- C-STORE SCU sender;
- C-FIND SCP;
- C-MOVE SCP.

Future roadmap items beyond the MVP:

- DICOMweb;
- Storage Commitment;
- MWL / HL7;
- MinIO-backed storage;
- PostgreSQL-backed indexing;
- production deployment.

These are planned items only. They are not implemented unless the source code
and milestone completion notes explicitly say so.

## Explicitly Out Of Scope For Now

The following are out of scope in the current codebase:

- C-ECHO SCP;
- C-STORE SCP request handling;
- C-STORE SCU file transfer;
- C-FIND;
- C-MOVE;
- C-GET;
- DICOMweb;
- HL7;
- Modality Worklist;
- Storage Commitment;
- persistent metadata database;
- MinIO or other object storage;
- PostgreSQL;
- UI;
- fake DICOM success responses.

The attached RTM/SKS documents include many additional DICOM services marked as
complete for a broader target system. In this repository, they remain planned or
out of scope until implemented and tested here.

## DCMTK Isolation Rules

DCMTK is the implementation library for DICOM parsing and networking, but it
must not become the project-wide domain model.

Rules:

- DCMTK includes belong in `src/dicom/*.cpp` by default.
- Public headers in `src/dicom` must expose project types, not raw DCMTK
  handles.
- `src/domain` must not include or link DCMTK.
- `src/config` must not include or link DCMTK.
- CLI apps must not manage raw DCMTK associations directly.
- `vna_dicom` may link DCMTK privately.
- Errors crossing out of `src/dicom` should use project exception types or
  domain result types.

Currently, DCMTK is used in:

- `DicomFileReader.cpp` for Part 10 file parsing;
- `EchoScu.cpp` for C-ECHO association and DIMSE networking.

## Basic DICOM Concepts Used Here

- AE Title: Application Entity title. This project validates AE titles as
  1-16 printable ASCII characters with no backslash.
- DICOM node: a remote or local endpoint represented by AE title, host, and
  port.
- Association: the DICOM network session negotiated between two Application
  Entities.
- Presentation Context: negotiated pairing of SOP Class and transfer syntax.
- SOP Class UID: identifies the type of DICOM object or service.
- SOP Instance UID: identifies a single DICOM instance.
- Study Instance UID and Series Instance UID: identify the study and series
  hierarchy for an instance.
- DIMSE status: the DICOM service status code returned by operations such as
  C-ECHO or C-STORE.
- Part 10 file: the standard on-disk DICOM file format with file meta
  information and a dataset.

## Operation Constraints

### C-ECHO

Current status: implemented as SCU only. C-ECHO SCP is not implemented.

Constraints:

- Use the Verification SOP Class.
- Send a real C-ECHO request with DCMTK `DIMSE_echoUser`.
- Return success only when the response status is DICOM success.
- Keep association cleanup explicit.
- Do not implement generic networking abstractions prematurely.

### C-STORE

Current status: not implemented.

Planned constraints:

- SCP must receive real C-STORE requests and return real DICOM success or
  failure status.
- SCU must send real files, not just print a success message.
- File persistence should go through a storage abstraction.
- The DICOM networking class should not own metadata indexing or database
  concerns directly.

### DICOMweb

Current status: not implemented.

Planned constraints:

- DICOMweb must not be added before DIMSE archive workflows are stable unless
  the roadmap is explicitly reprioritized.
- QIDO-RS should use the same metadata index as C-FIND where possible.
- WADO-RS/WADO-URI should use the same storage abstraction as C-MOVE.
- STOW-RS should share validation and storage rules with C-STORE SCP where
  possible.

### C-FIND

Current status: not implemented.

Planned constraints:

- Should answer from the metadata index.
- Should not scan raw files for every query unless a milestone explicitly
  chooses that tradeoff.
- Must return real DICOM pending and final statuses.

### C-MOVE

Current status: not implemented.

Planned constraints:

- Should retrieve indexed/stored instances and send them to the requested move
  destination.
- Should not be implemented before storage and indexing exist.
- Must distinguish association failures, missing objects, and DICOM status
  failures.
