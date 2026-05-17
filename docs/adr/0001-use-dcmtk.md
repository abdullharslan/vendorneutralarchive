# ADR 0001: Use DCMTK For DICOM Parsing And Networking

## Status

Accepted.

## Context

VNA needs to implement DICOM archive workflows in C++:

- read DICOM Part 10 files;
- negotiate DICOM associations;
- send and receive DIMSE operations such as C-ECHO, C-STORE, C-FIND, and
  C-MOVE.

Implementing DICOM parsing, transfer syntax handling, association negotiation,
and DIMSE messaging from scratch would be high risk and outside the purpose of
this project. The project needs a proven C++ DICOM toolkit while still keeping
the domain model independent from that toolkit.

The current implementation already uses DCMTK for:

- local DICOM file parsing in `DicomFileReader.cpp`;
- C-ECHO SCU networking in `EchoScu.cpp`.

## Decision

Use DCMTK as the DICOM implementation library.

DCMTK is allowed in the `src/dicom` implementation layer for:

- DICOM Part 10 file parsing;
- DICOM association handling;
- DIMSE service implementation;
- transfer syntax and presentation context negotiation.

Project code must wrap DCMTK behind VNA-owned interfaces and result types.
Domain, configuration, and application-facing headers must not expose raw DCMTK
types.

## Alternatives Considered

### Implement DICOM From Scratch

Rejected. The DICOM standard is large and subtle. Reimplementing parsing,
transfer syntaxes, association negotiation, and DIMSE messaging would create
unnecessary risk and delay the archive design work.

### Use Another C++ DICOM Library

Possible, but DCMTK is mature, widely used, and already provides the file and
networking capabilities needed by the current roadmap.

### Shell Out To DCMTK Command-Line Tools

Rejected for application behavior. CLI tools such as `storescu`, `storescp`,
and `dump2dcm` are useful for validation and fixture generation, but the VNA
application should call library APIs for its runtime behavior. Shelling out
would make error handling, lifecycle management, and testing weaker.

## Consequences

Positive:

- The project uses a proven DICOM implementation.
- File parsing and networking can focus on VNA layer boundaries and behavior.
- Interoperability can be validated against standard DCMTK tools.

Negative:

- DCMTK has C-style APIs in some networking areas, so cleanup paths must be
  explicit and carefully reviewed.
- DCMTK warnings can break strict builds when APIs are deprecated, so milestones
  must compile under `-Werror`.
- The project must actively prevent DCMTK types from leaking into domain and
  config layers.

## DCMTK Isolation Constraints

- DCMTK headers should appear only in `src/dicom/*.cpp` unless an ADR updates
  this decision.
- `src/domain` must have zero DCMTK includes and no DCMTK link dependency.
- `src/config` must have zero DCMTK includes and no DCMTK link dependency.
- `apps/*` must not manage raw DCMTK association or dataset types.
- `vna_dicom` may link DCMTK privately.
- Public `src/dicom` headers must expose VNA-owned types such as
  `DicomFileReadError`, `DicomFileReadResult`, `EchoScu`, and domain value
  objects.
- Milestone completion notes should include an isolation check whenever DCMTK
  usage is added.
