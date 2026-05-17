# Compliance Scope

## Purpose

This document records how the externally provided SKS/TITCK/KVKK/IHE-oriented
documents should influence the VNA roadmap.

It is not a legal opinion, certification claim, DICOM Conformance Statement, or
TITCK submission package. It is an engineering scope document for the current
C++ repository.

## Source Documents Reviewed

- `SKS_TITCK_Uyumluluk.md`
- `RTM.md`
- `RTM.xlsx`

The source documents describe the final production VNA/PACS Engine target with
DICOM, DICOMweb, storage, database, admin panel, audit, security, DevOps,
monitoring, worklist/HL7/FHIR, and certification evidence. All of these are
part of the long-term product scope. The current repository implements only the
early C++ DICOM foundation.

## Current Compliance Posture

The current project can provide limited engineering evidence for:

- C++17 buildability through CMake.
- Basic source control readiness.
- Layered architecture documentation.
- ADRs for DCMTK usage and layered architecture.
- Domain/config separation from DCMTK.
- DCMTK-based DICOM file parsing.
- Real C-ECHO SCU behavior.
- Small automated test suite through CTest.

The current project cannot claim SKS/TITCK/KVKK/IHE compliance.

## Current Evidence Available

| Evidence Area | Current Evidence |
|---|---|
| Architecture | `docs/ARCHITECTURE.md`, `docs/adr/0001-use-dcmtk.md`, `docs/adr/0002-layered-architecture.md`. |
| Requirements planning | `docs/PROJECT_BRIEF.md`, `docs/ROADMAP.md`, milestone files, and this scope assessment. |
| Build | CMake project builds C++17 libraries and CLI apps. |
| Unit verification | CTest tests for DICOM file reader and send config validation. |
| DICOM file parsing | `DicomFileReader` extracts core metadata from a real generated Part 10 fixture. |
| DICOM networking | `EchoScu` performs real C-ECHO SCU over DCMTK. |
| Tool isolation | DCMTK usage is limited to `src/dicom` implementation files. |

## Missing Compliance Evidence

The following are missing and must not be claimed as complete:

- DICOM Conformance Statement under PS3.2.
- Full RTM maintained against current code.
- Formal SKS/TITCK compliance matrix for this repository.
- IEC 62304 software development plan, architecture/design evidence,
  verification evidence, maintenance process, and problem resolution process.
- ISO 14971 risk management file.
- KVKK personal data processing inventory.
- Security architecture and threat model.
- Audit trail implementation and retention policy.
- DICOM TLS implementation and evidence.
- Access control and authentication implementation.
- Backup, restore, continuity, and disaster recovery implementation.
- Performance and load test reports.
- Release validation protocol.
- Production deployment documentation.

## Compliance-Oriented Final Scope

### DICOM Evidence

- Write a DICOM Conformance Statement only after C-STORE SCP, C-STORE SCU,
  C-FIND SCP, and C-MOVE SCP are implemented and tested.
- Document supported AE titles, ports, SOP Classes, transfer syntaxes,
  association limits, DIMSE roles, status codes, and known limitations.
- Keep C-ECHO SCU documented as implemented; do not claim C-ECHO SCP until it
  exists.

### SKS / KVKK / Security Evidence

- Define audit events before implementing audit persistence.
- Add audit events for C-STORE, C-FIND, C-MOVE, authentication, configuration,
  and data export workflows when those workflows exist.
- Add AE title whitelist before exposing receiver workflows in controlled
  environments.
- Add DICOM TLS before claiming encrypted DIMSE transport.
- Add clear data retention, deletion, backup, and restore documentation when
  storage exists.

### TITCK / IEC 62304 Evidence

- Maintain a repository-based RTM where every requirement maps to code,
  tests, and status.
- Keep milestone completion notes as verification evidence.
- Record known risks and mitigations before implementing storage, retrieval,
  deletion, and authentication.
- Treat performance targets as unverified until measured by repeatable tests.

### IHE Evidence

- ATNA requires more than local log messages. A future implementation must
  define audit event semantics, persistence, access control, and optionally
  external audit transport.
- SWF, MPPS, MWL, HL7, and FHIR are part of the final target scope, but they
  should be implemented after the core archive workflow is stable unless the
  roadmap is explicitly reprioritized.
- Connectathon readiness requires real interoperability testing, not only unit
  tests or documentation.

## Documentation Rules For Compliance Claims

- Use "target" for requirements imported from the attached RTM/SKS documents.
- Use "planned" for roadmap items not implemented yet.
- Use "implemented" only when current source code and tests support the claim.
- Use "validated" only when repeatable tests or documented manual validation
  exist.
- Do not describe external deployment components such as MinIO, PostgreSQL,
  Redis, Celery, Docker, FastAPI, nginx, or an admin panel as current parts of
  this repository until they are actually added.
