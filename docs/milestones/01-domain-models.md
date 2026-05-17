# Milestone 01: Domain Models

## Goal

Implement the initial domain models for the DICOM archive core.

The domain layer must stay independent from DCMTK, storage, database, networking, CLI parsing, and logging.

## Scope

Implement domain models under `src/domain`:

- DicomNode
- DicomFile
- DicomInstanceMetadata
- EchoResult
- StoreResult
- DicomStatus

## Rules

- Use English only.
- Do not add DCMTK dependencies.
- Do not implement DICOM networking.
- Do not implement storage.
- Do not implement database/indexing.
- Do not implement CLI parsing beyond existing placeholders.
- Do not implement fake success responses.

## Acceptance Criteria

- Domain models compile successfully.
- Domain models are independent from DCMTK.
- Basic validation exists where appropriate.
- `vna-send` and `vna-receiver` still build.
- No future milestone functionality is implemented.

## Completion Notes

Implemented on 2026-05-15.

### What was created

Six pure domain types under `src/domain`, all in namespace `vna::domain`:

- `DicomStatus` (`DicomStatus.h/.cpp`) — value object wrapping a 16-bit DIMSE
  service status code. Provides a `DicomStatusCategory` (Success / Warning /
  Failure / Cancel / Pending) derived from the code, helper queries
  (`isSuccess()` and friends), and named factories `success()`, `cancel()`,
  `pending()`. Categorization follows PS3.7: `0x0000` → Success, `0xFE00` →
  Cancel, `0xFFxx` → Pending, `0xBxxx` → Warning, everything else → Failure.
  No fake success: only `0x0000` reports `isSuccess() == true`.
- `DicomNode` (`DicomNode.h/.cpp`) — peer node identity (AE title, host,
  port). Constructor validates AE title (1–16 chars, ASCII printable, no
  backslash, no control chars), non-empty host, and port > 0.
- `DicomFile` (`DicomFile.h/.cpp`) — on-disk DICOM file descriptor
  (path + size in bytes). Constructor requires a non-empty path and
  `sizeBytes > 0`.
- `DicomInstanceMetadata` (`DicomInstanceMetadata.h/.cpp`) — metadata
  extracted from a single SOP instance: SOP Instance UID, SOP Class UID,
  Study Instance UID, Series Instance UID, Patient ID, Modality. UIDs are
  validated as 1–64 chars of digits and dots only, with no leading dot,
  no trailing dot, and no empty components. Modality must be non-empty;
  Patient ID is allowed to be empty (DICOM Type 2).
- `EchoResult` (`EchoResult.h/.cpp`) — result of a C-ECHO: a `DicomStatus`
  plus optional message. `succeeded()` defers to the status.
- `StoreResult` (`StoreResult.h/.cpp`) — result of a C-STORE for one
  instance: SOP Instance UID + `DicomStatus` + optional message. Constructor
  requires a non-empty SOP Instance UID.

All types provide `operator==` / `operator!=`. Validation failures throw
`std::invalid_argument` with a descriptive message.

### Build wiring

- New `src/domain/CMakeLists.txt` defines a static library `vna_domain` and
  publishes `src/` as a `BUILD_INTERFACE` include directory so consumers
  use `#include "domain/<Header>.h"`.
- New `src/CMakeLists.txt` adds the `domain` subdirectory.
- Root `CMakeLists.txt` now calls `add_subdirectory(src)` before
  `add_subdirectory(apps)`. The apps were not modified and do not yet link
  `vna_domain`.

### Verification

- `cmake -S . -B build` configures cleanly with AppleClang 21.0.0 / CMake 4.3.2.
- `cmake --build build` produces `libvna_domain.a` plus the two placeholder
  binaries `build/bin/vna-receiver` and `build/bin/vna-send`.
- A separate strict build with `-DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic
  -Werror"` also builds cleanly with no warnings.
- Both placeholder binaries still run and print the Milestone 00 banner.
- `grep -RIn -E "(dcmtk|DCMTK|dcm[a-z]*\.h|dcm[a-z]*/)" src/ apps/ *CMakeLists*`
  returns no matches, confirming the domain layer does not pull DCMTK in.

### Acceptance criteria check

- Domain models compile successfully: yes (both default and `-Werror`).
- Domain models are independent from DCMTK: yes (no DCMTK headers,
  libraries, or `find_package` calls anywhere in the project).
- Basic validation exists where appropriate: yes — AE title format, port
  range, non-empty host, non-empty file path with `sizeBytes > 0`,
  non-empty Modality, and DICOM UID format (digits and dots, 1–64 chars,
  no leading/trailing dot, no empty components). Patient ID is allowed
  to be empty.
- `vna-send` and `vna-receiver` still build: yes.
- No future milestone functionality is implemented: yes — no networking,
  no file I/O, no storage, no indexing, no CLI parsing, no logging.

### Review fixes (2026-05-15)

Three review findings on the initial implementation were addressed.

1. **`DicomInstanceMetadata::patientId` may now be empty.** The DICOM
   standard defines Patient ID (`0010,0020`) as Type 2 in most composite
   IOD modules — the tag must be present but its value may be zero-length
   (e.g. unidentified or anonymous patients). The earlier non-empty check
   was over-restrictive and would have rejected legitimate inbound studies.
   Removed the `requireNonEmpty(patientId_, ...)` call; Modality is still
   required to be non-empty.
2. **DICOM UID validation is stricter.** `isValidUid` in
   `DicomInstanceMetadata.cpp` now also rejects UIDs that start with `.`,
   end with `.`, or contain a `..` substring. This enforces the PS3.5 rule
   that UIDs are non-empty numeric components joined by dots (no empty
   component, no leading/trailing separator). The error message in
   `requireValidUid` was updated to mention these constraints.
3. **`DicomFile` now rejects zero-byte instances.** A real DICOM Part 10
   file is at minimum a 128-byte preamble plus the `DICM` magic, so a
   size of 0 bytes is always invalid. The constructor now throws
   `std::invalid_argument` if `sizeBytes == 0`. No "unknown size" escape
   hatch is needed: every current and planned caller obtains the size
   from the filesystem or from the C-STORE payload before constructing
   the object.

Re-verified after the fixes:

- Default build: `cmake --build build` succeeds, both placeholder binaries
  rebuilt and still run.
- Strict build with `-Wall -Wextra -Wpedantic -Werror` is also clean.