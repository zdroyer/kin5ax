# Roadmap

Where this is going: a part drawn in FreeCAD, cut by a toolpath generated there, judged by kin5ax
against a real five-axis machine, and posted as a program that machine will run. Five strands, of
which four are finished and one is two thirds built.

Updated 2026-10-14.

| | strand | state |
|---|---|---|
| 1 | The kinematics: solver, machine models, collisions | **done**, and in a C ABI |
| 2 | A five-axis toolpath generator in FreeCAD | **done** for a single face |
| 3 | The postprocessor: TCP, direct axes, TWP, Fanuc and Heidenhain | **two thirds**: Fanuc with TCP, and direct axes from the solver |
| 4 | Simulating the machine from a `.kc.json` model | **done**: the machine in the document, a player over the solved job |
| 5 | Simulating the cutting | **done** as voxels and points: the stock cut step by step, gouges and volume, on a machine or on the path alone |

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
angles, and writes the tip of the tool with the axis as `I J K`. A row is one pass. 149 tests, of
which 72 need no FreeCAD.

**Python can call kin5ax** (`kin5axcam/kin5axcam/kin5ax.py`): a `ctypes` binding to the C ABI — a
chain, a tool, the settings, a path solved in memory, its locations and report, the frames of the
machine. Checked against the forward kinematics of the library.

**The postprocessor** (`kin5axcam/post/fanuc_tcp_kin5ax_post.py`) writes that vector whole and wraps
an operation that has one in `G43.5`/`G49` — FreeCAD's Fanuc post with three changes, no kinematics,
the control does the work. With `--machine <chain file>` it hands every move to kin5ax instead and
writes the five axes: direct output, the one mode that runs without a TCP option. A location the
machine cannot reach is left out and said so.

## Next, in order

### 1. Done: Python can call kin5ax, and the post writes direct axes

The binding and the `--machine` mode of the post, 2026-10-13. The decisions were taken the cheap
way: the post scales by `--machine-unit` (metres by default), and the rotary letters come from the
names of the links when they are letters, else from an argument. The binding finds the shared
library through `KIN5AXC_LIBRARY`, `kin5axcam/lib/` or the build tree of a checkout; shipping a
built `kin5axc.dll` in `lib/` is still to do.

### 2. The rest of the postprocessor

- **Heidenhain**: TCP with `M128`/`FUNCTION TCPM` and `LN` blocks carrying `TX TY TZ`, and direct
  axes from the solver in its syntax
- **TWP** — `G68.2`/`G53.1` on Fanuc, `PLANE SPATIAL` on Heidenhain: for paths that lie in one
  tilted plane, which this generator does not make yet but a swarf or a planar operation would
- the rollover of a free rotary axis is handled in the post (the nearest turn is written); the
  library's own rollover model (#25) would let the trajectory be scored that way too
- a report of the solve in the task panel of the operation, not only in the report view at post time

### 3. Done: the machine on the screen

`kin5axcam/machine.py`, `simulation.py`, `machine_gui.py`, 2026-10-13. A part per segment with the
boxes of the file and an arm to the next, the tool mount and the material as parts, the job's tool
and model and stock in them, the trajectory in properties, a player with slider, play, step and the
report. Two calls were added to the ABI for it: `k5_chain_segment_boxes` and
`k5_chain_set_configuration`. 2026-10-14: which solids met is reported as well - a
`CCollisionReport` of named pairs with counts in the library, filled by `CChain::DetectCollisions`,
carried out of a solve by `CSolveReport` and aggregated per location into the result, through the
ABI as `k5_path_collision`, and shown in the dialog under *what met*. The same day: the colliding
solids **shown** - a location that collides poses the machine where it would have had to be
(`k5_chain_check_collisions` on a configuration solved with the check off) and paints the two solids
red, named through `k5_chain_solid_name` so the host matches what the library reports. A solid that
has met anything stays orange to the end of the run; the tool is red while it meets something and
never orange, being the thing that moves rather than a part that was in the way. The player can be
made to pause where the machine first meets itself, and a run can be reset to a whole stock at the
first location. Still to do here: real bodies per segment (a segment part takes them by hand
already; the file format does not reference them).

### 4. Done: cutting simulation, as voxels

Twelve calls in the ABI (`k5_material_*`, `k5_simulation_*`), the binding, and `Cut` on the
simulation object, 2026-10-13: the stock of the job as a block of voxels, cut step by step with the
tool of each operation, the surface under the material part, the removed volume
and the gouges as properties and a marker. The interpolation is made as dense as the voxels when
cutting, which is what the once-per-location stamp of the library needs.

2026-10-13, later the same day: the cut **without a machine** — `CToolPathSimulation` in the
library, the path made dense by the solver's own interpolation (`LocationInterpolation.h`, moved
out of the solver so both share it), one ABI constructor from positions and axes, and `Machine`
optional on the simulation object. The tool is drawn leaning in the frame of the part straight
from the path. Nothing of the densification is in Python.

2026-10-14: the surface as a **mesh of the exposed voxel faces**, worked out in the library
(`CMaterial::GetSurfaceTriangles`, `k5_material_surface_triangles`), in place of the point cloud;
the joints of the machine shown as the application draws them; a link carrying the placement of the
stock, and one simulation per job, which were the two faults of the first picture. The stock is cut
at a location the machine cannot reach as well (`k5_material_cut`, one stamp of the tool wherever it
is put), since the tool was asked to be there and what it would have taken off is worth seeing
beside the collision; `CutInCollision` turns that off.

2026-10-14, the same day: **every location of a path says what it was solved for**
(`CSolvedLocation::m_cvInput`, `k5_location.position` and `.axis`). An invented location had no
point of its own that a caller could reach, so the host used the cutter location the step was on the
way to, which is the end of the move: every step of a long move stood at its end, the machine did
not move through a plunge, the stamps of a whole unreachable stretch landed in one place and left
the stock ridged, and the player took such a run for a turn in place and passed it in one tick. One
record, five symptoms.

Next here, in order of value: **marching cubes** in the library and through the ABI, for a smooth
surface in place of the voxel faces; **the sweep** of the tool between two locations
in the library, so the interpolation can be coarse again; **the comparison** of what is left with
the model of the job, which is what a finish pass is judged by.

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

## Housekeeping, and two risks

- **`master` holds everything, and is the default branch.** `c-abi` was merged into it on
  2026-10-14 with the C ABI, the FreeCAD addon, the machine simulation and the cutting simulation;
  the feature branches behind it are finished and kept only for history. Nothing has to be merged
  before this repository can be read, which was the risk this entry used to record.
- **The tests that need FreeCAD still never run in CI**: the runner has no FreeCAD. 72 of the 149
  tests of the addon run without it, so the 74 that exercise the operation, the machine and the
  simulation are covered on one developer's machine alone — and that is the layer where every
  defect of the last three days was found, while the library, which is tested on every push, needed
  no repair at all. An AppImage and a second job would close it.
- **Nothing ships a built library.** `kin5axc` is built on Linux by CI and on Windows by hand. The
  binding looks in `KIN5AXC_LIBRARY`, then `kin5axcam/lib/`, then the build trees of a checkout, and
  that `lib` folder does not exist, so an installed addon finds nothing unless its user builds the
  C++ project first.
  This is the one thing standing between the project and a second user.
