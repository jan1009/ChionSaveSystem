# ChionSaveSystem

A lightweight, Blueprint-friendly save system for Unreal Engine 5 that automatically serializes variables marked with Unreal Engine's `SaveGame` flag.

ChionSaveSystem is designed for Blueprint projects that need persistent Actor state without writing custom save/load logic for every variable.

## Features

- Automatic serialization of Blueprint variables marked with `SaveGame`
- Persistent GUIDs for placed level Actors
- Optional `SaveKey` support for stable logical Actors such as the Player
- Optional Actor Transform save and restore
- Persistent destroyed Actor state
- Restores destroyed Actors when loading an older save during the same game session
- Blueprint events before saving and after loading
- Duplicate `SaveKey` detection
- Blueprint-only game projects supported
- No project-side C++ code required

## Compatibility

Currently tested with Unreal Engine 5.8 on Windows 64-bit, including Blueprint-only projects, source builds, and Epic Games Launcher builds.

## Installation

Copy the `ChionSaveSystem` folder into your project's `Plugins` directory:

```text
YourProject/
└── Plugins/
    └── ChionSaveSystem/
```

Restart Unreal Engine and enable **ChionSaveSystem** if necessary.

A precompiled Windows build for the Epic Games Launcher version of Unreal Engine 5.8 is provided separately in GitHub Releases.

## Basic Usage

1. Add a **Chion Save Component** to an Actor.
2. Enable the **SaveGame** flag on Blueprint variables that should persist.
3. Call **Save World** with a Slot Name and User Index.
4. Call **Load World** with the same Slot Name and User Index.

Variables without the `SaveGame` flag are ignored.

## Save Keys

Placed level Actors normally use automatically generated persistent GUIDs.

For stable logical Actors such as the Player, set a unique `SaveKey`, for example:

```text
Player
```

Duplicate Save Keys are detected and are not saved or loaded.

## Actor Transform

Enable **Save Actor Transform** on the Chion Save Component if position, rotation, and scale should also be persisted.

## Destroyed Actors

GUID-based level Actors destroyed during gameplay are tracked by the save system.

If the game is saved after an Actor was destroyed, loading that save removes the Actor again.

If an older save is loaded while that Actor is currently destroyed, ChionSaveSystem recreates it from the saved Actor class and spawn transform, restores its original Save ID, and then loads its saved state.

## Blueprint Events

The Chion Save Component exposes:

- **On Save State Saving** — called immediately before serialization.
- **On Save State Loaded** — called after the saved state has been restored.

## Current Scope

This release focuses on placed level Actors using persistent GUIDs and stable logical Actors using Save Keys.

General-purpose persistence for arbitrary runtime-spawned Actors, such as dropped inventory items that must survive across game restarts, is not part of this first release.

## License

ChionSaveSystem is released under the MIT License.
