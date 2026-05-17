# Milestone 09: C-FIND SCP

## Goal

Implement a C-FIND SCP that answers DICOM query requests from the metadata
index.

This milestone adds query behavior for the local archive MVP.

## Current Status

Not implemented.

No C-FIND networking or query model exists yet.

## Scope

Implement:

- DCMTK-based C-FIND SCP handling under `src/dicom`;
- query model types if needed;
- integration with the metadata index from Milestone 07;
- support for an intentionally limited initial query scope;
- real pending and final DIMSE responses;
- validation with DCMTK or dcm4che `findscu`.

Initial query scope should be conservative:

- Study Root Query/Retrieve Information Model;
- STUDY level first;
- optional SERIES/IMAGE levels only if the milestone remains small and tested.

## Out Of Scope

Do not implement:

- PostgreSQL;
- DICOMweb QIDO-RS;
- C-MOVE;
- C-GET;
- HL7;
- audit trail unless a dedicated audit milestone already exists;
- admin UI.

## Design Requirements

- C-FIND must answer from the metadata index, not from hard-coded fake rows.
- Pending responses must represent actual matching records.
- Final success/failure status must be real DICOM status.
- Wildcard matching rules must be explicitly defined and tested.
- Public headers must not expose raw DCMTK types.
- Index layer must remain DCMTK-free.

## Acceptance Criteria

- A real `findscu` command can query `vna-receiver`.
- At least STUDY-level query is supported.
- Empty result returns a valid final status without fake matches.
- Matching result returns pending response(s) plus final success.
- Invalid query returns a clear failure status where appropriate.
- Unit tests cover query matching rules.
- Integration/manual validation is documented.
- Strict warning build succeeds.

## RTM/SKS Alignment

This milestone contributes to C-FIND requirements in the external RTM. It does
not complete DICOMweb QIDO-RS, PostgreSQL-backed performance targets, audit
events, or full patient/study/series/image conformance until those are
implemented and tested.

## Completion Notes

Not implemented yet.
