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

Implemented.

### What was added

- `src/storage/` (new layer):
  - `InstanceStorage`: minimal abstract interface with `prepareDestination`
    (allocate target path) and `isPersisted` (post-write verification).
  - `LocalInstanceStorage`: filesystem-backed implementation. Creates the
    storage root on construction, rejects unsafe SOP Instance UIDs (empty,
    `..`, `/`, `\`), and writes one file per instance named
    `<sop-instance-uid>.dcm` under the root.
  - `StorageError`: exception type for storage failures.
- `src/dicom/StoreScp`:
  - DCMTK-based C-STORE SCP using `dcmnet`/`dcmdata`.
  - Negotiates a small initial storage SOP Class set (Verification,
    Secondary Capture, CT, MR, CR, DX-for-Presentation, US) with three
    uncompressed transfer syntaxes (Little Endian Explicit/Implicit,
    Big Endian Explicit).
  - Validates the called AE title and rejects mismatches with
    `ASC_REASON_SU_CALLEDAETITLENOTRECOGNIZED`.
  - Receives the dataset in memory via `DIMSE_receiveDataSetInMemory`,
    asks storage for a destination path, writes the file via
    `DcmFileFormat::saveFile`, then verifies persistence before
    sending a real DICOM response.
  - Returns DICOM status `0x0000` (Success) only after persistence is
    verified. Returns `0xA700` (Out of Resources) if storage refuses a
    destination, and `0xC001` (Cannot understand: processing failure)
    if writing or post-write verification fails.
  - Also handles in-association C-ECHO so DCMTK's verification probes
    (and `vna-send --echo-only`) work against the same listener.
  - Uses a 1-second `DUL_NOBLOCK` accept poll so a signal-driven
    `requestShutdown()` exits the loop cleanly within ~1 second.
- `apps/vna-receiver/main.cpp`:
  - Replaces the placeholder with a real listener: constructs
    `LocalInstanceStorage`, constructs `StoreScp`, and runs it.
  - Installs SIGINT/SIGTERM handlers that call `StoreScp::requestShutdown()`
    for a clean exit.
  - Surfaces `ConfigError`, `StorageError`, and `StoreScpError` with
    distinct exit codes.
- `tests/unit/local_instance_storage_validation.cpp`: unit test covering
  construction, path allocation, persistence verification, and unsafe-UID
  rejection.

### Verification

- `cmake -S . -B build && cmake --build build` succeeds.
- `ctest --test-dir build` — 4/4 tests pass (existing DICOM reader tests,
  send CLI validation, plus new `local_instance_storage_validation`).
- Strict build
  `cmake -S . -B build-strict -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Werror"`
  succeeds and `ctest --test-dir build-strict` is 4/4 green.

### Manual validation with DCMTK `storescu`

Performed on 2026-05-17 against the receiver compiled from this milestone
on macOS (DCMTK 3.7.0 from Homebrew).

```
./build/bin/vna-receiver --host 127.0.0.1 --port 11133 \
    --ae-title VNA --storage-path /tmp/vna-store
```

Observed behavior, captured from the receiver log:

```
vna-receiver: StoreScp: listening as AE 'VNA' on 127.0.0.1:11133
vna-receiver: Association accepted: calling='STORESCU', called='VNA'
vna-receiver: C-STORE: persisted 1.2.3.4.5.6.7.8.9.0 to /tmp/vna-store/1.2.3.4.5.6.7.8.9.0.dcm
vna-receiver: C-STORE: returned status 0x0000
vna-receiver: Association accepted: calling='TEST_SCU', called='VNA'
vna-receiver: Received C-ECHO request
vna-receiver: Association rejected: called AE 'WRONG' does not match local AE 'VNA'
vna-receiver: StoreScp: stopped
```

Cross-checks:

- `storescu -aec VNA 127.0.0.1 11133 build/tests/unit/positive_instance.dcm`
  reports `Received Store Response (Success)` and the receiver writes
  `/tmp/vna-store/1.2.3.4.5.6.7.8.9.0.dcm` (480 bytes, recognized by
  `file(1)` as `DICOM medical imaging data`).
- `vna-send --echo-only` against the receiver still completes
  `C-ECHO successful`.
- `storescu -aec WRONG ...` is rejected at the association level with
  `Reason: Called AE Title Not Recognized`; no file is created.
- A storage path that exists as a regular file (not a directory) causes
  the receiver to print
  `storage error: failed to create storage root '...': File exists`
  and exit with code 1, rather than silently swallowing the error.

### DCMTK isolation re-check

- `src/domain`, `src/config`, `src/storage`, and `apps/` contain no
  DCMTK includes, symbols, or link dependencies.
- `src/dicom/*.h` (public headers) reference only project types
  (`storage::InstanceStorage`, `StoreScpError`, etc.). DCMTK headers
  are confined to `src/dicom/*.cpp` implementation files.
- `vna_dicom` links `DCMTK::dcmdata` and `DCMTK::dcmnet` as `PRIVATE`,
  so DCMTK does not leak to consumers (`vna-receiver` only sees
  `vna_dicom`, `vna_config`, and `vna_storage`).

### Limitations carried over to future milestones

- Storage layout is flat under the configured root (`<root>/<uid>.dcm`).
  Deterministic study/series-aware layout is deferred to Milestone 06.
- No metadata index is created on receive; the file is persisted but
  not indexed. Indexing is Milestone 07.
- No AE-title whitelist beyond matching the configured called AE; an
  allow-list and audit trail are part of Phase 5.
- No TLS, authentication, or rate limiting.
- The accepted SOP Class list is intentionally small. Expansion to the
  broader radiology set is part of Phase 3 hardening.
- Only uncompressed transfer syntaxes are negotiated.

## Review Fix Notes

Applied on 2026-05-17 in response to a Milestone 05 review pass.

### Fix 1 + 2: Request vs dataset UID consistency

`StoreScp::handleCStore` now reads both `DCM_SOPInstanceUID` and
`DCM_SOPClassUID` from the received dataset and compares them to the
C-STORE request's `AffectedSOPInstanceUID` / `AffectedSOPClassUID`.

- On either mismatch, the receiver:
  - logs a clear English message naming both the request value and the
    dataset value;
  - does **not** call `DcmFileFormat::saveFile`, so no file is written;
  - returns DICOM status `0xA900`
    (`STATUS_STORE_Error_DataSetDoesNotMatchSOPClass`) to the SCU.

The mismatch checks run before `prepareDestination` is called, so the
storage layer is never asked to allocate a path for an inconsistent
message.

### Fix 3: DICOM UID format validation before storage

Added `src/domain/DicomUid.{h,cpp}` containing
`isValidDicomUid` and `requireValidDicomUid`. The validator enforces
PS3.5 §9.1: 1-64 characters, only digits and `'.'`, no leading/trailing
dot, no empty components, and no multi-digit component that starts with
`'0'`. It is implemented entirely with `std::string` so the domain
layer stays DCMTK-free.

`handleCStore` calls `domain::isValidDicomUid` on the dataset's SOP
Instance UID before passing it to the storage abstraction. If the UID
is malformed, the receiver:

- logs a clear English message including the offending UID;
- does **not** write a file;
- returns DICOM status `0xC001` (Cannot understand: processing failure)
  to the SCU.

`LocalInstanceStorage::prepareDestination` keeps its existing
path-safety rejection of `..`, `/`, and `\` as defense-in-depth, but the
authoritative "is this a DICOM UID?" check now lives in the domain
layer where it can be reused.

### Fix 4: `--host` is informational only

DCMTK 3.7.0's `ASC_initializeNetwork(NET_ACCEPTOR, port, timeout, &net)`
does not expose a bind-address parameter. There is no documented public
DCMTK API in this version that lets an acceptor bind to a specific local
host or interface; the listening socket is always bound to all
interfaces. We are not faking a host-bind by ignoring or silently
accepting the value.

To make the actual behavior unambiguous to the operator:

- `vna-receiver`'s startup output now annotates the host as
  `informational; DCMTK acceptor binds to all interfaces`.
- `StoreScp::run`'s listen message now reads
  `listening as AE '<AE>' on port <port>
  (all interfaces; advertised host '<host>')`.

The CLI surface and `ReceiverConfig::host` are unchanged. The value is
preserved for future use (logging, conformance statement metadata, and
a possible future explicit bind path if a clean API or a POSIX-socket
replacement is adopted).

### Fix 5: Tests

- Added `tests/unit/dicom_uid_validation.cpp` with positive cases
  (Verification SOP Class, Secondary Capture SOP Class, fixture SOP
  Instance UID, max-length UID, single-zero component) and negative
  cases (empty, oversize, leading/trailing/consecutive dots,
  leading-zero components, non-digits, path-separator characters).
- The end-to-end mismatch paths (request UID != dataset UID) require
  driving DCMTK's SCU to send a deliberately inconsistent C-STORE
  request, which is more involved than this milestone's scope. The
  mismatch handlers are exercised manually via code review of
  `handleCStore` and via the UID validator unit tests; full DIMSE-level
  mismatch testing is documented here as a known coverage gap to be
  closed in a later milestone alongside the C-STORE SCU work
  (Milestone 08), which will give us a programmable SCU client.

### Re-verification

- `cmake --build build` succeeds.
- `ctest --test-dir build --output-on-failure` — 5/5 pass
  (new `dicom_uid_validation` test included).
- `cmake -S . -B build-strict -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Werror"`
  builds clean and `ctest --test-dir build-strict` is 5/5 green.
- End-to-end smoke check with `storescu` against the rebuilt receiver
  still persists `1.2.3.4.5.6.7.8.9.0.dcm` and returns DICOM status
  `0x0000`.
- DCMTK isolation re-audited: no DCMTK headers, raw DCMTK types, DIMSE/ASC
  symbols, or link dependencies leak into `src/domain`, `src/config`,
  `src/storage`, `apps/`, or any `src/dicom/*.h`; `vna_dicom` still links
  `DCMTK::dcmdata` and `DCMTK::dcmnet` as `PRIVATE`. User-facing help/log text
  may mention DCMTK to explain the current all-interfaces bind behavior.

## Final Cleanup Notes

Applied on 2026-05-17 as the pre-commit cleanup pass for Milestone 05.

### Cleanup 1: One canonical DICOM UID validator

`DicomInstanceMetadata.cpp` previously carried its own private UID
validator (`isValidUid` / `requireValidUid`) that was looser than the
shared `domain::isValidDicomUid` introduced for the C-STORE SCP — it
did not enforce the PS3.5 Section 9.1 rule that a multi-digit component
may not start with `'0'`.

The private validator has been removed. `DicomInstanceMetadata`'s
constructor now calls `domain::requireValidDicomUid` for SOP Instance
UID, SOP Class UID, Study Instance UID, and Series Instance UID; the
existing error-message prefix (`DicomInstanceMetadata ...`) is preserved
so downstream callers (notably `DicomFileReader`, which wraps
`std::invalid_argument` into `DicomFileReadError`) see the same error
type and surface. The domain layer remains DCMTK-free.

The PS3.5 Section 9.1 rule set is now the single source of truth for
DICOM UID syntax across the project: the C-STORE SCP, the metadata
value object, and the file reader all go through the same domain check.

### Cleanup 2: `--host` help text matches actual behavior

`vna-receiver --help` previously described `--host` as `Bind hostname
or IP address`. With DCMTK 3.7.0's acceptor binding to all interfaces
(documented in Review Fix 4 above), that wording promised behavior we
do not implement. The help text now reads:

```
--host <host>             Informational/advertised host only.
                          The DCMTK acceptor always binds to all
                          local interfaces; this value is logged
                          and reserved for future explicit-bind
                          support.
```

### Cleanup 3: ASCII-only source comments

The non-ASCII section symbol in the `DicomUid.h` header comment
(`PS3.5 §9.1`) has been replaced with the ASCII form
(`PS3.5 Section 9.1`). A repo-wide scan confirms no other non-ASCII
characters remain in `src/`, `apps/`, or `tests/`.

### Cleanup re-verification

- `cmake --build build` clean; `ctest --test-dir build` — 5/5 pass.
- `cmake --build build-strict` (with `-Wall -Wextra -Wpedantic -Werror`)
  clean; `ctest --test-dir build-strict` — 5/5 pass.
- DCMTK isolation re-confirmed: no DCMTK headers, raw DCMTK types, DIMSE/ASC
  symbols, or link dependencies in `src/domain`, `src/config`, `src/storage`,
  `apps/`, or `src/dicom/*.h`. User-facing help/log text may mention DCMTK to
  explain the current all-interfaces bind behavior.
