
# Library Features

## Core concepts
- Data source abstraction (through Databases)
- Definition System 
- Extension System
- Asset System

### Data Source Abstraction

The Data Source Abstraction (DSA) provides a unified interface for accessing content from any storage backend.
All data—definitions, assets, and configs—is exposed through databases that represent logical collections of resources.
This system decouples data access from physical storage, enabling interchangeable backends such as filesystem directories, binary packs, or in-memory sources.

### Definition System

The Definition System provides a structured way to represent and manage typed data objects defined outside of code.
Each definition describes a piece of game or application data—such as items, entities, or rules—and is registered under a unique type and identifier.
Definitions are loaded from databases, constructed through reflection, and stored in registries for fast lookup and cross-referencing.
This system allows behavior to be driven by data instead of hard-coded logic.

### Extension System

The Extension System manages modular content and code packages such as mods or expansions.
Each extension defines metadata (name, version, dependencies) and provides both a database for data and an optional shared library for logic.
Extensions are loaded in dependency order and activated through a defined interface that allows registration of assets, definitions, and systems.

### Asset System

The Asset System provides typed, lazily-loaded access to external resources such as images, sounds, shaders, and text files.
Each asset is represented by an `Asset` object bound to a database path and resolved on demand when first used.
This allows assets to exist as lightweight references until explicitly touched, enabling efficient resource management and fast startup times.


# Details

## Databases

- Filesystem Database: A database that is a lightweight wrapper over a filesystem folder, allowing easy editing and development but poor performance.
- Binary Database: A database that abstracts a packed archive of files that is memory mapped into the process.
- Joined Database: A database that joins or mounts sub databases in different paths of the database
- In Memory Database: A temporary or staging database that is only in memory.
- Remote Database: (potentially) A remote / network database that communicates with a foreign server to serve files. Not really needed but included for future ideas.
- Real Archive Database (zip, tar, etc): A database that wraps over known archive formats.

A database can:
    - be loaded
    - be unloaded
    - activated (will be used for Asset definitions)
    - deactivated
    - list the root directory
    - list subdirectories
    - resolve an absolute path inside itself
    - check if an absolute path exists in itself
    - check if a path leads to a file or directory

## Definition System

### Definition Registry
A definition registry holds all known definitions and keeps them stored by type and key.
A definition type may be created by subclassing the "Definition" type or other classes that are subclasses thereof.

There should be in config inheritance of definition instances. 
This would allow for a more flexible and extensible way to modify definitions  

There should be binary serialization for a archive file. When an extension is exported into a packed binary file this can be used to cut down on loading times.

## Extension System

An extension would have these fields:
    - name: such as "base", "extension1". the formal naming convention should be lowercase words, separated by dots (ex: "base.extension1")
    - version
    - dependencies: a list of extension names that are needed for this extension to work
    - incompatible: a list of extension names that are incompatible with this extension
    - optional: a list of extension names that are optional and not required
    - definitions: either a path to a folder in the database or a packed definition archive
    for each platform:
        - entrypoint: an absolute path inside the database that leads to a dll

An extension would provide a single database (either packed or fs) which provides all resources. 
In that there may be dlls, scripts, images etc.



























