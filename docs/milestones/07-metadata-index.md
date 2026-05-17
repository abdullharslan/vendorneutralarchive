# Milestone 07: Metadata Index

## Goal

Implement a minimal metadata index that records received DICOM instance
metadata and supports query/retrieve workflows.

This milestone prepares C-FIND and C-MOVE without adding PostgreSQL.

## Current Status

Not implemented.

`src/index` currently exists as an empty folder.

## Scope

Implement under `src/index`:

- index interface for storing instance metadata;
- in-process/local implementation suitable for tests and early workflows;
- lookup by Study Instance UID;
- lookup by Series Instance UID;
- lookup by SOP Instance UID;
- query support needed by planned C-FIND levels;
- error handling for invalid or missing records;
- unit tests.

The indexed metadata should come from `vna::domain::DicomInstanceMetadata`.

## Out Of Scope

Do not implement:

- PostgreSQL;
- Alembic migrations;
- tenant tables;
- remote device tables;
- audit tables;
- REST API;
- DICOMweb;
- admin UI.

## Design Requirements

- Index APIs must use domain types and project-owned query types.
- Index must not expose DCMTK types.
- Index must not read raw DICOM files directly.
- DICOM file reading remains in `src/dicom`.
- Storage remains in `src/storage`.
- Application orchestration may coordinate reader, storage, and index if needed.

## Acceptance Criteria

- Instance metadata can be inserted into the index.
- Duplicate SOP Instance UID behavior is defined and tested.
- Study, series, and instance lookups work.
- Query-level behavior needed by C-FIND is represented in tests.
- CTest includes metadata index tests.
- No database dependency is introduced.

## RTM/SKS Alignment

This milestone is an early local substitute for the external RTM database/index
requirements. It does not satisfy PostgreSQL, migrations, tenant support,
remote device tracking, audit tables, or production query performance targets.

## Completion Notes

Not implemented yet.
