# Milestone 03: DICOM File Reader

## Goal

Implement a DCMTK-based DICOM file reader that reads a local DICOM file and extracts core instance metadata.

This milestone introduces DCMTK for local file parsing only. It must not implement DICOM networking.

## Scope

Implement under `src/dicom`:

- `DicomFileReader`
- `DicomFileReadError`

The reader must:
- Read a local Part 10 DICOM file from disk.
- Validate that the file exists and is non-empty.
- Extract:
  - SOP Instance UID
  - SOP Class UID
  - Study Instance UID
  - Series Instance UID
  - Patient ID
  - Modality
- Return domain objects:
  - `vna::domain::DicomFile`
  - `vna::domain::DicomInstanceMetadata`

## Rules

- Use English only.
- DCMTK is allowed only inside `src/dicom`.
- Do not implement DICOM networking.
- Do not implement C-ECHO, C-STORE, C-FIND, C-MOVE, C-GET, DICOMweb, HL7, Storage Commitment.
- Do not implement storage logic.
- Do not implement database/indexing.
- Do not modify CLI behavior except optionally documenting how the reader will be used later.
- Do not create fake DICOM success responses.

## Design Requirements

- Domain types must remain independent from DCMTK.
- DCMTK headers must not leak into public domain headers.
- DICOM parsing errors must be reported through `DicomFileReadError`.
- Missing required UIDs must fail with clear English errors.
- Patient ID may be empty.
- Modality must be non-empty.

## Acceptance Criteria

- `src/dicom` builds as a separate library, for example `vna_dicom`.
- `vna_dicom` links DCMTK privately.
- `vna_domain` still has no DCMTK dependency.
- Build succeeds.
- Strict warning build succeeds.
- A small validation command or test demonstrates:
  - nonexistent file fails
  - empty file fails
  - invalid DICOM file fails
- No DICOM networking is implemented.

## Completion Notes

Implemented on 2026-05-15.

### What was created

DCMTK 3.7.0 was already available via Homebrew at `/opt/homebrew`. CMake's
`find_package(DCMTK REQUIRED CONFIG)` picked it up directly through the
`/opt/homebrew/lib/cmake/dcmtk/DCMTKConfig.cmake` package — no path hint
needed.

Under `src/dicom`, in namespace `vna::dicom`:

- `DicomFileReadError.h` — `std::runtime_error` subclass used for every
  reader-level failure (filesystem, DCMTK parse, dataset shape, domain
  validation).
- `DicomFileReader.h` — public header. DCMTK-free by design: it only
  pulls in `domain/DicomFile.h` and `domain/DicomInstanceMetadata.h`.
  Declares `class DicomFileReader { DicomFileReadResult read(...) const; }`
  and the small `DicomFileReadResult` aggregate that bundles the two
  domain types.
- `DicomFileReader.cpp` — implementation. This is the only translation
  unit in the project that includes DCMTK headers
  (`dcmdata/dcfilefo.h`, `dcmdata/dcdeftag.h`, `ofstd/ofcond.h`,
  `ofstd/ofstring.h`).

The reader does four things, in order:

1. **Filesystem pre-checks** (no DCMTK involved yet) — empty path,
   nonexistent path, directory instead of file, zero-byte file. Each
   raises a `DicomFileReadError` with a clear English message.
2. **DCMTK parse** — `DcmFileFormat::loadFile`. On `OFCondition::bad()`,
   the DCMTK status text is wrapped into a `DicomFileReadError`.
3. **Tag extraction** — required UIDs (SOP Instance, SOP Class, Study,
   Series), required Modality, and the optional Patient ID. A small
   anonymous helper `requireString` treats both "tag missing" and "tag
   present but value empty" as failure for required fields; an
   `optionalString` helper returns an empty string for Patient ID, which
   is allowed by the domain.
4. **Domain construction** — `domain::DicomFile{path, size}` and
   `domain::DicomInstanceMetadata{...}`. If domain validation throws
   `std::invalid_argument` (e.g. malformed UID per the Milestone 01
   rules), the reader catches it and re-throws as a
   `DicomFileReadError` so callers see a single error type.

### Build wiring

- New `src/dicom/CMakeLists.txt` calls `find_package(DCMTK REQUIRED CONFIG)`
  and defines a static library `vna_dicom`. DCMTK is linked **PRIVATE**
  via the imported target `DCMTK::dcmdata`, so the include and link
  flags do not propagate to consumers. The library publishes only
  `src/` as a public include directory (so consumers write
  `#include "dicom/DicomFileReader.h"`).
- `src/CMakeLists.txt` now does `add_subdirectory(dicom)` after `config`.
- Root `CMakeLists.txt` calls `enable_testing()` and
  `add_subdirectory(tests)`. The existing app build is unchanged; no CLI
  behavior was modified.

A new tests subtree was wired in:

- `tests/CMakeLists.txt` → `add_subdirectory(unit)`
- `tests/unit/CMakeLists.txt` defines a CTest executable
  `dicom_file_reader_validation` linking `vna_dicom`.
- `tests/unit/dicom_file_reader_validation.cpp` synthesises three
  inputs at runtime under `std::filesystem::temp_directory_path()` and
  asserts each one throws `DicomFileReadError`.

### Verification

- `cmake -S . -B build` configures cleanly; DCMTK 3.7.0 found via
  Homebrew config package.
- `cmake --build build` produces `libvna_domain.a`, `libvna_config.a`,
  `libvna_dicom.a`, `vna-receiver`, `vna-send`, and
  `dicom_file_reader_validation`.
- A separate strict build with `-Wall -Wextra -Wpedantic -Werror`
  builds cleanly with no warnings.
- `ctest --output-on-failure` (under both default and strict builds)
  reports `1/1 Test #1: dicom_file_reader_validation ... Passed`.
- Direct test output:

  ```
  [pass] nonexistent file: DICOM file does not exist: /this/path/does/not/exist/vna-test-12345.dcm
  [pass] empty file: DICOM file is empty: /var/folders/.../vna-empty-file.dcm
  [pass] invalid DICOM file: Failed to parse DICOM file /var/folders/.../vna-not-dicom.dcm: I/O suspension or premature end of stream
  ```

  (DCMTK's own logger writes a single `E: DcmElement: ...` diagnostic
  line to stderr for the invalid-file case. That is upstream DCMTK
  behavior and is independent of our error reporting; suppression would
  require configuring `oflog`, which is out of scope for this
  milestone.)

- DCMTK isolation verified by grep:
  - `grep -RIn -E "(dcmtk|DCMTK|dcm[a-z]*\.h)" src/domain` → no matches.
  - `grep -RIn -E "(dcmtk|DCMTK|<dcmtk/)" src/config src/dicom/*.h apps`
    → no matches.
  - `grep -c dcmtk src/dicom/DicomFileReader.cpp` → 4 (the four DCMTK
    `#include` lines, all inside the .cpp).

### Acceptance criteria check

- `src/dicom` builds as a separate library `vna_dicom`: yes.
- `vna_dicom` links DCMTK privately: yes — `target_link_libraries(... PRIVATE DCMTK::dcmdata)`.
- `vna_domain` still has no DCMTK dependency: yes — grep confirms zero
  DCMTK references under `src/domain`, and the domain library's link
  list is unchanged.
- Build succeeds: yes (default).
- Strict warning build succeeds: yes (`-Wall -Wextra -Wpedantic -Werror`).
- A small validation command or test demonstrates:
  - nonexistent file fails: yes (see test output).
  - empty file fails: yes.
  - invalid DICOM file fails: yes.
- No DICOM networking is implemented: yes — only DCMTK's `dcmdata` and
  `ofstd` (transitively) are linked; no `dcmnet`, no socket calls, no
  associations, no SCP/SCU code.

### Notes for future milestones

- The CLI was deliberately not modified. When Milestone 08 wires up the
  C-STORE SCU, it can read a file with `DicomFileReader` and then hand
  the resulting `DicomFile` + `DicomInstanceMetadata` to the SCU layer.
- `DicomFileReader::read` is `const` and stateless, so a single
  instance can be reused (or constructed on demand) without ownership
  concerns.
- If the noisy DCMTK error log on parse failures ever becomes a problem
  for tests or CLI UX, configure `OFLog::configureLogger(LOG4CPLUS_ERROR_LOG_LEVEL_NAME)`
  (or higher) at process startup; this is intentionally not done now to
  keep the scope tight.

### Positive test (2026-05-15)

A second CTest, `dicom_file_reader_positive`, exercises the happy path.

**Fixture generation.** A textual DICOM dump lives at
`tests/unit/fixtures/positive_instance.dump.txt`. At build time
`tests/unit/CMakeLists.txt` looks up the `dump2dcm` CLI with
`find_program(DUMP2DCM_EXECUTABLE dump2dcm)` and adds an
`add_custom_command` that runs `dump2dcm +g <dump> <build>/positive_instance.dcm`.
The `+g` flag tells `dump2dcm` to compute the File Meta Information
Group Length automatically, producing a real Part 10 file with the
128-byte preamble + `DICM` magic + populated file-meta group + dataset.
The custom target `positive_instance_dcm` is added as a build
dependency of the test executable, so CTest never runs against a stale
or missing fixture.

**`dump2dcm` is mandatory.** The positive test is part of the test
suite and must always run. If `find_program` does not locate
`dump2dcm`, `tests/unit/CMakeLists.txt` issues a `message(FATAL_ERROR
...)` and the CMake configure step aborts before any build files are
generated. The error reads:

```
CMake Error at tests/unit/CMakeLists.txt:13 (message):
  dump2dcm (DCMTK CLI tool) is required to generate the positive
  DicomFileReader test fixture, but it was not found on PATH.  Install
  the DCMTK CLI tools (for example, 'brew install dcmtk' on macOS) and
  re-run CMake.
```

This was verified by running `cmake ... -DDUMP2DCM_EXECUTABLE=` to
force the cache variable to empty; CMake printed the error above and
finished with "Configuring incomplete, errors occurred!".
(`find_program(... REQUIRED)` would also work but is a CMake 3.18+
feature, and the project still declares `cmake_minimum_required(VERSION 3.16)`,
so the explicit `FATAL_ERROR` is used to keep the supported range.)

The dump declares:

| tag | value |
|---|---|
| MediaStorageSOPClassUID / SOPClassUID | `1.2.840.10008.5.1.4.1.1.7` (Secondary Capture Image Storage) |
| MediaStorageSOPInstanceUID / SOPInstanceUID | `1.2.3.4.5.6.7.8.9.0` |
| TransferSyntaxUID | `1.2.840.10008.1.2.1` (Explicit VR Little Endian) |
| StudyInstanceUID | `1.2.3.4.5.6.7.8.9.1` |
| SeriesInstanceUID | `1.2.3.4.5.6.7.8.9.2` |
| Modality | `SC` |
| PatientID | `[]` (zero-length value, present as Type 2) |

**Test executable.** `tests/unit/dicom_file_reader_positive.cpp` reads
the fixture path from `argv[1]`, calls `DicomFileReader::read`, and
asserts the nine properties below. It links only `vna_dicom`; it does
not include or link DCMTK directly, so the rule "DCMTK only inside
src/dicom" is preserved on the test side too.

**Assertions and run result.**

```
[pass] DicomFile.path() equals input path
[pass] DicomFile.sizeBytes() is non-zero
[pass] SOP Instance UID matches expected value
[pass] SOP Class UID matches expected value
[pass] Study Instance UID matches expected value
[pass] Series Instance UID matches expected value
[pass] Patient ID is empty (allowed)
[pass] Modality is non-empty
[pass] Modality value matches expected
All positive checks passed.
```

`ctest --test-dir build --output-on-failure` now reports:

```
1/2 Test #1: dicom_file_reader_validation .....   Passed    0.02 sec
2/2 Test #2: dicom_file_reader_positive .......   Passed    0.55 sec
100% tests passed, 0 tests failed out of 2
```

**Isolation re-checked.** `grep -RIn -E "(dcmtk|DCMTK|<dcmtk/)" tests/unit/*.cpp`
returns no matches; the test executable observes the reader purely
through its DCMTK-free public header. `src/domain` is still DCMTK-free.