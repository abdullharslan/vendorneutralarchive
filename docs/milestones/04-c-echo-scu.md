# Milestone 04: C-ECHO SCU

## Goal

Implement a DCMTK-based C-ECHO SCU that verifies connectivity to a remote DICOM node.

This milestone introduces DICOM networking only for C-ECHO. It must not implement C-STORE, C-FIND, C-MOVE, C-GET, Storage Commitment, DICOMweb, HL7, storage, or database logic.

## Scope

Implement under `src/dicom`:

- `EchoScu`
- `EchoScuError` if needed

Integrate `vna-send --echo-only` so it performs a real C-ECHO using:

- `--host`
- `--port`
- `--called-ae`
- `--calling-ae`
- `--timeout`

## Rules

- Use English only.
- DCMTK networking is allowed only inside `src/dicom`.
- Use DCMTK `dcmnet` only for C-ECHO.
- Do not implement C-STORE.
- Do not implement C-FIND, C-MOVE, C-GET.
- Do not implement DICOMweb, HL7, Storage Commitment.
- Do not implement storage logic.
- Do not implement database/indexing.
- Do not create fake success responses.
- Do not log instead of sending a real DICOM response/request.
- Public headers must not expose raw DCMTK association types.

## Design Requirements

- `EchoScu` must take domain/config values, not raw `argv`.
- `EchoScu` must return `vna::domain::EchoResult`.
- `vna-send --echo-only` must call `EchoScu`.
- Non-echo `vna-send` mode must still not send C-STORE yet.
- DICOM networking code must stay inside `src/dicom`.
- Domain types must remain DCMTK-free.
- Config types must remain DCMTK-free.
- Association release/abort handling must be explicit and safe.

## Acceptance Criteria

- `vna-send --echo-only --host <host> --port <port> --called-ae <ae> --calling-ae <ae>` performs a real C-ECHO.
- Success returns exit code `0`.
- Failure returns a non-zero exit code and a clear English error.
- `vna-send` without `--echo-only` still prints that C-STORE is not implemented.
- Build succeeds.
- Strict warning build succeeds.
- Tests or manual validation document how to verify against a local DCMTK `echoscp`.
- No C-STORE, C-FIND, C-MOVE, C-GET, DICOMweb, HL7, Storage Commitment, storage, or database logic is implemented.

## Manual Validation Example

Run an echo SCP in one terminal:

```bash
echoscp -v -aet TEST_SCP 11112
```

Then in another terminal:

```bash
./build/bin/vna-send --echo-only --host 127.0.0.1 --port 11112 \
    --called-ae TEST_SCP --calling-ae VNA
```

If the Homebrew DCMTK bottle does not include `echoscp`, substitute
`storescp -v -aet TEST_SCP 11112` (it accepts the Verification SOP
Class by default, so the C-ECHO wire test is the same).

## Completion Notes

Implemented on 2026-05-15.

### What was created

In `src/dicom`, namespace `vna::dicom`:

- `EchoScuError.h` — `std::runtime_error` subclass for any
  association-level / DIMSE I/O failure (network init, parameter
  creation, request rejected, TCP error, release failure, etc.).
- `EchoScu.h` — DCMTK-free public header. It only includes
  `domain/DicomNode.h` and `domain/EchoResult.h` and declares:
  ```cpp
  class EchoScu {
  public:
      domain::EchoResult echo(const std::string& callingAe,
                              const domain::DicomNode& peer,
                              std::uint32_t timeoutSeconds) const;
  };
  ```
- `EchoScu.cpp` — the only translation unit in the project that
  includes `<dcmtk/dcmnet/*>`. The function is a single, explicit
  sequence:
  1. `ASC_initializeNetwork(NET_REQUESTOR, …)`.
  2. `ASC_createAssociationParameters(…, ASC_DEFAULTMAXPDU,
     tcpConnectTimeout)` — note the new 3-argument form; the
     2-argument overload is deprecated in DCMTK 3.7.0 and would not
     compile under `-Werror`.
  3. `ASC_setAPTitles(params, callingAe, peer.aeTitle(), nullptr)`.
  4. `ASC_setPresentationAddresses(params, "localhost", "<host>:<port>")`.
  5. `ASC_addPresentationContext(params, 1, UID_VerificationSOPClass,
     {LE Explicit, BE Explicit, LE Implicit}, 3)`.
  6. `ASC_requestAssociation(net, params, &assoc)`.
  7. `ASC_countAcceptedPresentationContexts(params) == 0` →
     `EchoScuError("peer did not accept the Verification SOP Class")`
     after `ASC_abortAssociation`.
  8. `DIMSE_echoUser(assoc, msgId, DIMSE_BLOCKING, timeout, &status,
     &statusDetail)`, deleting `statusDetail` immediately.
  9. `ASC_releaseAssociation` on success / `ASC_abortAssociation` on
     any failure between request and release, then
     `ASC_destroyAssociation`, then `ASC_dropNetwork`.
  10. Build `domain::EchoResult{DicomStatus{status}, message}` where
     the message is `"C-ECHO successful"` only when
     `DicomStatus::isSuccess()` is true (so the rule "no fake success
     responses" is upheld: the response message is derived from the
     actual DIMSE status returned by the peer).

  All raw DCMTK handles (`T_ASC_Network*`, `T_ASC_Parameters*`,
  `T_ASC_Association*`) are local variables inside the function, never
  exposed to callers. Release / abort / destroy / drop calls are
  explicit on every code path.

### App wiring

- `apps/vna-send/CMakeLists.txt` now also links `vna_dicom`
  (`vna_config` was already linked).
- `apps/vna-send/main.cpp` is split into two small helpers:
  - `runEchoOnly(cfg, programName)` — constructs a
    `vna::domain::DicomNode` from `cfg.host / cfg.port / cfg.calledAe`,
    creates an `EchoScu`, calls `echo(cfg.callingAe, peer,
    cfg.timeoutSeconds)`. On success: prints `vna-send: C-ECHO
    successful` to stdout and returns 0. On peer-level non-success
    status or `EchoScuError`: prints the message to stderr and
    returns 1.
  - `runNonEcho(cfg)` — prints the parsed configuration and the line
    `C-STORE is not implemented yet.`, then returns 1. No bytes are
    sent on the wire; there is no fake success.
- Help and `ConfigError` paths are unchanged from Milestone 02.

### Build wiring

- `src/dicom/CMakeLists.txt` now compiles `EchoScu.cpp` alongside the
  existing `DicomFileReader.cpp` and adds `DCMTK::dcmnet` to the
  **PRIVATE** link list (next to the existing `DCMTK::dcmdata`). No
  consumer of `vna_dicom` sees DCMTK headers or libraries.

### Verification

- `cmake --build build` succeeds.
- A separate strict build with `-Wall -Wextra -Wpedantic -Werror` also
  succeeds. (First pass tripped the deprecated 2-arg
  `ASC_createAssociationParameters`; switching to the 3-arg form fixed
  it. No warning suppressions were needed.)
- `ctest --test-dir build --output-on-failure` still reports
  `2/2 tests passed` — the file-reader tests from Milestone 03 are
  unaffected.

**DCMTK isolation re-checked**:
- `grep -RIn -E "(dcmtk|DCMTK)" src/domain` — no matches.
- `grep -RIn -E "(dcmtk|DCMTK)" src/config` — no matches.
- `grep -RIn -E "(dcmtk|DCMTK|<dcmtk/)" apps` — no matches.
- `grep -RIn -E "(<dcmtk/)" src/dicom/*.h` — no matches; DCMTK
  appears only in `src/dicom/EchoScu.cpp` and
  `src/dicom/DicomFileReader.cpp`.

**Live validation against a local DCMTK SCP.**
`echoscp` is not shipped in the Homebrew DCMTK 3.7.0 bottle (only
`echoscu` is), so `storescp` was used instead — it accepts the
Verification SOP Class by default, so the C-ECHO wire exchange is
identical. SCP started with:

```
storescp -v -aet TEST_SCP 11112
```

Cases exercised:

| Scenario | Command | Expected | Actual |
|---|---|---|---|
| Echo against running SCP | `vna-send --echo-only --host 127.0.0.1 --port 11112 --called-ae TEST_SCP --calling-ae VNA` | exit 0 | exit 0, `vna-send: C-ECHO successful` |
| Echo with `--timeout 5` | same + `--timeout 5` | exit 0 | exit 0, `C-ECHO successful` |
| Echo to wrong port (no SCP) | same with `--port 11199 --timeout 3` | non-zero with clear English error | exit 1, `vna-send: association request failed: TCP Initialization Error: Operation now in progress` |
| Non-echo mode | `vna-send --host 127.0.0.1 --port 11112 --called-ae TEST_SCP --calling-ae VNA --file /tmp/fake.dcm` | prints "C-STORE is not implemented yet." and exits non-zero | exit 1, prints parsed config plus `C-STORE is not implemented yet.` |

The SCP log confirmed the two successful associations:

```
I: Association Received
I: Association Acknowledged (Max Send PDV: 16372)
I: Received Echo Request (MsgID 1)
I: Association Release
I: Association Received
I: Association Acknowledged (Max Send PDV: 16372)
I: Received Echo Request (MsgID 1)
I: Association Release
```

### Acceptance criteria check

- `vna-send --echo-only ...` performs a real C-ECHO: yes (verified by
  the SCP log).
- Success returns exit code 0: yes.
- Failure returns a non-zero exit code with a clear English error:
  yes — both wrong-port (TCP failure) and non-echo (C-STORE-not-yet)
  paths return 1 with a descriptive message.
- `vna-send` without `--echo-only` still prints that C-STORE is not
  implemented: yes.
- Build succeeds: yes.
- Strict warning build succeeds: yes.
- Manual validation documented: yes (see this section and the example
  block above).
- No C-STORE / C-FIND / C-MOVE / C-GET / DICOMweb / HL7 / Storage
  Commitment / storage / database logic implemented: yes — the only
  DIMSE call in the codebase is `DIMSE_echoUser`, and the only DCMTK
  link addition is `DCMTK::dcmnet` used solely for that path.

### Notes for future milestones

- The error wording `association request failed: TCP Initialization
  Error: Operation now in progress` is DCMTK's own string for a
  connect-time error (it surfaces `EINPROGRESS` from the underlying
  non-blocking socket). It is descriptive enough for the milestone's
  acceptance criterion; a friendlier mapping ("connection refused",
  "connection timed out") can be added later if needed.
- C-STORE SCU is Milestone 08. When implemented it will reuse the
  domain types (`DicomNode`, `DicomFile`, `DicomInstanceMetadata`,
  `StoreResult`, `DicomStatus`) and the same explicit
  request/release/abort pattern; no association helper or generic SCU
  framework was introduced in this milestone, by design.

## Review Fix Notes

Applied on 2026-05-15 after a review of `EchoScu` robustness.

### Issues addressed

1. **Calling AE title was only validated by the CLI.** `EchoScu::echo`
   previously rejected an empty `callingAe` but otherwise trusted the
   caller, so a direct library caller could pass an over-long or
   non-printable AE title and the bad value would silently propagate
   into `ASC_setAPTitles`. Fixed by calling
   `vna::domain::requireValidAeTitle(callingAe, "calling AE title")`
   inside `EchoScu::echo`. The shared validator's
   `std::invalid_argument` is caught and rethrown as `EchoScuError`,
   so the EchoScu interface keeps its single, documented exception
   type. `EchoScu` no longer relies on CLI validation.

2. **`timeoutSeconds` could overflow when cast to `int`.** The previous
   code did `static_cast<int>(timeoutSeconds)` directly, so any
   `std::uint32_t` value greater than `std::numeric_limits<int>::max()`
   (typically `2147483647`) was undefined behaviour on the cast and
   would also have been passed as a negative `Sint32` to
   `ASC_createAssociationParameters` and `DIMSE_echoUser`. Fixed in two
   places:
   - `EchoScu::echo` now rejects `timeoutSeconds >
     std::numeric_limits<int>::max()` with a clear `EchoScuError` that
     names both the offending value and the supported maximum, before
     the cast.
   - `parseSendCli` now also rejects `--timeout` values greater than
     `std::numeric_limits<int>::max()` with a `ConfigError`, so the
     bad value is caught at the CLI boundary with exit code 2 (config
     error) instead of being caught later by `EchoScu` with exit
     code 1.

   The lower bound (`timeoutSeconds == 0`) check is unchanged.

### Files touched

- `src/dicom/EchoScu.cpp`:
  - Added `#include "domain/AeTitle.h"`, `<limits>`, `<stdexcept>`.
  - Replaced the empty-string check with a `requireValidAeTitle` call
    (rethrown as `EchoScuError`).
  - Added the `timeoutSeconds > int::max` guard before the cast.
- `src/config/SendConfig.cpp`:
  - Added `<limits>`.
  - Added the `--timeout > int::max` guard immediately after parsing
    the `--timeout` value via `parsePositiveUint32`.
- `tests/unit/send_config_validation.cpp` (new):
  - Asserts that `--timeout 4000000000` (above `int::max`, below
    `uint32::max`) is rejected with a `ConfigError`.
  - Asserts that `--timeout 9999999999` (above `uint32::max`) is
    rejected with a `ConfigError` from the existing
    `parsePositiveUint32` range check.
- `tests/unit/CMakeLists.txt`:
  - Registers `send_config_validation` as a CTest target linked
    against `vna_config`.

### Verification

- `cmake --build build` succeeds.
- `ctest --test-dir build --output-on-failure` reports `3/3 tests
  passed` (the new `send_config_validation` plus the two pre-existing
  DicomFileReader tests).
- A separate strict warning build configured with
  `-Wall -Wextra -Wpedantic -Werror` still compiles cleanly with the
  added headers and checks.
- Manual CLI smoke checks against `build/bin/vna-send`:
  - `--timeout 4000000000` → exit 2,
    `--timeout must not exceed 2147483647 seconds, got "4000000000"`.
  - `--timeout 9999999999` → exit 2,
    `--timeout expects a positive integer, got "9999999999"` (from the
    existing `parsePositiveUint32` range check).
  - `--timeout 30` → unchanged behaviour, proceeds to the C-ECHO
    attempt as before.

### Out of scope (unchanged)

- No C-STORE, C-FIND, C-MOVE, C-GET, DICOMweb, HL7, Storage Commitment,
  storage, or database logic was added.
- The public architecture (layer boundaries, header surfaces, DCMTK
  isolation in `src/dicom`) is unchanged. `EchoScu.h` still exposes
  only domain types and `std::uint32_t`; `EchoScuError` and
  `ConfigError` remain the only error types crossing the layer
  boundaries.