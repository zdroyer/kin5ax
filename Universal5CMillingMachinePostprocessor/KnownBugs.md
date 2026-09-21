# Known bugs

List from the code scan of 2026-09-22 (commit 351fb62). To be handled one by one.
Status: **open**, **fixed** (with the commit), **rejected** (not a bug after all).
"Verified" = the code was read and the defect confirmed; "reported" = found by the scan, not yet checked.
Nothing here was observed at run time unless a test is named.

## Wrong results in normal use

| # | Where | Defect | Check | Status |
|---|---|---|---|---|
| 1 | `kin5ax/3DGeometry/Rotation.cpp` `CRotation(v0,v1)` | The vector product of two versors has the length sin(angle), not 1, but it is used as a unit axis: the matrix is a rotation only for 0 and 90 deg (and -I for 180 deg). Only user: `CMillingTool::SetPositionAndOrientation` (voxel simulation). Tests: `RotationTests.cpp` (before the fix it passed only for 0 and 90 deg; at 45 deg det = 0.64 and v0 landed 0.21 away from v1). | verified | **fixed** b263fa5 |
| 2 | `kin5ax/MillingTool/MillingTool.cpp` `SetPositionAndOrientation` | The reference point offset is applied before the rotation: `R*(p - pos + ref*OZ)` instead of `R*(p - pos) + ref*OZ`; the cut volume of a tilted tool is displaced. | verified | open |
| 3 | `KinematicChainPropertiesDialog.cpp:439` | `m_dParameterValue` is in degrees but is converted with `radians_to_degrees`; a revolute joint set to 30 deg jumps after any other field is edited. | verified | open |

## Collision geometry

| # | Where | Defect | Check | Status |
|---|---|---|---|---|
| 4 | `R3Box.cpp` `InnerBoxFromOZCylinder` | Half side `R*sqrt(2)` instead of `R/sqrt(2)`: the "inner" box is bigger than the outer one, the tool shank collides like a square prism. | verified | open |
| 5 | `R3Box.cpp` `AsimetricIntersection` | The top face tests the diagonal (1,3) instead of the edge (0,3): one edge of the box is never tested. | verified | open |
| 6 | `R3Box.cpp` `HasIntersectionWithLineSegment` | The clipping assumes increasing coordinates; a segment running in the negative direction can be missed. | reported | open |
| 7 | `R3Box.cpp` `LineSegmentAndOZCylinderIntersection` | `+` instead of `*` in `dB`, `/2.0*dA` instead of `/(2.0*dA)`; hits on the extrapolated line. | reported | open |
| 8 | `MillingToolBall.cpp`, `MillingToolCylinder.cpp` | Ball: only the sphere is tested between R and 2R, the collision cylinder overlaps the ball and misses the top R; cylinder tool subtracts the reference point twice. | reported | open |

## CL-data parsing

| # | Where | Defect | Check | Status |
|---|---|---|---|---|
| 9 | `CutterLocationData.cpp` `ReadDouble` | `errno` is tested but never cleared: one out-of-range number turns every following line into an error. | verified | open |
| 10 | `CutterLocationData.cpp` `ReadDelims` | A leading `.` is a delimiter: `.5` is read as `5`, no error. | verified | open |
| 11 | `CutterLocationData.cpp` `LoadFile` | The last line is lost when the file does not end with a newline. | reported | open |

## Crashes and undefined behaviour

| # | Where | Defect | Check | Status |
|---|---|---|---|---|
| 12 | `Universal5CMillingMachinePostprocessorDoc.cpp` `GetPostprocessor` | Deletes `m_pPostprocessor` and may then throw without nulling it: double delete on the next call. | verified | open |
| 13 | `PostprocessingDialog.cpp` `fprintf(pFile,GenerateReportHead().c_str())` | Data (file paths) used as the format string. | verified | open |
| 14 | `Universal5CMillingMachinePostprocessorDoc.cpp` `OnSaveDocument` `fprintf(file,sXML.c_str())` | Data (the XML) used as the format string. | verified | open |
| 15 | `KinematicChainPropertiesDialog.cpp` `m_pActiveSegment` | Pointer into a vector; stale after add/delete segment or a replaced chain. | reported | open |
| 16 | `PostprocessingDialog.cpp`, `OnOpenDocument` | No try/catch around calls that throw `CGeneratorException`/`_com_error`: the application terminates. | reported | open |
| 17 | `CLDataPostprocessor.cpp` `InterpolateLocation` + dialog | A zero interpolation limit (also while typing "0.01") gives an endless loop; no locations gives 0/0. | reported | open |
| 18 | `R3Point.cpp` `toString` | 35-byte static buffer, overflows with coordinates of 4+ digits (mm files). | reported | open |
| 19 | `MainFrm.cpp` simulation thread | Pause+Stop leaves a suspended thread; tool/material replaced under the worker; pointer nulled after a blind Sleep; exit during simulation. | reported | open |
| 20 | `MainFrm.cpp:380` | Refresh period 0 ends the simulation after one step. | reported | open |

## Solver

| # | Where | Defect | Check | Status |
|---|---|---|---|---|
| 21 | `5CPostprocessor.cpp` `SolveWithKnownAlpha1` | Limits and collisions are checked while alpha1 is still ANY (value 0); the interpolated alpha1 and the recomputed shifts are never checked. | reported | open |
| 22 | `OrientationPostprocessor.cpp` `getAlpha1AxisMethod` (`todo: WHY`) | A ratio slightly below -1 is forced to +1: acos gives 0 instead of pi, "no solution" on a boundary orientation. | reported | open |
| 23 | `double_tools.h` `static double dEPSILON` | One copy per source file: the precision of the settings dialog has no effect; the working tolerance is 1E-15 everywhere. | verified | open |

## Design issues

| # | Where | Issue | Status |
|---|---|---|---|
| 24 | `CSimulationData::FindShortestPath` | The path length is a sum of absolute joint changes: one 180 deg step costs the same as two 90 deg steps on the pole; the choice depends on the last bits (Win32 and x64 differ for WM308S2, see `MeridianCLDataTests.cpp`). | open |
| 25 | `ChainParameter.h` `difference` + `ExportToGCode` | The angle distance wraps modulo 360 deg, the G-code does not: C359 -> C1 counts as 2 deg but is a 358 deg sweep on an axis without rollover. | open |

## Minor

| # | Where | Defect | Status |
|---|---|---|---|
| 26 | `MillingTool.h` | `~CMillingTool` is not virtual, the tool is deleted through a `CMillingTool*`. | open |
| 27 | `CutterLocationData.h` | Owns `char *m_pData` without copy constructor/assignment; `LoadFile` throws without `_close`; `GetLine` uses `index>size`. | open |
| 28 | `ChainSegment.cpp` | `SetPartCollision` uses `n>size`; `HasPartInCollision` prints a leading ", "; `toXML` does not escape the name. | open |
| 29 | `M4x4.h` `GetAt`/`SetAt` | `i<0` tested twice, `j<0` never. | open |
| 30 | `Chain.cpp` `CheckAndCorrectSegmentsNames` | `j=0` inside the `for(...;j++)` restarts at 1: duplicate names can survive. | open |
| 31 | `Chain.cpp` `GetActualParameter` | Off by one for the material holder part (unused function). | open |
| 32 | `5CPostprocessor.cpp` `SolveSingularOrientation` | `vFirst.front()` on an empty vector when no input is solvable (not reachable from the current caller). | open |
| 33 | `Universal5CMillingMachinePostprocessorView.cpp:82` | A GLU quadric is created on every repaint and never freed. | open |
| 34 | `KinematicChainPropertiesDialog.cpp` | `size_t m_nActivePart`: `GetCurSel()` of -1 and `max(0,n-1)` underflow. | open |
| 35 | `GLTools.cpp` arrows | A vector (0,0,-1) gives a zero rotation axis: drawn pointing up. | open |
| 36 | various | `MaterialBlockDialog` accepts voxel size 0; `CMaterial::m_dPixelSize` and `CMillingToolPropertiesDialog::m_pDoc` uninitialised; `"%d"` with `size_t` in two dialogs. | open |

## Fixed earlier (for the record)

| Where | Defect | Commit |
|---|---|---|
| `ChainSegment.cpp` | `=` instead of `==` for the revolute type | cadba46 |
| `Chain.cpp` `GetTranslationAxesAndOffset` | prismatic joints of the material holder part | d4717e1 |
| `Doc.cpp` `OnOpenDocument` | NULL routing frame, crash when a .kc is opened from the command line | 39c2024 |
| `SimulationData.cpp` | two wrong index bounds | eb58376 |
| `CutterLocationData.cpp` `LoadFile` | every file "too big" on x64 | b13eda3 |
| `Chain.cpp` `DetectCollisions` | false collisions with joints in the material holder part | 898e955 |
| `SimulationData.cpp` `FindShortestPath` | backtracking: set 0 never assigned, last set assumed non-empty | acb9eb3 |
