# kin5ax

Inverse kinematics and postprocessing for five-axis (5C) milling machines. A static C++17
library (`namespace kin5ax`), standard library only; the MFC application in the parent folder
is its editor, viewer and simulator, `kin5axTests` its GoogleTest suite.

## Goal

Turn a tool path given relative to the workpiece (cutter location data) into a machine
trajectory (values of the five joints, written as G-code) for *any* machine built of three
prismatic and two revolute joints in any order, without writing a dedicated postprocessor for
each machine type. The inverse problem is solved analytically, every solution is found (0, 1,
2, 4 or infinitely many per location), the joint limits and collisions are checked, and the
shortest feasible trajectory through the solution sets is selected.

## Base paper and author

K. Marciniak, J. Porter-Sobieraj, P. Zdroik, *Analytical methods of the inverse problem
solving for 5C milling machines*, MMSS'06, Kraków 2006 (Faculty of Mathematics and Information
Science, Warsaw University of Technology). The paper and this library grew out of
P. Zdroik's Master's thesis of 2004; they build on Marciniak, Porter-Sobieraj, *Universal
postprocessor for 5-axis milling machines with two rotating tables*, MMSS'03, Kraków 2003.

Code: Przemysław Zdroik (2004, revived in 2026 on Visual Studio 2022, x64).

## The machine model (.kc file)

A machine is two open kinematic sub-chains fixed to a common base: the *tool part* ends in
the spindle (segment type `TOOL`), the *material holder part* ends in the workpiece frame
(`MATERIAL_HOLDER`). Every segment is a constant transformation (translation, then a rotation
about an axis by an angle) followed by its joint motion: a translation along the local Z for a
`PRISMATIC_LINK`, a rotation about the local Z for a `REVOLUTE_LINK`. A segment carries an
offset, a default value, optional minimum/maximum limits and any number of collision boxes.
The material holder part is applied inverted, so the whole chain is one product of
transformations from the workpiece frame to the tool frame (the paper's re-indexed chain (6)).

The file is XML validated by the DTD embedded in every file (`kc.dtd`); angles in the file are
degrees, inside the library radians; lengths are whatever unit the file uses.

Example machines in `kin5axTests/TestChains/` (all are round-trip tested by the suite; the last
three also come as `.kc.json`, built from catalogue travels and swivel ranges, with the head and pivot
offsets estimated and marked so in their `author` field):

| file | machine |
|---|---|
| `3CWithSphericalWrist_*.kc` | three linear axes with a spherical wrist on the head (the original sample) |
| `AC_TiltRotaryTable.kc` | trunnion table: A tilts, C rotates the workpiece |
| `AC45_NutatingTable.kc` | nutating table, the A axis at 45 degrees |
| `BC_TiltRotaryTable.kc`, `..._XYonTable.kc` | B/C table; the second one with X and Y under the table |
| `WM308S2_BC_SwivelHead_RotaryTable.kc` | swivel head (B) plus rotary table (C) |
| `WM508MT2_AB_MillTurn.kc` | mill-turn: A and B in the head, the workpiece on prismatic axes |
| `SHW_UniForce7000_AC_ForkHead.kc` | SHW UniForce 7000: travelling column with a continuous fork milling head (C about the ram, A tilt), fixed clamping plate, no table axis at all |
| `Reiden_RX10_A_SwivelHead_C_RotaryTable.kc` | Reiden RX10: Z and the A swivel head (vertical to horizontal) above an X/Y cross table carrying the C rotary table |
| `Mazak_VTC800-30SR_B_SwivelHead_C_RotaryTable.kc` | Mazak VTC-800/30SR: travelling column with the B swivel head, a C rotary table on the fixed long table |

## The inverse problem in short

Input: the tool reference point *p* and the unit tool axis *v* in the workpiece frame.
Unknowns: two angles and three shifts. Because the orientation does not depend on the
translations, the problem decouples:

1. **Orientation**: the two rotary angles alpha1, alpha2 from the vector equation
   `V0 = R1(alpha1) F2 R2(alpha2) V3`, where F2 is the constant rotation between the revolute
   axes and V3 the tool axis in the last frame. Three analytic methods are implemented and
   selectable (`CPostprocessorSettings`): multiplication by the first axis, by the second axis,
   and the cone (geometric) method; the second angle is found either independently (then the
   pairs are validated) or from the first one (`alpha1 then alpha2`, the default, and the
   reverse).
2. **Position**: with the angles known the three shifts are a 3x3 linear system solved by
   Cramer's rule (`CPositionPostprocessor`).

Every combination gives 0, 1, 2 or 4 candidate configurations; each is then filtered by the
joint limits (including the equivalent angles, plus or minus a full turn) and by the collision
test of all boxes and of the tool.

## Data

- **.kc** (in): the machine, see above. Read by `CChainXmlReader` (pugixml, vendored in
  `3rdParty/pugixml/`, portable; the inline DTD is not validated, the reader checks the elements,
  attributes and numbers itself and names the line of every problem), then `CChain::CheckStructure`.
  Written by `CChain::toXML`. The MSXML reader was removed once the format path tests had shown
  both readers equal on every chain.
- `CChainFileReader::ReadFile` opens either of the two: by the extension (`.kc`, `.json`, any
  case), else by the first non-blank character of the file. The application's single Open goes
  through it.
- **.kc.json** (in and out): the same machine in JSON, described by `kc.schema.json`
  (draft 2020-12). The tool and the workpiece holder are members of their branch instead of
  entries in the segment list and carry no type and no limits; a joint is `"rotary"` or
  `"linear"`; a branch may have no joint at all; the name of the model, the machine, the
  author and the date of the last change are members of the document. `CChainJsonReader`
  and `CChainJsonWriter` on nlohmann/json (`3rdParty/nlohmann/`); no library validates a
  schema, so the reader checks the same rules itself and names the member at fault by its
  path, e.g. `tool_branch.links[1].rotation.axis`.
- **CL data** (in): APT-like text, one location per line, `GOTO/x,y,z,i,j,k`; `MSYS/` lines
  are recognised, anything else is reported as an error line and skipped. Parsed by
  `CCutterLocationData` (locale independent, CRLF tolerant).
- **.fcm** (in and out): the machine definition of the FreeCAD CAM workbench, a different
  description of the same machine. `CChainFcmReader` and `CChainFcmWriter`; see the section
  below, because the two files cannot hold the same things.
- **G-code** (out): one `N.. G1 X.. Y.. Z.. A.. B..` block per location from
  `CSimulationData::ExportToGCode`; the caller maps the five joints to the axis letters,
  angles in degrees or radians. No feed rates, no tool changes, no rollover handling.

## Single and multiple input

- *Single input*: `C5CPostprocessor::Solve` takes one location and returns every feasible
  configuration with a text log of the solving steps (the application's single input dialog).
- *Multiple input*: `CCLDataPostprocessor::Process` runs a whole CL file: it interpolates
  between locations so that no step exceeds the configured position/orientation limits,
  solves each location, handles the singular sections (below), reports the largest
  reproduction error, and fills a `CSimulationData` with one solution set per location.
  `CSimulationData::FindShortestPath` then picks one configuration per location by dynamic
  programming on the weighted sum of joint changes, skipping locations without a solution;
  the chosen trajectory feeds the simulation and the G-code export.

## Singularities

- **Orientation**: when the tool axis coincides with the first revolute axis, alpha1 is
  arbitrary (the tool is axially symmetric); the solver returns alpha1 of type `ANY_NUMBER`
  and alpha2 normally. Over a singular section of the path the ambiguous alpha1 is not
  resolved locally: it is interpolated along the path length between the last determined
  value before and the first after the section (both ends of the path: taken from the
  neighbour); the shifts are then recomputed for each interpolated angle
  (`SolveWithKnownAlpha1`). The other collinearities (two revolute axes, or the second axis
  and the tool) mean an ill-designed chain and are rejected when the chain is set.
- **Position**: prismatic axes that become linearly dependent for some joint values
  (zero determinant) give no solution and are logged; real machines limit the joints so it
  does not occur.

## The FreeCAD bridge (*.fcm)

`CChainFcmReader` and `CChainFcmWriter`, in the application under Chain > Import (FreeCAD .fcm)
and Export (FreeCAD .fcm).

**The two descriptions.** An .fcm file names every axis and gives its joint as an origin and a
direction in the machine base frame with every axis at zero; `role` sorts the axes into the head
(tool) and the table (workpiece) branch, `parent` and `sequence` order them, and at the home
position the tool frame and the workpiece frame both coincide with the base frame. kin5ax gives
every segment a constant transformation relative to the one before it, moves along or about the
local OZ, and closes each branch with a terminal segment.

Importing walks the axes and asks, for each one, where its joint sits in the frame of the
previous segment: the translation is the origin seen from there, the rotation is the one that
takes OZ onto the direction of the joint. Every such rotation is carried by the whole rest of the
branch, so the terminal segment is the inverse of everything accumulated before it, which is what
puts both frames back on the base frame at home. Exporting does the opposite: the forward
kinematics of the model at zero gives each joint its origin and its OZ, and everything is
expressed relative to the workpiece frame at home, which becomes the base frame of the file.

### What the converter and the library have in common

The uploaded `kcconv.cpp` carries its own geometry and its own model. All of it already exists
here, so nothing of it was taken as code; only the mapping above is new.

| kcconv | already in kin5ax |
|---|---|
| `Vec3` | `CR3Vector` |
| `Mat3::axisAngle` | `CRotation(axis, angle)` |
| `Mat3::between` | `CRotation(v0, v1)` |
| `Mat3::toAxisAngle` | `CRotation::GetAxis` and `GetAngle` (usable at 0 and 180 degrees since this branch) |
| `Frame`, its product and its inverse | `CTransformation`, `CChainSegment::GetConstantTransformation` and its inverted form |
| `Joint`, `Branch` | `CChainSegment` and the two segment vectors of `CChain` |
| `kcTransform`, `branchToKc`, `kcBranchToModel`, `modelToKc` | `CChain` itself; the .kc.json side is `CChainJsonReader` and `CChainJsonWriter` |
| `loadJson`, `saveJson` | nlohmann/json, vendored in `3rdParty/nlohmann/` |
| `--flip-table-linear`, `--flip-table-rotary` | `CFcmOptions` |
| the warnings on stderr | `CFcmReport`, shown by the application |

### What does not fit through, and what to do about it

Nothing is dropped in silence: every item below is added to the `CFcmReport` that the reader and
the writer fill, and the export also writes it into `machine.kinematics.notes`.

**The terminal transformations.** An .fcm machine has none: at home both frames are the base
frame. Importing therefore loses nothing, and exporting can always give away one of the two,
because the base frame of the file is chosen to be the workpiece frame at home. What is left is
the tool frame, and it has to be the base frame as well:

- *A rotation about the tool axis* is not reported and costs nothing. A milling tool is
  symmetric about its axis.
- *A shift along the tool axis* is what all eight sample machines have, between 0.1 and 0.4 of a
  unit. It is a tool length: carry it in the tool length offset of the post (G43, or the tool
  table), or give the milling tool a reference point height that accounts for it. The report
  prints the vector.
- *A shift across the tool axis* is a work offset: carry it in G54 to G59, or move the zero of
  the linear joints of the chain by the same amount and export again.
- *A tool axis that is not +Z at home* means the zero position of the chain is not the home
  position of the machine. Change the constant rotation of the joint that tilts the tool so that
  its parameter is zero when the tool stands upright, then export again. A machine that cannot
  bring its tool upright at all cannot be written as an .fcm file.

**Collision boxes** have no place in an .fcm file and are dropped. Keep the .kc or .kc.json as
the master copy of the machine and treat the .fcm as an export.

**Joint limits** are always present in an .fcm axis and optional in a chain. A segment without
limits is written with the full range (a turn either way, or a thousand units) and reported;
give the segment its limits before exporting, or narrow them in FreeCAD afterwards.

**Axis names** are the keys of the axes object, so two segments of the same name make the export
refuse the chain, and a name that is not a single upper-case letter is reported because the
FreeCAD post-processors expect X, Y, Z, A, B or C.

**Everything in an axis that is not kinematics** (`max_velocity`, `solution_preference`,
`allow_flip`, `wrap_strategy`) and the `toolheads` and `postprocessor` sections are dropped on
import, each one reported by name. On export, `CFcmOptions::m_sTemplateFilePath` names an
existing .fcm whose other sections are kept as they are, so a file can be round-tripped through
the chain without losing them.

**A base frame of its own** in `machine.kinematics.base_frame` is ignored on import and
reported: the joints are read exactly as they stand in the file.

### The one convention that has to be checked against the machine

The direction of a table axis. The value in the file is taken as the value of the joint
parameter of the model, with no negation, and the solver's own sign convention for the workpiece
branch does the rest. If a machine counts one of its table axes the other way round, set
`m_bNegateTableLinearAxes` or `m_bNegateTableRotaryAxes` in `CFcmOptions`. To find out: import
the machine, set one table axis to a known value, and compare the tool position the model
computes with the one FreeCAD shows for the same value. The same check settles the other
assumption, that the tool axis points along +Z at the home position.

## Main classes

| area | classes |
|---|---|
| geometry (`3DGeometry/`) | `CR3Point`, `CR3Vector`, `CR3CaughtVector` (position + axis), `CM4x4`, `CTransformation`, `CTranslation`, `CRotation`, `CScale`, `CQuaternion`, `CR3Box` (collision boxes, exact box/box, box/segment, box/sphere, box/cylinder tests) |
| machine (`Chain/`) | `CChain` (two segment vectors, forward kinematics, limits, collisions, XML out), `CChainSegment`, `CChainParameter` (value, unit, type single/any/none), `CChainConfiguration` (the five joint values, distance, equivalents), `CChainXmlReader`, `CChainJsonReader`, `CChainFcmReader` and the writers |
| tool (`MillingTool/`) | `CMillingTool` and `CMillingToolBall`, `CMillingToolCylinder`: cut volume for the voxel simulation and collision solid |
| solver (`Postprocessor/`) | `COrientationPostprocessor` (the angles), `CPositionPostprocessor` (the shifts), `C5CPostprocessor` (signs, limits, collisions, singular sections) |
| batch | `CCutterLocationData`, `CCLDataPostprocessor` (+ `CCLDataPostprocessingResult`), `CSimulationData` (solution sets, shortest path, G-code), `CPostprocessorSettings`, `CProgressCallback` |
| support | `CGeneratorException` and subclasses with the `MY_THROW` macro (file, line, function), `string_tools`, `double_tools` |

## What the postprocessor includes

Chain model with limits and collision boxes; analytic solver with three methods and both
solving orders; equivalent angles; interpolation of the input path; singular section handling;
limit and collision filtering of every candidate; shortest trajectory; error report; G-code
export; voxel material simulation (in the application, using `CMillingTool`); a test suite of
312 tests including random solver round trips on every model and hemisphere/meridian CL data.

## What is still missing

Tracked one by one in `../KnownBugs.md`; the open items that matter most:

- **C axis unwinding.** The trajectory metric wraps angles modulo a full turn while the
  G-code does not (#25): C359 to C1 counts as 2 degrees but sweeps 358 on a machine without
  rollover. Related: the singular-section interpolation forces alpha1 into [0, 2pi) and does
  not take the shortest arc (#40), only the first in-range equivalent of a solution is kept
  (#59), and the metric cannot tell one 180-degree step from two 90-degree steps on the pole
  (#24). A rollover-aware axis model (per-axis modulo or not, preferred winding) is the fix.
- Feasibility checks are skipped for the interpolated singular sections (#21).
- The non-default orientation methods (`independently`, `alpha2 then alpha1`) use a wrong
  first axis when the middle constant rotation is not about a horizontal axis (#39), and the
  axis method loses the sign on a boundary ratio (#22).
- A tool on a `TOOL` segment without boxes is never collision-tested (#38).
- Working tolerance: one global epsilon of 1e-15, effectively exact comparison at
  millimetre scale (#23, #56).
- Portability: the library has been compiled with MSVC only so far; the GUI and the tests still
  have a few include names that differ from the file names in case (#60).
- No feed rate, tool change or machine-specific words in the G-code; no DLL build yet.
