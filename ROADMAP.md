# Roadmap

Where this is going: a part drawn in FreeCAD, cut by a toolpath generated there, judged by kin5ax
against a real five-axis machine, and posted as a program that machine will run. Five strands, of
which two are finished, one is half built and two have not started.

Updated 2026-10-13.

| | strand | state |
|---|---|---|
| 1 | The kinematics: solver, machine models, collisions | **done**, and in a C ABI |
| 2 | A five-axis toolpath generator in FreeCAD | **done** for a single face |
| 3 | The postprocessor: TCP, direct axes, TWP, Fanuc and Heidenhain | **a third of it**: vector output with TCP |
| 4 | Simulating the machine from a `.kc.json` model | not started |
| 5 | Simulating the cutting | library has it, FreeCAD side not started |

## What is done

**The library** (`kin5ax/`) solves the inverse kinematics of a five-axis chain analytically, reads a
machine from a `.kc.json` file, checks joint limits and collisions, interpolates singular sections,
chooses a trajectory start by a stated policy, and carries a per-location report of what it could and
could not solve. CMake, MSVC and GCC, Linux built and tested on every push by a self-hosted runner.

**The C ABI** (`kin5axc/`, `include/kin5ax.h`) is version 1 and has everything the rest of this
roadmap needs:

- `k5_chain_open` — a machine from a `.kc.json` file; `k5_chain_link`, `k5_chain_frames` — the
  joints and the frame of every segment, which is what drawing a machine needs
- `k5_postprocess_file` and **`k5_postprocess_locations`** — a whole toolpath solved, the second one
  from arrays the host holds in memory: `positions[3n]` and `axes[3n]`, which is exactly the shape of
  what the FreeCAD operation produces
- `k5_path_location` — per location: status, where it came from, and `q[5]`, the joint values
- `k5_path_errors`, `k5_path_count_of_status`, `k5_path_last_configuration`
- `k5_chain_default_gcode_axes`, `k5_path_write_gcode` — direct-axes G-code, `G1 X Y Z A C`

**The FreeCAD operation** (`kin5axcam/`) lays a zigzag over the `u,v` space of one face with any
shape of tool, holds the tool either standing up or along the surface normal leaned over by two
angles, and writes the tip of the tool with the axis as `I J K`. A row is one pass. 72 tests.

**The postprocessor** (`kin5axcam/post/fanuc_tcp_kin5ax_post.py`) writes that vector whole and wraps
an operation that has one in `G43.5`/`G49`. It is FreeCAD's Fanuc post with three changes, and it
needs no kinematics: the control does the work.

## Next, in order

### 1. Python can call kin5ax

A `ctypes` binding to `kin5axc` inside the addon: open a chain, make a tool, solve a path held in
memory, read the joint values and the report back. Perhaps two hundred lines and a day, and
**everything below needs it** — the direct-axes post, the machine simulation, any judging of a path
before it is cut. Nothing else in this list unblocks as much.

It also needs the shared library built for Windows and found at run time, and it forces two decisions
that have been open since the ABI was designed (below).

Worth doing as one vertical slice with step 2 rather than on its own: *a path generated in FreeCAD,
solved by kin5ax, written as `X Y Z A C`* is the first thing that proves the whole chain works.

### 2. The postprocessor proper

With the binding, the post can hand its points and vectors to kin5ax and get joint values, which is
what the roadmap item has always meant:

- **direct axes** — `G1 X Y Z A C`, no TCP, no RTCP option needed on the control; the one mode that
  runs on any machine, and the one that needs the kinematics
- **TCP** — what exists today, extended with Heidenhain (`M128`/`FUNCTION TCPM` and `LN` blocks with
  `TX TY TZ`)
- **TWP** — `G68.2`/`G53.1` on Fanuc, `PLANE SPATIAL` on Heidenhain: for paths that lie in one
  tilted plane, which this generator does not make yet but a swarf or a planar operation would
- the solve report surfaced in the report view: how many locations are unsolved, where limits were
  exceeded, where the tool collided

Open: whether to grow the one post with a `--machine` argument, or to write a second one. The first
keeps a single file to maintain; the second keeps the stock-Fanuc copy free of kin5ax.

### 3. The machine on the screen

Build the machine from the `.kc.json` in a FreeCAD document — a part per link, placed by
`k5_chain_frames` — and step it through a solved path. The library already answers every question
this needs; the work is FreeCAD-side: making the shapes (the file carries collision boxes and
cylinders, not meshes), a tree of placements, and something to drive the frame number.

This is also the honest fix for what the stock simulator cannot do: FreeCAD places the tool with a
fixed upright rotation, so a leaning path is drawn as if the tool stood up.

### 4. Cutting simulation

The voxel material and the cutting are in the library already (`kin5ax/Simulation/`), driven by the
MFC application. Bringing them to FreeCAD means new ABI calls — a material handle, a step, a mesh or
a volume out — and is the largest of these. Worth it only after 3 can show the machine moving.

### 5. The library's own list

macOS build; the rollover-aware axis model (#25, #40, #59) so that C359→C1 is not scored as 2°; the
feasibility check inside interpolated singular sections (#21); tolerances by kind (#56); a
`kin5ax-post` command-line tool over the C ABI with a JSON machinability report.

## Decisions these are waiting on

**The unit.** `kin5ax.h` says lengths are in the unit of the chain file, which is metres for every
model shipped, and the files do not say so themselves. FreeCAD is millimetres. Either the file format
grows a unit field and the ABI fixes itself to millimetres, or the binding scales at the boundary and
the host is told to. The binding cannot be written without choosing. (Question 3 of the ABI plan.)

**What the control calls the axes.** `k5_chain_default_gcode_axes` guesses `A` and `C` because
nothing in the chain file says. Either the file carries a letter per link, or the post takes them as
arguments — it already takes arguments, so that is the cheap half. (Question 6.)

**Where the solve happens.** In the postprocessor, so that the path in the document stays
machine-independent and one path can be posted for two machines — or in the operation, so that the
report view can say "this is not reachable" while the user is still editing. The recommendation is
the postprocessor, with the operation gaining a *check against a machine* button later.

## Housekeeping, and one risk

- **Everything lives on `c-abi`**: 89 commits ahead of `fcm-bridge`, which is the default branch, and
  17 of them not pushed anywhere. The C ABI and the whole FreeCAD addon are on that one branch. It
  wants merging, or `c-abi` wants to become the default.
- The tests that need FreeCAD never run in CI — the runner has no FreeCAD. An AppImage and a second
  job would run all 72 instead of the 44 that need no FreeCAD.
- `kin5axc` is built on Linux by CI; the Windows build of it is a local matter so far, and the
  binding will need it where FreeCAD can find it.
