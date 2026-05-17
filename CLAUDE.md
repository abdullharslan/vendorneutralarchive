# Claude Instructions

We are building a clean C++17 DICOM archive node using DCMTK.

The product goal is to implement core DICOM archive workflows similar to DCM4CHEE:
receive, store, index, query, and retrieve DICOM studies over standard DICOM networking.

## Language Rules

All code, comments, logs, file names, class names, function names, documentation, commit messages, and test names must be written in English.

## Engineering Rules

- Follow SOLID principles.
- Keep DICOM networking separate from storage, indexing, configuration, and application entrypoints.
- Do not implement fake success responses.
- Do not log instead of sending real DICOM protocol responses.
- Do not add UI, Redis, Celery, MinIO, PostgreSQL, DICOMweb, HL7, C-FIND, C-MOVE, or Storage Commitment until their milestone explicitly asks for them.
- Prefer small, testable milestones.
- Do not skip acceptance criteria.
- Do not introduce unrelated refactors.

## Architecture Layers

- apps: CLI and service entrypoints
- src/domain: pure domain models, no DCMTK dependency
- src/application: use cases and orchestration
- src/dicom: DCMTK-based DICOM networking and file parsing
- src/storage: storage abstractions and implementations
- src/index: metadata indexing abstractions and implementations
- src/config: CLI and environment configuration
- src/logging: logging helpers
- tests: unit and integration tests
- docs: architecture, roadmap, decisions, and milestone specs

## Current Working Method

Before implementing any milestone:
1. Read docs/PROJECT_BRIEF.md
2. Read docs/ARCHITECTURE.md
3. Read docs/ROADMAP.md
4. Read the active milestone file under docs/milestones
5. Implement only that milestone
6. Run relevant tests
7. Update the milestone file with completion notes