# Milestone 10: C-MOVE SCP

## Goal

Implement a C-MOVE SCP that retrieves stored DICOM instances from the local
archive and sends them to a requested move destination.

This milestone completes the first receive-store-query-retrieve MVP loop.

## Current Status

Not implemented.

No C-MOVE networking, destination resolution, or retrieval orchestration exists
yet.

## Scope

Implement:

- DCMTK-based C-MOVE SCP handling under `src/dicom`;
- move destination configuration or lookup mechanism;
- retrieval from local storage;
- instance selection through the metadata index;
- outbound C-STORE sub-operations to the move destination;
- real pending/progress/final DICOM statuses;
- validation with a real destination storage SCP.

Initial retrieve scope should be conservative:

- Study Root model;
- STUDY level first;
- SERIES level if storage/index APIs are ready and tests remain focused.

## Out Of Scope

Do not implement:

- C-GET;
- DICOMweb WADO-RS/WADO-URI;
- PostgreSQL;
- MinIO;
- queue-based async retrieval;
- audit trail unless a dedicated audit milestone already exists;
- admin UI.

## Design Requirements

- C-MOVE must use actual stored files, not generated placeholders.
- Missing studies/series must produce correct DICOM failure or empty-result
  behavior.
- Sub-operation counts must reflect actual attempted sends.
- Destination association failures must be reported clearly.
- Storage and index layers must remain independent from raw DCMTK types.
- Public headers must not expose raw DCMTK association handles.

## Acceptance Criteria

- A real `movescu` command can request a stored study.
- The receiver opens an outbound association to the configured move
  destination.
- Stored DICOM instances are sent through real C-STORE sub-operations.
- Pending/progress/final statuses are returned correctly.
- Missing data and destination failure scenarios are tested or documented.
- Strict warning build succeeds.
- No DICOMweb, MinIO, PostgreSQL, C-GET, or UI code is added.

## RTM/SKS Alignment

This milestone contributes to C-MOVE requirements in the external RTM. It does
not complete C-GET, DICOMweb WADO-RS, audit, performance, IHE Connectathon, or
production retrieval guarantees.

## Completion Notes

Not implemented yet.
