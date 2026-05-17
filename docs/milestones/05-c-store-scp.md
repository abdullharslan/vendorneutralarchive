# Milestone 05: C-STORE SCP Receiver

## Goal

Implement the first real DICOM archive ingress path: a C-STORE SCP receiver
that accepts DICOM instances from a remote SCU and persists them locally.

This milestone must build on the existing C++17/DCMTK foundation without adding
database, MinIO, Redis, DICOMweb, UI, or compliance infrastructure.

## Current Status

Not implemented.

The current `vna-receiver` only parses CLI configuration and prints that DICOM
networking is not implemented yet.

## Scope

Implement:

- a DCMTK-based C-STORE SCP under `src/dicom`;
- integration with `apps/vna-receiver`;
- association initialization and cleanup;
- called AE title validation;
- presentation context negotiation for an intentionally small storage SOP Class
  set;
- real C-STORE request handling;
- local persistence of the received file through a minimal storage abstraction;
- real DICOM success/failure statuses based on actual receive/persist outcome;
- manual or automated validation with DCMTK `storescu`.

Because no storage abstraction exists yet, this milestone may introduce the
minimal `src/storage` interface required for the receiver to write files. Keep
that interface small. Do not implement MinIO, database indexing, audit, or async
queues in this milestone.

## Out Of Scope

Do not implement:

- PostgreSQL;
- MinIO;
- Redis;
- Celery;
- DICOMweb;
- C-FIND;
- C-MOVE;
- C-GET;
- MPPS;
- Storage Commitment;
- HL7;
- FHIR;
- admin UI;
- internal HTTP API;
- audit trail;
- fake success responses.

## Design Requirements

- DCMTK networking code must stay in `src/dicom` implementation files.
- Public headers must not expose raw DCMTK types.
- `vna-receiver` must not own raw association logic.
- The receiver must return DICOM failure if persistence fails.
- The received bytes must be written to a local file before success is returned.
- Storage must be behind a project-owned abstraction, even if the only
  implementation is local filesystem.
- Domain and config layers must remain DCMTK-free.
- Metadata indexing is not required yet.

## Acceptance Criteria

- `vna-receiver` starts with `--host`, `--port`, `--ae-title`, and
  `--storage-path`.
- A DCMTK `storescu` command can send a valid DICOM file to `vna-receiver`.
- The received file is saved under the configured local storage path.
- The receiver returns a real DICOM success status only after persistence
  succeeds.
- The receiver returns a real DICOM failure status if the file cannot be
  persisted.
- `vna-send --echo-only` still works.
- Existing DICOM file reader tests still pass.
- Strict warning build succeeds.
- DCMTK remains isolated to `src/dicom`.
- No database, object storage, audit, DICOMweb, or UI code is added.

## Suggested Manual Validation

Start the receiver:

```bash
./build/bin/vna-receiver --host 127.0.0.1 --port 11112 \
    --ae-title VNA --storage-path /tmp/vna-store
```

Send a test instance from another terminal:

```bash
storescu -aec VNA 127.0.0.1 11112 /path/to/sample.dcm
```

Expected result:

- `storescu` reports C-STORE success;
- the file appears under `/tmp/vna-store`;
- the receiver logs/prints a clear English result;
- invalid storage path scenarios return failure, not fake success.

## RTM/SKS Alignment

This milestone contributes to the external C-STORE SCP requirement area, but it
does not complete the full RTM target. The external RTM also includes MinIO,
PostgreSQL indexing, Redis queueing, audit, whitelist, validation, and
performance requirements. Those remain planned future work.

## Completion Notes

Not implemented yet.
