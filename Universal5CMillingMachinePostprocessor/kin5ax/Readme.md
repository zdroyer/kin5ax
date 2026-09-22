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
solving for 5C milling machines*, Faculty of Mathematics and Information Science, Warsaw
University of Technology. It builds on Marciniak, Porter-Sobieraj, *Universal postprocessor
for 5-axis milling machines with two rotating tables*, MMSS'03, Krakow 2003.

Code: Przemysław Zdroik (2004-2006, revived in 2026 on Visual Studio 2022, x64).

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

Example machines in `kin5axTests/TestChains/` (all are round-trip tested by the suite):

| file | machine |
|---|---|
| `3CWithSphericalWrist_*.kc` | three linear axes with a spherical wrist on the head (the original sample) |
| `AC_TiltRotaryTable.kc` | trunnion table: A tilts, C rotates the workpiece |
| `AC45_NutatingTable.kc` | nutating table, the A axis at 45 degrees |
| `BC_TiltRotaryTable.kc`, `..._XYonTable.kc` | B/C table; the second one with X and Y under the table |
| `WM308S2_BC_SwivelHead_RotaryTable.kc` | swivel head (B) plus rotary table (C) |
| `WM508MT2_AB_MillTurn.kc` | mill-turn: A and B in the head, the workpiece on prismatic axes |

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

- **.kc** (in): the machine, see above. Read by `CChainXmlReader` (MSXML on Windows; the
  interface is std-only, the reader is the file to swap for a portable parser), written by
  `CChain::toXML`.
- **CL data** (in): APT-like text, one location per line, `GOTO/x,y,z,i,j,k`; `MSYS/` lines
  are recognised, anything else is reported as an error line and skipped. Parsed by
  `CCutterLocationData` (locale independent, CRLF tolerant).
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

## Main classes

| area | classes |
|---|---|
| geometry (`3DGeometry/`) | `CR3Point`, `CR3Vector`, `CR3CaughtVector` (position + axis), `CM4x4`, `CTransformation`, `CTranslation`, `CRotation`, `CScale`, `CQuaternion`, `CR3Box` (collision boxes, exact box/box, box/segment, box/sphere, box/cylinder tests) |
| machine (`Chain/`) | `CChain` (two segment vectors, forward kinematics, limits, collisions, XML out), `CChainSegment`, `CChainParameter` (value, unit, type single/any/none), `CChainConfiguration` (the five joint values, distance, equivalents), `CChainXmlReader` |
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
- Portability: the XML reader is MSXML (Windows only); the planned Linux port also needs the
  `<io.h>` include removed and a few case-sensitive include names fixed (#60).
- Not thread-safe: static working variables in the solver (#46).
- No feed rate, tool change or machine-specific words in the G-code; no DLL build yet.
