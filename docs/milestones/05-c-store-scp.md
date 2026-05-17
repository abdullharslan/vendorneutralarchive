# Milestone 05: C-STORE SCP Receiver

## Goal

Implement a C-STORE SCP service that receives DICOM files over standard DICOM networking using DCMTK.

## Scope

Implement:
- apps/vna-receiver
- DICOM receiver service
- association handling
- presentation context negotiation
- C-STORE request handling
- save received file through IObjectStorage

Do not implement:
- PostgreSQL
- MinIO
- DICOMweb
- C-FIND
- C-MOVE
- Storage Commitment
- UI

## Acceptance Criteria

- vna-receiver starts with AE title, host, port, and storage path.
- DCMTK storescu can send a DICOM file to vna-receiver.
- The file is saved locally.
- The receiver returns a real DICOM success or failure status.
- No fake success responses are allowed.
- DICOM networking does not depend on database code.

## Implementation Notes

Use DCMTK.
Keep storage behind IObjectStorage.
Keep application orchestration outside the DCMTK class.

## Test Command

Example:

```bash
storescu -aec VNA 127.0.0.1 11112 sample.dcm