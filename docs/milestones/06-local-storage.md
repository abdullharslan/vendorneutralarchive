# Milestone 06: Local File Storage

## Goal

Harden the local filesystem storage layer so received DICOM instances can be
stored, located, and read back deterministically by later archive workflows.

This milestone supports C-STORE SCP and prepares for C-MOVE. It is still local
storage only.

## Current Status

Not implemented.

`src/storage` currently exists as an empty folder. Milestone 05 may introduce a
minimal storage interface if needed for C-STORE SCP. This milestone should
complete and test that local storage layer.

## Scope

Implement under `src/storage`:

- storage interface for writing and reading DICOM instance bytes;
- local filesystem implementation;
- deterministic object path strategy;
- safe directory creation;
- duplicate handling policy;
- readback API needed by C-MOVE;
- storage-specific error type;
- unit tests for successful and failing storage operations.

Recommended object path:

```text
<root>/<StudyInstanceUID>/<SeriesInstanceUID>/<SOPInstanceUID>.dcm
```

The path strategy must avoid trusting arbitrary user-supplied path fragments.
Use validated UIDs from `DicomInstanceMetadata`.

## Out Of Scope

Do not implement:

- MinIO;
- S3;
- PostgreSQL;
- Redis;
- Celery;
- audit trail;
- retention cleanup;
- encryption;
- backup/restore;
- DICOM networking;
- metadata index.

## Design Requirements

- Storage must depend on domain types, not on DCMTK.
- Storage must not parse DICOM datasets itself.
- Storage must not own DICOM association logic.
- File writes should avoid partial-success behavior where possible.
- Error messages must be clear and English.
- Generated files in tests must live in temporary/build directories.

## Acceptance Criteria

- Local storage can write a DICOM file from a source path or byte stream.
- Stored path is deterministic from Study/Series/SOP Instance UIDs.
- Local storage can read back a stored instance by identity.
- Invalid root path, missing source file, invalid metadata, or write failure
  produces a storage error.
- Storage tests run through CTest.
- Domain, config, and DICOM public headers remain cleanly layered.
- No MinIO, PostgreSQL, Redis, or DICOMweb code is added.

## RTM/SKS Alignment

This milestone covers only local filesystem storage. It does not satisfy the
external RTM target for MinIO/S3 storage, server-side encryption, lifecycle
cleanup, or distributed backup.

## Completion Notes

Not implemented yet.
