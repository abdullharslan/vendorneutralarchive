# Roadmap

## Current Status

The repository currently implements milestones 00 through 04:

- project skeleton;
- domain models;
- configuration layer;
- DICOM file reader;
- C-ECHO SCU.

The external RTM/SKS documents describe the intended final VNA/PACS target.
Those documents are treated as final-scope inputs, but not as current
implementation status. See:

- `docs/RTM_SCOPE_ASSESSMENT.md`
- `docs/COMPLIANCE_SCOPE.md`

## Phase 1: Foundation

Completed:

- 00 Project skeleton
- 01 Domain models
- 02 Configuration layer
- 03 DICOM file reader
- 04 C-ECHO SCU

## Phase 2: Archive MVP Core

These milestones build the first usable DICOM archive workflow. They must stay
focused on local, minimal infrastructure. Do not add MinIO, PostgreSQL, Redis,
Celery, DICOMweb, HL7, or UI in this phase.

- 05 C-STORE SCP receiver
  - Receive real C-STORE requests.
  - Persist files locally through a minimal storage abstraction.
  - Return real DICOM success/failure statuses.
- 06 Local file storage
  - Harden the local storage abstraction and deterministic file layout.
  - Add readback support needed by C-MOVE.
  - Add storage-focused tests.
- 07 Metadata index
  - Index core instance metadata in a local in-process implementation.
  - Support query inputs needed by C-FIND and C-MOVE.
- 08 C-STORE SCU sender
  - Send a local DICOM file to a remote C-STORE SCP.
  - Reuse `DicomFileReader` and `EchoScu` where appropriate.
- 09 C-FIND SCP
  - Answer study/series/instance metadata queries from the local index.
  - Return real pending/final DICOM statuses.
- 10 C-MOVE SCP
  - Retrieve stored instances from local storage and send them to a move
    destination.
  - Return real C-MOVE progress/final statuses.

## Phase 3: MVP Hardening

These items are required before making strong interoperability or operational
claims:

- C-ECHO SCP role.
- Configurable association limits.
- Configurable transfer syntax ordering.
- Common radiology storage SOP Class coverage.
- Failure-status mapping for C-STORE, C-FIND, and C-MOVE.
- Integration tests against DCMTK/dcm4che/Orthanc tools where appropriate.
- DICOM Conformance Statement draft based on actual implemented roles.

## Phase 4: Production Storage And Persistence

These items move the local MVP toward the final archive platform:

- PostgreSQL metadata backend.
- Database schema/migration strategy.
- MinIO/S3 object storage backend.
- Object layout compatible with study/series/instance identity.
- Storage readback for retrieve workflows.
- Duplicate instance policy.
- Data integrity checksums if selected for the final design.
- Backup and restore documentation.

## Phase 5: Security And Audit

These items are part of the final product scope:

- AE title whitelist.
- Strict/permissive whitelist mode if required.
- DICOM tag validation policy.
- DICOM TLS (PS3.15).
- Authentication and authorization for non-DIMSE surfaces.
- RBAC for administrative workflows.
- Failed-login lockout for admin/API authentication.
- Audit event model.
- Audit persistence.
- Audit logging for DICOM operations.
- Audit logging for user/admin operations.
- Immutable audit retention rules.
- Audit export if required by the final UI/API scope.

## Phase 6: DICOMweb

These items are part of the final product scope after core DIMSE archive
workflows exist:

- QIDO-RS Study, Series, and Instance queries.
- WADO-URI instance retrieval.
- WADO-RS instance retrieval.
- STOW-RS upload.
- DICOM JSON formatting according to PS3.18.
- Multipart request/response handling.
- DICOMweb integration tests.

## Phase 7: Worklist, HL7, And FHIR

These items are part of the final product scope:

- Modality Worklist (MWL) C-FIND SCP.
- HL7 v2.x MLLP listener if required by the integration design.
- ORM/ORU message handling if required by the integration design.
- MPPS integration with worklist state.
- FHIR ImagingStudy if selected for the final integration scope.

## Phase 8: Internal API And Admin Panel

These items are part of the final product scope:

- Internal HTTP API for health/status/studies/devices/logs/settings/audit.
- Admin dashboard.
- Study list and filters.
- Device management.
- C-ECHO test action for configured devices.
- Logs and audit views.
- Settings view.
- RBAC-protected pages and actions.

## Phase 9: Monitoring, DevOps, And Production Deployment

These items are part of the final product scope:

- Docker production deployment.
- Service health checks.
- Watchdog/monitoring service.
- Email/SMS or equivalent alerting if selected.
- Environment-based configuration.
- Crash restart policy.
- Operational backup/restore runbooks.
- Production deployment documentation.

## Phase 10: Performance, Regression, And Compliance Evidence

These items are part of the final product scope:

- C-STORE load tests.
- C-FIND load tests.
- Concurrent association tests.
- Daily instance capacity tests.
- Full DIMSE regression suite.
- DICOMweb regression suite.
- DICOM Conformance Statement.
- Repository-maintained RTM.
- SKS/TITCK/KVKK/IHE evidence package.
- IEC 62304 software lifecycle evidence.
- ISO 14971 risk management file.
- Release validation protocol.

## Compliance Evidence Scope

These items are part of the final product scope, but they become claimable only
after implementation and evidence exist:

- AE title whitelist.
- DICOM TLS (PS3.15).
- Audit event model.
- Audit persistence.
- Audit logging for DICOM operations.
- Immutable audit retention rules.
- KVKK-oriented data inventory documentation.
- IEC 62304 evidence package.
- ISO 14971 risk management file.
- Release validation protocol.

## Final Product Target Summary

The following infrastructure and product expansion items remain part of the
final product scope:

- PostgreSQL metadata backend.
- MinIO/S3 object storage backend.
- Redis/Celery-style asynchronous processing.
- DICOMweb: QIDO-RS, WADO-URI, WADO-RS, STOW-RS.
- Internal HTTP API.
- Admin panel.
- Worklist / HL7 / FHIR.
- Monitoring/watchdog/alerting.
- Docker production deployment.
- Performance and load test suite.

The original advanced target list is also preserved explicitly:

- DICOMweb
- Storage Commitment
- MWL / HL7
- MinIO
- PostgreSQL
- production deployment

Additional final product scope from the RTM/SKS documents:

- C-ECHO SCP
- C-STORE SCP and SCU
- C-FIND SCP
- C-MOVE SCP
- C-GET SCP if prioritized
- MPPS
- Transfer syntax and SOP Class expansion
- AE whitelist
- DICOM TLS
- Audit trail
- Internal API
- Admin panel
- RBAC/authentication
- Monitoring/watchdog
- Docker/DevOps
- Performance/load/regression testing
- DICOM Conformance Statement
- SKS/TITCK/KVKK/IHE evidence

## Status Rules

- Final-scope items must stay visible in this roadmap.
- Current implementation status must still be based only on code and tests.
- Do not claim SKS/TITCK/KVKK/IHE compliance from the current codebase yet.
- Do not claim DICOMweb, MinIO, PostgreSQL, Redis/Celery, admin panel, MWL/HL7,
  audit, or production deployment support until those modules exist.
- Do not mark external RTM rows as complete unless current repository code and
  tests prove them.
