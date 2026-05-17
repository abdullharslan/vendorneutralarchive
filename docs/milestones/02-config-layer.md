# Milestone 02: Configuration Layer

## Goal

Implement configuration parsing and validation for the application entrypoints without adding DICOM networking.

The configuration layer must turn CLI arguments into strongly typed configuration objects used by future milestones.

## Scope

Implement under `src/config`:

- `SendConfig`
- `ReceiverConfig`
- `ConfigError`
- CLI parsing for `vna-send`
- CLI parsing for `vna-receiver`

## Required CLI

`vna-send` must support:

- `--file <path>`
- `--host <host>`
- `--port <port>`
- `--called-ae <ae-title>`
- `--calling-ae <ae-title>`
- `--timeout <seconds>`
- `--echo-only`
- `--help`

`vna-receiver` must support:

- `--host <host>`
- `--port <port>`
- `--ae-title <ae-title>`
- `--storage-path <path>`
- `--help`

## Rules

- Use English only.
- Do not add DCMTK.
- Do not implement DICOM networking.
- Do not implement storage logic.
- Do not implement database/indexing.
- Do not parse CLI arguments inside DICOM classes.
- Do not create fake DICOM success responses.

## Validation

- AE titles must reuse or align with `vna::domain::DicomNode` validation.
- Ports must be in range `1..65535`.
- Timeout must be positive.
- Required arguments must produce clear English errors.
- `--help` should print usage and exit successfully.

## Acceptance Criteria

- `vna-send --help` prints usage.
- `vna-receiver --help` prints usage.
- Missing required arguments fail with a clear error.
- Invalid port fails with a clear error.
- Invalid AE title fails with a clear error.
- Existing domain models remain independent from CLI parsing.
- No DICOM networking is implemented.

## Completion Notes

Implemented on 2026-05-15.

### What was created

**Domain refactor** (precondition for "AE titles must reuse domain
validation"):

- `src/domain/AeTitle.h` / `.cpp` — public `isValidAeTitle()` and
  `requireValidAeTitle(title, context)` extracted from the previously
  private validator in `DicomNode.cpp`. The `context` parameter
  preserves the exact wording of `DicomNode`'s existing error messages
  while letting the config layer prepend option names (e.g.
  `--called-ae must be at most 16 characters`).
- `src/domain/DicomNode.cpp` now delegates to `requireValidAeTitle` with
  context `"DicomNode AE title"`. No external behavior change.

**Config layer** under `src/config`, namespace `vna::config`:

- `ConfigError` — `std::runtime_error` subclass for parsing/validation
  failures, with a plain English message.
- `SendConfig` — plain struct holding the parsed `vna-send` settings:
  `std::optional<std::string> filePath`, `host`, `port`, `calledAe`,
  `callingAe`, `timeoutSeconds` (default 30), `echoOnly`.
- `ReceiverConfig` — plain struct holding the parsed `vna-receiver`
  settings: `host`, `port`, `aeTitle`, `storagePath`.
- `parseSendCli` / `sendUsage` — CLI parser and help text for
  `vna-send`. Supports every flag listed in the milestone:
  `--file --host --port --called-ae --calling-ae --timeout --echo-only --help`.
- `parseReceiverCli` / `receiverUsage` — CLI parser and help text for
  `vna-receiver`. Supports:
  `--host --port --ae-title --storage-path --help`.
- `internal::takeValue`, `internal::parsePort`, `internal::parsePositiveUint32`
  — shared CLI helpers in `CliParsing.h/.cpp`. Numeric parsing uses
  `std::from_chars` and rejects partial / non-numeric input.

The parsers throw `ConfigError` with a clear English message on any of:
unknown option, missing value, missing required argument, invalid port,
invalid AE title (delegated to the domain validator), non-positive
timeout, empty host / file / storage path, or `--file` + `--echo-only`
combined. `--help` short-circuits parsing and returns a result flagged
`helpRequested = true`.

**App updates**

- `apps/vna-send/main.cpp` and `apps/vna-receiver/main.cpp` now call the
  parsers, print the parsed configuration, and on `ConfigError` print the
  message to stderr and exit with code 2. They still finish with the
  line "DICOM networking is not implemented yet." — no fake DICOM
  responses are produced.
- Both apps now link `vna_config` (which transitively brings
  `vna_domain`).
- `--help` prints usage and exits 0.

**Build wiring**

- New `src/config/CMakeLists.txt` declares static library `vna_config`,
  exports `src/` as its `BUILD_INTERFACE` include directory, and links
  `vna_domain` publicly so consumers also pick up the domain headers.
- `src/CMakeLists.txt` now does `add_subdirectory(config)` after
  `add_subdirectory(domain)`.

### Verification

- `cmake --build build` from a clean cache produces `libvna_domain.a`,
  `libvna_config.a`, and both binaries.
- A separate strict build with `-Wall -Wextra -Wpedantic -Werror` is
  also warning-free.
- `grep -RIn -E "(dcmtk|DCMTK|dcm[a-z]*\.h)" src/ apps/` returns no
  matches.
- Help and validation paths exercised:

  | scenario | expected | exit |
  |---|---|---|
  | `vna-send --help` | prints usage | 0 |
  | `vna-receiver --help` | prints usage | 0 |
  | `vna-send --host h --port 11112 --called-ae R --calling-ae L --file f` | echoes parsed config | 0 |
  | `vna-send ... --echo-only --timeout 10` (no `--file`) | echoes config, `echo-only:true` | 0 |
  | `vna-send` missing `--called-ae` | `--called-ae is required` | 2 |
  | `vna-send` without `--file` and without `--echo-only` | `--file is required unless --echo-only is set` | 2 |
  | `vna-send` with both `--file` and `--echo-only` | `--file must not be combined with --echo-only` | 2 |
  | `vna-send --port 70000` | `--port expects an integer in the range 1-65535, got "70000"` | 2 |
  | `vna-send --port abc` | same shape, `got "abc"` | 2 |
  | `vna-send --called-ae THIS_AE_IS_WAY_TOO_LONG` | `--called-ae must be at most 16 characters` | 2 |
  | `vna-send --called-ae 'BAD\AE'` | `--called-ae must not contain backslash` | 2 |
  | `vna-send --timeout 0` | `--timeout expects a positive integer, got "0"` | 2 |
  | `vna-send --bogus foo` | `unknown option: --bogus` | 2 |
  | `vna-receiver` missing `--storage-path` | `--storage-path is required` | 2 |
  | `vna-receiver --ae-title` (no value) | `--ae-title requires a value` | 2 |

### Acceptance criteria check

- `vna-send --help` prints usage: yes.
- `vna-receiver --help` prints usage: yes.
- Missing required arguments fail with a clear error: yes (see table).
- Invalid port fails with a clear error: yes — both out-of-range and
  non-numeric values are rejected by `parsePort`.
- Invalid AE title fails with a clear error: yes — validation delegates
  to `vna::domain::requireValidAeTitle`, so config errors match the
  same rules the domain enforces.
- Existing domain models remain independent from CLI parsing: yes — the
  domain layer has no `iostream`, no `argv` handling, no dependency on
  `vna_config`. The dependency direction is `config → domain`.
- No DICOM networking is implemented: yes — no DCMTK headers, no
  sockets, no association code. The apps explicitly print "DICOM
  networking is not implemented yet."

### Notes for future milestones

- `SendConfig::filePath` is an `optional<std::string>`. Once C-ECHO
  (Milestone 04) and C-STORE SCU (Milestone 08) land, the SCU code can
  switch on `cfg.echoOnly` and on the presence of `filePath`.
- The CLI parser does not currently accept `--option=value`
  (space-separated only) or short flags. Both can be added later if
  there is demand; the milestone only required the listed long flags.