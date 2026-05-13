# JRNZ Agent Guide

This file is for coding agents working in this repository. It explains what the project is, how it is laid out, what currently works, what does not, how to build and test it, and where changes are most likely to matter.

## Project Summary

`jrnz` is a ZX Spectrum 48K emulator with an in-house Z80 core.

The project is intentionally narrow in scope:

- The main target is the ZX Spectrum 48K machine model.
- The CPU core is tested as a standalone Z80 implementation, but many design decisions are still Spectrum-specific.
- SDL is used for video, keyboard input, and the beeper.

The project is no longer at the “barely boots” stage. It can load ROMs and snapshots, run Spectrum software, and has a substantial automated test suite. It is still not a cycle-perfect or hardware-complete emulator.

## Current High-Level Status

### What currently works well

- Z80 documented instruction coverage is strong.
- Undocumented instruction and flag coverage is also strong by unit-test standards.
- `z80doc` and `z80all` have recently been brought to a passing state during development work.
- 48K ROM loading works.
- 48K `SNA` loading works.
- `Z80` snapshot loading works for version 1, 2, and 3 in 48K mode.
- The 48K machine model is now centralized and passed around explicitly rather than being spread across hardcoded literals.
- Bus timing has model-driven floating-bus and RAM-contention support, with direct tests around both.
- Port I/O contention is now modelled for the key 48K ULA-port access patterns, with direct tests around the current behaviour.
- Port `0xFE` EAR input is explicit and can be driven through a generic machine input-line API.
- Basic display, keyboard, and beeper support are present.
- A number of real programs and games are known to run.

### What is still limited

- The emulator is still 48K-oriented rather than a general full-family Spectrum emulator.
- ULA/video timing is still simplified overall.
- Mid-scanline display effects are still not rendered accurately.
- Memory contention is present but should still be treated as an evolving 48K-model feature rather than finished hardware-accuracy work.
- Broader timing realism is still incomplete even though several contention-sensitive cases now work correctly.
- Snapshot support is intentionally limited to 48K-compatible cases.
- Peripheral support is minimal.
- The debugger exists, but it is basic and interactive rather than polished.

## Repository Layout

### Main runtime code

- `src/main.cpp`
  - SDL setup, option parsing, and top-level emulator wiring.
- `src/system.cpp`, `src/system.hpp`
  - Per-tstate coordination between Z80, ULA, bus, debugger, and beeper.
- `src/bus.cpp`, `src/bus.hpp`
  - Memory, port I/O, ROM write protection, snapshot loading entry points, floating bus behavior, machine input lines, opcode fetch logic, contention timing.
- `src/ula.cpp`, `src/ula.hpp`
  - Frame timing, interrupt triggering, SDL rendering, beam-aware border history, frame pacing.
- `src/keyboard.cpp`, `src/keyboard.hpp`
  - Spectrum keyboard matrix mapping onto SDL keyboard state.
- `src/beeper.hpp`
  - Simple beeper implementation using SDL audio.
- `src/debugger.cpp`, `src/debugger.hpp`
  - Interactive debugger and disassembly/memory dump helpers.
- `src/options.cpp`, `src/options.hpp`
  - CLI argument handling.
- `src/machine_config.hpp`
  - Runtime machine-model definition for the 48K Spectrum, including timing, display, and contention constants.

### Z80 core

- `src/z80/z80.cpp`, `src/z80/z80.hpp`
  - CPU state, fetch/decode/execute loop, interrupt handling, `R` register, cycle countdown.
- `src/z80/decoder.cpp`, `src/z80/decoder.hpp`
  - Opcode decode tables and ROM label metadata.
- `src/z80/instructions.cpp`, `src/z80/instructions.hpp`
  - Instruction semantics and shared helpers.
- `src/z80/register.cpp`, `src/z80/register.hpp`
  - Register abstractions, including AF flag handling.
- `src/z80/storage_element.cpp`, `src/z80/storage_element.hpp`
  - Operand abstraction used by instruction execution.

### Snapshot formats

- `src/formats/format_sna.cpp`
- `src/formats/format_z80.cpp`

### Tests

- `tests/`
  - Catch2-based unit and compliance-style tests.

### Utilities and sample assets

- `utils/`
  - Snapshot inspection helpers.
- `tap/`
  - Local TAP files used for manual testing.

## Build and Run Workflow

### Build scripts

The project now uses separate build directories per configuration:

- `./build_debug.sh`
  - Configures and builds `__build/debug` with `CMAKE_BUILD_TYPE=Debug`
- `./build_release.sh`
  - Configures and builds `__build/release` with `CMAKE_BUILD_TYPE=Release`
- `./build_relwithdebinfo.sh`
  - Configures and builds `__build/relwithdebinfo` with `CMAKE_BUILD_TYPE=RelWithDebInfo`

### Run scripts

- `./run_jrnz.sh`
  - Defaults to `release`
  - Can also be called as:
    - `./run_jrnz.sh debug ...`
    - `./run_jrnz.sh release ...`
    - `./run_jrnz.sh relwithdebinfo ...`
- `./run_tests.sh`
  - Defaults to `debug`
  - Can also be called as:
    - `./run_tests.sh debug`
    - `./run_tests.sh release`
    - `./run_tests.sh relwithdebinfo`

### Clean script

- `./clean.sh`
  - Cleans all configured build directories.

### CMake and dependencies

- C++20 is required.
- SDL2 is required.
- Catch2 can either be:
  - fetched via `FetchContent`, or
  - found from a local install with `-DUSE_VENDORED_CATCH2=OFF`

Note:

- `FetchContent` uses a GitHub HTTPS URL.
- If builds fail while cloning Catch2 and errors mention `ssh.github.com`, check global Git URL rewrite rules. A global rewrite from `https://github.com/...` to SSH can break dependency fetching.

## Runtime Capabilities

### Machine model

Current machine assumptions are centered on the 48K Spectrum:

- 64K address space
- ROM at low memory, RAM above ROM
- ULA port handling centered on `0xFE`
- 50 Hz frame cadence
- model-driven contention fields and display timing constants
- 48K memory layout assumptions in snapshots and display code

### CPU support

The CPU core supports:

- documented Z80 instructions
- many undocumented opcodes and aliases
- indexed instructions with `IX`/`IY`
- block instructions
- interrupt modes `IM 0`, `IM 1`, `IM 2`
- `R` register behavior
- undocumented flag behavior across several tricky families
- `MEMPTR`-related behavior in at least the currently tested paths

The instruction implementation has been exercised heavily by:

- unit tests
- compliance-style matrix tests
- `z80doc`
- `z80all`

### Display

Current display support includes:

- SDL window creation
- 256x192 bitmap rendering with explicit horizontal and vertical viewport tuning around the border area
- attributes, bright, and flash
- border color from ULA port state
- border rendering that now records colour history across the visible frame and can show at least coarse mid-frame border changes
- scanline-based bitmap/attribute snapshots, which are enough for some raster-sensitive titles that update display memory during the frame
- frame pacing to approximately 50 Hz unless `--fast` is used

Important limitation:

- Rendering is no longer a pure end-of-frame snapshot.
- Border colour is tracked across the visible frame and bitmap/attribute data is snapshotted per scanline, which improves many mid-frame effects.
- The current draw routine still does not render mid-frame or mid-scanline bitmap/attribute changes correctly.
- This means color bars, border effects, and timing-sensitive raster tricks are not faithfully represented.

## Timing Notes That Matter

- The system publishes the current ULA frame t-state to the bus before advancing the ULA for the next tick. Moving that ordering can shift contention and interrupt phase by one t-state.
- Z80 instruction scheduling assumes the current `clock()` call already consumes the first T-state of the decoded instruction. Future cycle-accounting changes need to preserve that convention.
- `HALT` should wake as soon as an interrupt becomes visible, even if the core is part-way through the synthetic halt wait. Delaying that wake can cause stable two-position frame jitter in timing-sensitive software.
- Horizontal border placement follows the 48K line order in `src/machine_config.hpp`: display, right border, blanking, then left border. The visible left and right borders are asymmetric, and the blanking period is not rendered.
- Vertical border placement is calibrated with `vertical_blank_top_lines` in `src/machine_config.hpp`. Screen scanline snapshots are anchored separately to the active display area, so avoid re-coupling those two concerns.

### Keyboard

Keyboard input is mapped through the Spectrum half-row matrix using SDL keyboard state.

Implemented keys include the standard matrix rows:

- Shift/Z/X/C/V
- A/S/D/F/G
- Q/W/E/R/T
- 1/2/3/4/5
- 6/7/8/9/0
- Y/U/I/O/P
- H/J/K/L/Enter
- B/N/M/Symbol Shift/Space

### Audio

The beeper exists and is audible, but it is simple:

- SDL audio device
- mono sample stream
- simple accumulation based on EAR/MIC state
- adjustable software gain

It should be treated as functional rather than highly accurate.

### Port input lines

The bus now exposes a small machine-input-line API:

- `MachineInputLine::Ear`
- `Bus::set_input_line(...)`
- `Bus::input_line_active(...)`

`Bus::set_ear_input(...)` still exists as a compatibility wrapper, but new code should prefer the generic input-line API when possible.

### Snapshots

Implemented:

- `.sna` 48K loading
- `.z80` version 1
- `.z80` version 2 and 3 in 48K mode

Not implemented or intentionally rejected:

- 128K machine modes
- Interface 1 pages
- Multiface pages
- broader hardware-mode features present in `.z80` metadata

## Current Technical Limitations

This section is intentionally blunt. Agents should assume these are real constraints until the code proves otherwise.

### Timing and ULA realism

- Rendering is not beam-accurate.
- Mid-frame display changes are only partially represented: border colour history is tracked across the visible frame, and bitmap/attribute data is snapshotted per scanline rather than per pixel or per beam position.
- RAM contention exists and is tested, but should not yet be treated as fully hardware-accurate 48K behavior.
- Port contention is incomplete.
- Some real software may run but still show timing artifacts, especially sprite flicker or border-effect inaccuracies.
- Passing CPU-focused diagnostics such as `z80doc` and `z80all` does not imply accurate machine-level video timing.
- A game can have correct CPU semantics and still render poorly because the ULA/display model is too coarse.
- Timing-sensitive bugs may show up as:
  - sprite flicker
  - unstable border effects
  - missing mid-scanline color changes
  - effects that look correct in memory but wrong on screen

### Spectrum hardware scope

- The emulator is fundamentally 48K-centric.
- 128K paging and AY sound are not supported.
- Tape loading is not implemented as a full tape subsystem, though the EAR input path now exists at the bus level.
- Peripheral support is minimal.

### Debugger ergonomics

- The debugger is functional but old-style.
- It is CLI-driven and blocking.
- It is useful for stepping and inspection, but not pleasant enough to treat as a polished debugger UI.

### Architecture tradeoffs

- The CPU execution model is correctness-first rather than speed-first.
- `StorageElement` is elegant but not cheap.
- Decode and execution flow are still abstraction-heavy in hot paths.
- If performance work starts, likely hot spots are:
  - `decode_opcode()`
  - `Instruction::execute()`
  - `StorageElement`
  - memory access/timing integration

## Testing Overview

The repository currently has a strong automated test base spanning the documented core, undocumented behavior, machine plumbing, and compliance-style matrices.

The suite is intentionally split between:

- narrative unit tests
- compliance-style matrix tests
- documented behavior
- undocumented behavior

### Test executables

- `run_tests`
  - documented instructions, core execution, bus, snapshots, interrupts, indexed forms, compliance matrices
- `run_tests_undocumented`
  - undocumented opcode families and prefix behavior
- `run_tests_undocumented_flags`
  - undocumented flag behavior, `MEMPTR`, and CCF/SCF latch-sensitive cases

### Core testing categories

#### Decoder and fetch

- `tests/test_decoder.cpp`
  - base decode coverage
  - CB/ED table completeness
  - indexed-prefix handling
  - opcode fetch semantics
  - ignored-prefix behavior

#### Arithmetic and flags

- `tests/test_adc.cpp`
- `tests/test_sbc.cpp`
- `tests/test_neg.cpp`
- `tests/test_flags_misc.cpp`
- `tests/test_flags_matrix.cpp`
- `tests/test_execution_arithmetic16.cpp`
- `tests/test_execution_alu.cpp`
- `tests/test_execution_alu_source_matrix.cpp`
- `tests/test_execution_alu_immediate_matrix.cpp`

Coverage includes:

- 8-bit and 16-bit add/subtract families
- carry/half-carry/overflow corner cases
- full-flag byte checking
- `CP`, `DAA`, `NEG`, `INC`, `DEC`, `BIT`
- source-mode and immediate-mode matrices

#### Control flow and interrupts

- `tests/test_control_flow.cpp`
- `tests/test_execution_control.cpp`
- `tests/test_execution_control_matrix.cpp`
- `tests/test_interrupt_matrix.cpp`

Coverage includes:

- `CALL`, `RET`, `RST`, `JR`, `DJNZ`
- `EI`, `DI`
- `RETI`, `RETN`
- `IM 0/1/2`
- `HALT`
- NMI and maskable interrupts
- interrupt sequencing and edge cases

#### Loads, exchange, stack, and indexed register families

- `tests/test_execution_loads.cpp`
- `tests/test_execution_exchange_stack.cpp`
- `tests/test_execution_index_halves.cpp`
- `tests/test_execution_index_halves_matrix.cpp`
- `tests/test_execution_index_load_matrix.cpp`
- `tests/test_execution_index_pairs_misc.cpp`
- `tests/test_execution_16bit_indexed_matrix.cpp`
- `tests/test_execution_family_matrix.cpp`

Coverage includes:

- documented load families
- extended and indexed load forms
- IX/IY half-register behavior
- stack push/pop
- `EX` and `EX (SP),rr`
- broader matrix coverage for register-family consistency

#### Bit, rotate, shift, nibble, and indexed-CB behavior

- `tests/test_execution_bitsetres.cpp`
- `tests/test_execution_bit_matrix.cpp`
- `tests/test_execution_rotate_shift.cpp`
- `tests/test_execution_nibbles.cpp`
- `tests/test_execution_indexed_cb_matrix.cpp`

Coverage includes:

- `BIT`, `SET`, `RES`
- CB rotate and shift families
- accumulator rotate variants
- `RLD` and `RRD`
- indexed CB register-copy and memory-target behavior

#### Block instructions and ports

- `tests/test_execution_extended.cpp`
- `tests/test_execution_blocks_ports.cpp`
- `tests/test_execution_block_io.cpp`
- `tests/test_execution_ports_matrix.cpp`

Coverage includes:

- `LDI`, `LDD`, `LDIR`, `LDDR`
- `CPI`, `CPD`, `CPIR`, `CPDR`
- `INI`, `IND`, `INIR`, `INDR`
- `OUTI`, `OUTD`, `OTIR`, `OTDR`
- repeat vs terminal timing
- self-modifying `ED` repeat regressions
- documented port read/write behavior

#### Bus and machine plumbing

- `tests/test_bus.cpp`
- `tests/test_snapshot_sna.cpp`
- `tests/test_snapshot_z80.cpp`

Coverage includes:

- ROM write protection
- RAM visibility
- word writes across the ROM/RAM boundary
- keyboard vs floating bus reads
- machine input-line / EAR-bit behavior on `0xFE`
- floating bus wraparound
- floating bus beam-phase behavior
- contention timing and delay-pattern coverage
- `0xFE` port write latching
- snapshot header restoration

#### Undocumented opcodes and undocumented flags

- `tests/test_undocumented_opcodes.cpp`
- `tests/test_undocumented_matrix.cpp`
- `tests/test_undocumented_flags.cpp`
- `tests/test_undocumented_flags_matrix.cpp`

Coverage includes:

- `SLL`
- undocumented `NEG`, `RETN`, `IM` aliases
- `OUT (C),0`
- ignored prefix combinations
- indexed CB destination-copy forms
- undocumented `F3/F5`
- `CP` operand-bit behavior
- `BIT` source and `MEMPTR` behavior
- `LD A,I`, `LD A,R`
- `SCF`, `CCF`, `CPL`
- rotate/shift undocumented flags
- block compare, block transfer, and block I/O undocumented flags
- curated `z80ccf`-style regressions

### Testing philosophy

The suite now mixes two styles on purpose:

- narrative tests
  - easier to read
  - useful for behavior explanation
- compliance-style matrix tests
  - dense
  - useful for exhaustive or near-exhaustive flag and decode validation

When adding new tests, prefer:

- narrative tests for one-off regressions or behavior stories
- matrix tests for flag-heavy or family-wide instruction rules

### External diagnostic context

Recent work used external diagnostics to drive regressions, especially:

- `z80doc`
- `z80all`

Good practice in this repo is:

- when an external diagnostic finds a bug, reduce it to a small unit test
- do not import the external program wholesale into the unit suite
- keep new regressions local, deterministic, and named for the actual bug

## Known Important Behavior

### Prefix handling

This codebase explicitly cares about:

- repeated `DD`/`FD`
- ignored `DD`/`FD` before `ED`
- indexed `CB` forms
- consumed bytes vs effective opcode selection

This area is well tested and easy to regress. Touch it carefully.

### Undocumented flags

This emulator intentionally models a significant amount of undocumented behavior:

- `F3/F5` result bits
- `CP` operand-bit behavior
- `BIT` special cases
- block instruction undocumented flags
- `SCF`/`CCF` previous-instruction-latch behavior
- `MEMPTR`-visible effects through later probes

Do not assume undocumented flags are “don’t care” in this repo. They are part of the tested behavior.

### Undefined ED opcodes

Undefined `ED` opcodes are treated as 2-byte NOP-like instructions, which matters for self-modifying block-repeat tests and external compliance.

### `R` register behavior

The `R` register behavior is explicitly tested:

- prefix fetch length matters
- ignored prefixes matter
- interrupts matter

Avoid “simplifying” fetch accounting without checking those tests.

## Where Future Work Is Likely Needed

These are the areas most likely to matter next.

### Video and timing

- proper ULA contention
- more accurate port contention
- scanline-progress or beam-based rendering
- mid-scanline border and attribute changes beyond the current coarse border-history pass
- better frame presentation/vsync handling

### Performance

- decode path cost
- `StorageElement` overhead
- instruction dispatch overhead
- timing hooks that do not distort hot-path performance too much

### Hardware completeness

- broader snapshot compatibility
- tape support beyond local/manual workflows
- 128K-era machine features if the project scope ever widens

## Practical Advice For Agents

### Before editing

- Check `git status`.
- The user sometimes keeps local ROMs, TAPs, and scratch files in the repo root.
- Do not remove untracked assets unless explicitly asked.

### When changing CPU semantics

- Run the relevant focused tests first.
- Then run the full test suite if the change touches shared helpers.
- If a bug comes from a compliance program, add a distilled regression test.

### When changing timing or rendering

- Expect game-visible differences that unit tests may not catch.
- Manual testing with real software is important.
- Do not assume a visual bug is a CPU-core bug just because the instruction tests are green.
- If a game shows flicker or raster issues, consider at least these causes before changing CPU semantics:
  - missing or inaccurate RAM contention
  - missing port contention
  - frame-snapshot rendering
  - missing beam-position-aware border or attribute updates
- Changes in:
  - `src/system.cpp`
  - `src/ula.cpp`
  - `src/bus.cpp`
  - `src/z80/z80.cpp`
  often interact in non-obvious ways.

### When changing snapshots

- Be careful not to widen support claims without tests.
- Current `.z80` support is intentionally 48K-only even though v2/v3 headers are handled.

### When changing tests

- Keep the documented/undocumented split meaningful.
- Keep matrix-style tests in dedicated files where possible.
- Avoid turning every test into a huge matrix if a narrative regression is clearer.

## Useful Commands

### Build

```bash
./build_debug.sh
./build_release.sh
./build_relwithdebinfo.sh
```

### Run tests

```bash
./run_tests.sh
./run_tests.sh debug
./run_tests.sh release
./run_tests.sh relwithdebinfo
```

### Run emulator

```bash
./run_jrnz.sh --rom 48.rom
./run_jrnz.sh release --rom 48.rom --z80 some_snapshot.z80
./run_jrnz.sh debug --rom 48.rom --sna some_snapshot.sna
```

### Debugger-oriented run

```bash
./run_jrnz.sh debug --rom 48.rom --debug --break 0x1234
```

## Final Notes

This repo is no longer a minimal emulator toy. It has real CPU coverage and real behavioral expectations, especially around undocumented Z80 details. At the same time, the machine-level timing and rendering model still has obvious gaps. Treat CPU correctness and video/timing realism as separate axes: the core is strong, the machine model is still evolving.
