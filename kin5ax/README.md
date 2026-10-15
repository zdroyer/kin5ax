# kin5ax — library reference

Inverse kinematics and postprocessing for five-axis (5C) milling machines. A C++17 library,
`namespace kin5ax`, standard library only; pugixml and nlohmann/json are vendored under
`3rdParty/`. The MFC application in `../kin5axChainEditorPlus` is its editor,
viewer and simulator; `../kin5axTests` is its GoogleTest suite. The pitch, the build and the state
of the code are in the [repository README](../README.md); this file is the reference.

## Contents

1. [The machine model](#the-machine-model)
2. [The inverse problem](#the-inverse-problem)
3. [Singularities](#singularities)
4. [Single and batch solving](#single-and-batch-solving)
5. [File formats](#file-formats)
6. [The FreeCAD bridge (.fcm)](#the-freecad-bridge-fcm)
7. [Main classes](#main-classes)
8. [Tolerances, units, threads, locale](#tolerances-units-threads-locale)
9. [Known limitations](#known-limitations)

## The machine model

A machine is two open kinematic sub-chains fixed to a common base:

- the **tool part** ends in the spindle (terminal segment of type `TOOL`);
- the **material holder part** ends in the workpiece frame (terminal `MATERIAL_HOLDER`).

The end of the tool branch is the **tool mount**, the place where the holder of the tool is clamped.
The tool point, which is the point the CL data programs, hangs the length of the assembly below it:
the holder (`CToolHolder`, declared by the chain file), the tool, and the zero point of the shape of
the tool, the tip of a flat end mill or the centre of the ball of a ball nose tool. A chain file that
declares no holder has its tool point at the mount itself, which is how the files were written before
the holder existed.

Every segment is a constant transformation (a translation, then a rotation about an axis by an
angle) followed by its joint motion: a translation along the local Z for a `PRISMATIC_LINK`, a
rotation about the local Z for a `REVOLUTE_LINK`. A segment carries an offset, a default value,
optional minimum/maximum limits and any number of collision boxes. The material holder part is
applied inverted, so the whole machine is one product of transformations from the workpiece
frame to the tool frame — the re-indexed chain (6) of the paper.

Any order of the five joints is allowed, with exactly three prismatic and two revolute joints in
total across the two parts. `CChain::CheckStructure` rejects chains with a wrong terminal, an
empty part, or limits in the wrong order; the solver rejects chains whose two revolute axes, or
second revolute axis and tool axis, are collinear (an ill-designed or redundant machine).

Angles are degrees in the files and radians inside the library. Lengths are in the unit the file
declares: `unit` is a required field of both forms of a chain file, `mm`, `m` or `in`, and a file
without it is refused rather than guessed at. The library converts nothing - a chain's numbers
stay in its own unit - and `CChain::GetUnitInMillimetres` is what a caller multiplies by. The
models shipped are metres. An `.fcm` file is millimetres by FreeCAD's definition, so a chain read
from one says `mm`, and a chain written to one is scaled into millimetres on the way out.

The bridge is reachable from C through `kin5axc/include/kin5axfcm.h`, a header of its own over
the same shared library, remarks included.

## The inverse problem

Input: the tool reference point *p* and the unit tool axis *v* in the workpiece frame.
Unknowns: two angles and three shifts. The orientation does not depend on the translations, so
the problem decouples.

**Orientation.** The two rotary angles α₁, α₂ solve the vector equation

    V0 = R1(α1) · F2 · R2(α2) · V3

where F2 is the constant rotation between the revolute axes and V3 the tool axis in the last
frame. Three analytic methods are implemented and selectable in `CPostprocessorSettings`:

| method | idea | status |
|---|---|---|
| **axis method, first axis** (default) | scalar product with the first revolute axis, which is invariant under R1 | tested on all models |
| axis method, second axis | scalar product with the second axis | wrong first-axis vector when the middle constant rotation is not about a horizontal axis (#39) |
| cone (geometric) method | the second axis lies on the intersection of two cones with a common apex | implemented, less tested |

The second angle is found either from the first (`ALPHA1_THEN_ALPHA2`, the default, and the
reverse) or independently, with the pairs then validated against the equation. **Use the
default** unless you are working on #39/#22. Every combination yields 0, 1, 2 or 4 candidate
pairs.

**Position.** With the angles known, the three shifts are a 3×3 linear system solved by
Cramer's rule (`CPositionPostprocessor`). A zero determinant means the prismatic axes have
become linearly dependent for these angles (possible on some head/head machines) and is
reported as no solution.

**Filtering.** Each candidate is checked against the joint limits — including the equivalent
angles, ± a full turn — and against the collision boxes of all segments and the three solids of the
tool assembly: the cutting part, the shank above it and the holder.
Only candidates passing both are returned. Both checks can be switched off in
`CPostprocessorSettings` (`m_bCheckLimits`, `m_bCheckCollisions`); with the collision check off
no tool model is needed to solve.

## Singularities

**Orientation singularity.** When the tool axis coincides with the first revolute axis, α₁ is
arbitrary (the tool is axially symmetric). The solver returns α₁ as `ANY_NUMBER` and α₂
normally. Over a singular *section* of a path the ambiguous α₁ is not resolved locally: it is
interpolated along the path length between the last determined value before the section and the
first after it (at either end of the path, the neighbour's value is used), and the shifts are
recomputed for each interpolated angle (`SolveWithKnownAlpha1`). Open issues: the interpolation
is a raw linear blend forced into [0, 2π), not the shortest arc (#40), and the interpolated
configurations are not re-checked against limits and collisions (#21).

**Position singularity.** See the zero determinant above. Real machines limit the joints so it
does not occur; it is logged when it does.

## Single and batch solving

- **Single location**: `C5CPostprocessor::Solve(p, v, chain, tool, log, report)` returns every
  feasible configuration. `log` is the solving narrated in words (what was found, what was
  rejected and why); `CSolveReport` is the same in numbers, and `Status()` says why a result is
  empty: no orientation solution, no position solution, all out of limits, all in collision.
  This is what the application's single-input dialog shows. The tool may be a null pointer when
  collisions are not checked.
- **Batch**: `CCLDataPostprocessor::Process` runs a whole CL file. It interpolates between
  locations so that no step exceeds the configured position/orientation limits (constant-rate
  turn of the axis, antiparallel axes handled), solves each location, handles singular sections,
  reports the largest reproduction error (forward kinematics of the solution against the input),
  fills a `CSimulationData` with one solution set per location and records for each its
  `SOLVE_STATUS`, where it came from, the cutter location of the file it belongs to, and the point
  and the axis it was solved for (`CSolvedLocation`) — so that a location without a solution still
  says where the tool was asked to be. A `CProgressCallback` receives the progress and may stop the
  batch between two locations
  (`Continue()` returning false; the result says it was cancelled).
- **Trajectory**: `CSimulationData::FindShortestPath` picks one configuration per location by
  dynamic programming on the weighted sum of joint changes, skipping locations without a
  solution; it reports progress the same way and returns false when stopped. The result feeds the
  simulation and `ExportToGCode`.

## File formats

| format | direction | reader / writer | notes |
|---|---|---|---|
| `.kc` | in, out | `CChainXmlReader`, `CChain::toXML` | XML, DTD embedded (`kc.dtd`); not validated against the DTD, the reader (pugixml) checks elements, attributes and numbers itself and names the line of a problem; numbers are written in their shortest exact form |
| `.kc.json` | in, out | `CChainJsonReader`, `CChainJsonWriter` | JSON, schema `kc.schema.json` (draft 2020-12); terminals are members of their branch, joints are `"rotary"` or `"linear"`, a branch may have no joint; model name, machine, author and date are document members; the reader checks the schema rules itself and names the member at fault by path, e.g. `tool_branch.links[1].rotation.axis` |
| `.fcm` | in, out | `CChainFcmReader`, `CChainFcmWriter` | FreeCAD 1.2 CAM machine definition; see below |
| CL data | in | `CCutterLocationData` | APT-like text, one `GOTO/x,y,z,i,j,k` per line; `MSYS/` recognised; other lines reported and skipped; locale independent, CRLF tolerant; inf, nan and out-of-range numbers are errors of their line |
| G-code | out | `CSimulationData::ExportToGCode` | one `N.. G1 X.. Y.. Z.. A.. B..` block per location; the caller maps joints to axis letters; degrees or radians; no feed rates, tool changes or rollover handling |

`CChainFileReader::ReadFile` opens `.kc` or `.kc.json` by extension (any case), else by the
first non-blank character of the file; this is the application's single Open. `.fcm` goes
through `CChainFcmReader`, which takes the import options and returns the report described below.

## The FreeCAD bridge (.fcm)

An `.fcm` file names every axis and gives its joint as an origin and a direction in the machine
base frame with all axes at zero; `role` sorts an axis into the head (tool) or the table
(workpiece) branch, `parent` and `sequence` order them, and at the home position the tool frame
and the workpiece frame both coincide with the base frame. kin5ax instead gives every segment a
constant transformation relative to the previous one and closes each branch with a terminal.

**Import** walks the axes and asks, for each, where its joint sits in the frame of the previous
segment: the translation is the origin seen from there, the rotation the one that takes OZ onto
the joint direction. Each rotation is carried by the rest of the branch, so the terminal segment
is the inverse of everything accumulated before it, which puts both frames back on the base
frame at home. **Export** does the opposite: forward kinematics at zero gives each joint its
origin and OZ, expressed relative to the workpiece frame at home, which becomes the file's base
frame.

Neither file can hold everything the other has. Nothing is dropped silently: every item below
goes into the `CFcmReport` the reader and writer fill, and the export also writes it into
`machine.kinematics.notes`.

| item | import | export |
|---|---|---|
| terminal transformations | none in `.fcm`, nothing lost | the workpiece terminal is absorbed into the base frame; the tool terminal must be the base frame too: a rotation about the tool axis costs nothing; a shift *along* the axis is a tool length (G43 or tool table, or the tool's reference point height) and is reported with its vector; a shift *across* the axis is a work offset (G54–G59, or move the zero of the linear joints and export again); a tool axis not +Z at home means the chain's zero is not the machine's home — change the constant rotation of the tilting joint so its parameter is zero with the tool upright; a machine that cannot bring its tool upright cannot be written as `.fcm` |
| collision boxes | — | dropped; keep `.kc`/`.kc.json` as the master and treat `.fcm` as an export |
| joint limits | always present | a segment without limits is written with the full range (a turn either way, or 1000 units) and reported |
| axis names | — | keys of the axes object: duplicate names refuse the export; a name that is not one upper-case letter is reported (FreeCAD posts expect X, Y, Z, A, B, C) |
| `max_velocity`, `solution_preference`, `allow_flip`, `wrap_strategy`, `toolheads`, `postprocessor` | dropped, each reported by name | kept from `CFcmOptions::m_sTemplateFilePath`, an existing `.fcm` whose other sections are preserved, so a file round-trips through the chain |
| `machine.kinematics.base_frame` | ignored and reported; joints are read as they stand | — |

**The one convention to check against the real machine**: the direction of a table axis. The
file value is taken as the joint parameter with no negation and the solver's own sign convention
for the workpiece branch does the rest. If a machine counts a table axis the other way, set
`CFcmOptions::m_bNegateTableLinearAxes` or `m_bNegateTableRotaryAxes`. To find out: import the
machine, set one table axis to a known value, and compare the tool position the model computes
with the one FreeCAD shows for the same value. The same check settles the other assumption, that
the tool axis points along +Z at home.

## Main classes

| area | classes |
|---|---|
| geometry (`3DGeometry/`) | `CR3Point`, `CR3Vector`, `CR3AnchoredVector` (position + axis), `CM4x4`, `CTransformation`, `CTranslation`, `CRotation`, `CScale`, `CR3Box` (collision boxes: exact box/box, box/segment, box/sphere, box/cylinder tests) |
| machine (`Chain/`) | `CChain` (two segment vectors, forward kinematics, limits, collisions with a `CCollisionReport` of which solids met, XML out), `CChainSegment`, `CChainParameter` (value, unit, type single/any/none), `CChainConfiguration` (the five joint values, distance, equivalents), `CChainXmlReader`, `CChainJsonReader`/`Writer`, `CChainFcmReader`/`Writer`, `CChainFileReader` |
| tool (`MillingTool/`) | `CMillingTool`, `CMillingToolBall`, `CMillingToolCylinder`: the cutting part (the cut volume of the simulation and a collision solid), the non-cutting shank and the holder (`CToolHolder`), the lengths they add up to and the distance from the tool mount to the tool point |
| solver (`Postprocessor/`) | `COrientationPostprocessor` (the angles), `CPositionPostprocessor` (the shifts), `C5CPostprocessor` (signs, limits, collisions, singular sections), `CSolveReport` and `SOLVE_STATUS` |
| batch | `CCutterLocationData`, `CCLDataPostprocessor` (+ `CCLDataPostprocessingResult`), `CSimulationData` (solution sets, shortest path, G-code), `CPostprocessorSettings`, `CProgressCallback`, `LocationInterpolation.h` (the densification between two cutter locations the solver and the chain-less simulation share: `InterpolateLocation`, `DensifyLocations`) |
| simulation (`Simulation/`) | `CMaterial` (the voxel block: material, surface, removed; the cut of the tool at its position; volumes), `CCuttingSimulation` (what every cutting simulation offers its driver: `Step`, `Run` with progress and stop, where it is, the gouges), `CMaterialSimulation` (the trajectory replayed on the chain, the tool following, the material cut), `CToolPathSimulation` (the same with no chain: a path of programmed points and tool axes, made dense by `LocationInterpolation.h`, the tool put at each location), `CSimpleSimulationSetup` (a hemisphere job sized for a machine). No thread of their own. The simulation is deliberately simple and not efficient: a voxel grid sampled at the voxel corners, no sweep between two locations. It is there to check the geometry of the postprocessor and to see how a machine model moves, not to predict a machined surface. |
| support | `CGeneratorException` (a `std::exception`) and subclasses with the `MY_THROW` macro (file, line, function), `string_tools`, `double_tools` |

## Tolerances, units, threads, locale

- One global tolerance, `double_tools.h` `dEPSILON = 1e-12`: a picometre at the metre scale of the
  models, a few hundred of the last bits of a double even at the 8 m travel of the largest one, and
  three orders below the accuracy the solver demonstrates. It is absolute and serves lengths, angles
  and degeneracy checks alike, so it is a stopgap: the right value follows the unit of the file, and
  the plan is a `CTolerances` of length, angle and relative in the settings (#56). The precision
  field of the settings dialog shows it and cannot change it (#23).
- Lengths are the unit the machine file declares; the CL file must use the same. Angles are
  degrees in files and dialogs, radians inside.
- `C5CPostprocessor` is safe to use from one thread per instance (the former shared statics are
  locals since #46).
- Nothing in the library depends on the numeric locale of the process: files are read with
  `std::from_chars` and written with `std::to_chars`, and no `setlocale` is called (#62). The
  `toString` methods used in logs still follow the locale.
- Every error is a `CGeneratorException`, a `std::exception` whose `what()` is the message; nothing
  else leaves the library.

## Known limitations

Tracked one by one in [`../KnownBugs.md`](../KnownBugs.md). The ones that matter when using the
solver:

- **No rotary-axis unwinding.** The trajectory metric wraps angles modulo a full turn, the
  G-code does not (#25): C359→C1 is scored as 2° but sweeps 358° on a machine without rollover.
  Related: the singular-section blend (#40), only the first in-range equivalent of a solution is
  kept (#59), and the metric cannot tell one 180° step from two 90° steps on the pole (#24). A
  rollover-aware axis model is the fix and is on the roadmap.
- Interpolated singular sections skip the limit and collision checks (#21).
- The non-default orientation methods are wrong on some chains (#39) and the axis method loses
  the sign on a boundary ratio (#22).
- A ball tool with height < 2R has disagreeing cut and collision volumes (#49).
- The holder and the shank are tested against the machine by the solver and against the remaining
  material by the simulation (a gouge). The solver cannot judge the material, because what is left
  depends on the trajectory so far.
- The generated machine models still have boxes and link positions in the part area, now only outside
  the tool branch (#67).
- No feed rate, tool change or machine-specific words in the G-code.

## References

- K. Marciniak, J. Porter-Sobieraj, P. Zdroik, *Analytical methods of the inverse problem
  solving for 5C milling machines*, MMSS'06, Kraków 2006.
- K. Marciniak, J. Porter-Sobieraj, *Universal postprocessor for 5-axis milling machines with two
  rotating tables*, MMSS'03, Kraków 2003, pp. 3–12.
- D. Manocha, Y. Zhu, *A fast algorithm and system for the inverse kinematics of general serial
  manipulators*, 1994 — the numerical alternative this library avoids.
