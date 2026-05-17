# Testing Strategy

## Current Test Structure

The project currently uses CTest with small C++ test executables.

Current structure:

- `tests/CMakeLists.txt`
- `tests/unit/CMakeLists.txt`
- `tests/unit/dicom_file_reader_validation.cpp`
- `tests/unit/dicom_file_reader_positive.cpp`
- `tests/unit/send_config_validation.cpp`
- `tests/unit/fixtures/positive_instance.dump.txt`
- `tests/integration/` reserved for future integration tests

The root `CMakeLists.txt` calls `enable_testing()` and adds the `tests`
subdirectory.

## Current Tests

Implemented tests:

- `dicom_file_reader_validation`
  - nonexistent file fails with `DicomFileReadError`;
  - empty file fails with `DicomFileReadError`;
  - invalid non-DICOM bytes fail with `DicomFileReadError`.
- `dicom_file_reader_positive`
  - generates a real Part 10 DICOM fixture from a textual dump via
    `dump2dcm +g`;
  - verifies returned `DicomFile` path and non-zero size;
  - verifies SOP Instance UID, SOP Class UID, Study Instance UID,
    Series Instance UID, empty Patient ID, and Modality `SC`.
- `send_config_validation`
  - verifies oversized timeout values are rejected at the CLI parsing layer.

## How To Run Tests

Configure and build:

```bash
cmake -S . -B build
cmake --build build
```

Run tests:

```bash
ctest --test-dir build --output-on-failure
```

Strict build and test:

```bash
cmake -S . -B build-strict -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Werror"
cmake --build build-strict
ctest --test-dir build-strict --output-on-failure
```

The positive DICOM reader test requires `dump2dcm` from DCMTK to be available
on `PATH`. If it is missing, CMake configuration fails intentionally.

## Unit Test Scope

Unit tests should cover behavior that can be exercised without launching
long-running services.

Expected unit test coverage by module:

- `src/domain`
  - value object validation;
  - equality operators;
  - DICOM status categorization;
  - result `succeeded()` behavior.
- `src/config`
  - required arguments;
  - unknown options;
  - missing option values;
  - port range;
  - AE title validation;
  - timeout validation;
  - invalid option combinations such as `--file` with `--echo-only`.
- `src/dicom/DicomFileReader`
  - missing path;
  - directory path;
  - nonexistent file;
  - empty file;
  - invalid DICOM bytes;
  - valid Part 10 fixture;
  - missing required dataset tags;
  - invalid UID values.
- `src/dicom/EchoScu`
  - pure unit testing is limited because the behavior is network-facing;
  - boundary validation such as invalid calling AE and invalid timeout can be
    tested without a live SCP if a test target is added.

## Integration Test Scope

Integration tests should be added when real services exist.

Future integration tests should cover:

- C-ECHO SCU against a local DCMTK `echoscp` or `storescp`;
- C-STORE SCP receiving from DCMTK `storescu`;
- local storage writing and file layout;
- DICOM file reader plus storage plus metadata index;
- C-FIND SCP responses from indexed metadata;
- C-MOVE SCP retrieval to a local destination SCP.

Integration tests may start local processes on test ports, but they must clean
up processes and files reliably.

## Test Data And Fixture Rules

- Keep fixtures small.
- Prefer text-based fixture sources when possible.
- Generate binary DICOM fixtures in the build directory, not in the source
  tree.
- Do not commit generated `.dcm` files unless a milestone explicitly chooses
  that because the fixture cannot be generated reproducibly.
- Do not depend on private patient data.
- Patient ID may be intentionally empty when testing DICOM Type 2 behavior.
- Fixture UIDs must be deterministic and documented in the test.

## What Not To Test With Fake Success

Do not write tests that prove only that a message was printed while the real
DICOM operation did not happen.

Forbidden fake-success patterns:

- treating `std::cout << "success"` as a DICOM success response;
- returning success from C-ECHO without a real DIMSE response;
- accepting C-STORE without writing data or returning a real DICOM status;
- marking C-FIND or C-MOVE complete without pending/final DICOM responses;
- replacing association negotiation with a mock unless the test is explicitly
  scoped to non-network validation.

Protocol behavior should be tested against real DCMTK code paths or a real
local DCMTK peer whenever the milestone is about DICOM networking.

## AI Agent Testing Rules

- Run the relevant build and tests before marking a milestone complete.
- For source changes, prefer a strict warning build before completion notes.
- Include exact commands and high-level results in milestone notes.
- If a test cannot be run because a tool is missing, either make the dependency
  explicit or document the limitation clearly.
- Do not silently skip required tests for core behavior.
