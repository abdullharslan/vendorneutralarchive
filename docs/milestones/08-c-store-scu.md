# Milestone 08: C-STORE SCU Sender

## Goal

Implement a C-STORE SCU that sends a local DICOM Part 10 file to a remote
DICOM storage SCP.

This milestone completes the basic outbound send path for `vna-send`.

## Current Status

Not implemented.

Current `vna-send` supports real `--echo-only` C-ECHO. Non-echo mode prints
`C-STORE is not implemented yet.` and exits non-zero.

## Scope

Implement under `src/dicom`:

- DCMTK-based C-STORE SCU;
- association setup and cleanup;
- presentation context negotiation based on the file's SOP Class and transfer
  syntax where feasible;
- real C-STORE request sending;
- mapping of returned DIMSE status to `vna::domain::StoreResult`;
- integration with `vna-send --file`.

Reuse:

- `SendConfig`;
- `DicomNode`;
- `DicomFileReader`;
- `EchoScu` only if an explicit pre-send echo option is added within scope.

## Out Of Scope

Do not implement:

- retry policy;
- Redis queue;
- async send pipeline;
- batch send;
- DICOMweb STOW-RS;
- MinIO;
- PostgreSQL;
- audit trail;
- UI.

## Design Requirements

- Public headers must not expose raw DCMTK types.
- C-STORE success must come from the real DICOM response status.
- File parsing errors must remain `DicomFileReadError`.
- Send/association errors should use a project-owned DICOM send error type.
- `vna-send --echo-only` behavior must remain unchanged.
- The domain layer must remain DCMTK-free.

## Acceptance Criteria

- `vna-send --file <path> --host <host> --port <port> --called-ae <ae>
  --calling-ae <ae>` sends the file to a real storage SCP.
- Successful remote C-STORE returns exit code 0.
- Remote non-success status returns non-zero and a clear English message.
- Invalid local DICOM file returns non-zero and a clear English message.
- Manual or automated validation against DCMTK `storescp` is documented.
- Strict warning build succeeds.
- No retry, queue, database, audit, DICOMweb, or UI code is added.

## Suggested Manual Validation

Start a storage SCP:

```bash
storescp -v -aet TEST_SCP 11112
```

Send a file:

```bash
./build/bin/vna-send --file /path/to/sample.dcm --host 127.0.0.1 \
    --port 11112 --called-ae TEST_SCP --calling-ae VNA
```

## RTM/SKS Alignment

This milestone contributes to outbound DICOM DIMSE requirements. It does not
complete the external RTM targets for retry, pre-send echo policy, async queue,
audit, performance, or DICOMweb STOW-RS.

## Completion Notes

Not implemented yet.
