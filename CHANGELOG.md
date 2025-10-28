# Changelog

All notable changes to this project will be documented in this file.

## Unreleased
- Added an asset prototype layer with typed wrappers (binary, image, shader, sound, font, text) that lazily resolve database resources, cache parsed payloads on the backing resource, and expose convenient accessors.
- Introduced a changelog and refreshed documentation guidance (`AGENTS.md`, `CODING_STYLE.md`) to keep contributions recorded.
- Hardened test helpers to create parent directories and open files with explicit modes, fixing intermittent CI failures in the filesystem database suite.
- Extended git submodule tracking to cover future asset-processing dependencies (`libjpeg-turbo`, `stb`, `libspng`).
- Tweaked the definition registry ingestion flow to avoid redundant `set_source` calls and clarified loader tests while the loader remains WIP.
- Reworked `example/data/houses.yml` to use the new keyed house definition format and expanded the catalog with additional variants.

## v0.1.0 - Database & Resource Foundations
- Established the static library build with CMake, wiring in the external `MTL` framework.
- Implemented the abstract database interface, the filesystem database backend, and the resource handle reference-counting mechanics.
- Added the definition registry with YAML ingestion, duplicate detection, and factory lookup.
- Seeded the initial unit tests for resource lifetime, filesystem traversal, and loader scaffolding.
- Bootstrapped project documentation and contributor guidance to direct early development.
