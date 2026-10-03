# Known bugs

List from the code scan of 2026-09-22 (commit 351fb62). To be handled one by one.
Status: **open**, **fixed** (with the commit), **rejected** (not a bug after all).
"Verified" = the code was read and the defect confirmed; "reported" = found by the scan, not yet checked.
Nothing here was observed at run time unless a test is named.

## Wrong results in normal use

| # | Where | Defect | Check | Status |
|---|---|---|---|---|
| 1 | `kin5ax/3DGeometry/Rotation.cpp` `CRotation(v0,v1)` | The vector product of two unit vectors has the length sin(angle), not 1, but it is used as a unit axis: the matrix is a rotation only for 0 and 90 deg (and -I for 180 deg). Only user: `CMillingTool::SetPositionAndOrientation` (voxel simulation). Tests: `RotationTests.cpp` (before the fix it passed only for 0 and 90 deg; at 45 deg det = 0.64 and v0 landed 0.21 away from v1). | verified | **fixed** b263fa5 |
| 2 | `kin5ax/MillingTool/MillingTool.cpp` `SetPositionAndOrientation` | The reference point offset is applied before the rotation: `R*(p - pos + ref*OZ)` instead of `R*(p - pos) + ref*OZ`; the cut volume of a tilted tool is displaced. Tests: `MillingToolTests.cpp` (before the fix it passed only for a vertical tool or a reference point on the tip). | verified | **fixed** 1756023 |
| 3 | `KinematicChainPropertiesDialog.cpp:439` | `m_dParameterValue` is in degrees but is converted with `radians_to_degrees`; a revolute joint set to 30 deg jumps after any other field is edited. No automated test (MFC dialog). | verified | **fixed** ba593a1 |

## Collision geometry

| # | Where | Defect | Check | Status |
|---|---|---|---|---|
| 4 | `R3Box.cpp` `InnerBoxFromOZCylinder` | Half side `R*sqrt(2)` instead of `R/sqrt(2)`: the "inner" box is bigger than the outer one, the tool shank collides like a square prism. Tests: `R3BoxTests.cpp` `CylinderBoxes`, `CornerRegionOfTheCircumscribedSquareIsFree`. | verified | **fixed** 15c4724 |
| 5 | `R3Box.cpp` `AsymmetricIntersection`, `HasIntersectionWithOZCylinder` | Both edge lists test the diagonal (1,3) instead of the edge (0,3): one edge of the box is never tested. Tests: `R3BoxTests.cpp` `BoxEdgeTest` (edge 7 failed before the fix). | verified | **fixed** f998e5c |
| 6 | `R3Box.cpp` `HasIntersectionWithLineSegment` | The clipping kept the wrong part of the segment whenever it ran towards smaller coordinates: a segment through the box with both ends outside was missed in most orientations. Now slab clipping (Liang-Barsky). Tests: `R3BoxTests.cpp` `BoxAgainstSegment` (2000 random segments). | verified | **fixed** c5f35f1 |
| 7 | `R3Box.cpp` `LineSegmentAndOZCylinderIntersection` | `+` instead of `*` in `dB`, `/2.0*dA` instead of `/(2.0*dA)`; the final `dZ1*dZ2` test uses the z of the extrapolated line: a segment inside the infinite cylinder but entirely above it is a hit (12 false hits out of 400 random cases). The function is removed with #37. | verified | **fixed** b6dfe81 |
| 8 | `MillingToolBall.cpp`, `MillingToolCylinder.cpp` | Ball: the cut volume had only the ball up to 2R (no shank beside its upper half); the collision cylinder began at the tip (false collisions beside the narrow nose) and ended R below the end of the tool. Cylinder tool: the cut volume was shifted down by the reference point. Now both tools have one solid for cutting and collisions (ball + shank from the equator; cylinder from the tip). Tests: `MillingToolTests.cpp`. | verified | **fixed** 88903cc |
| 37 | `R3Box.cpp` `HasIntersectionWithOZCylinder` | The algorithm (outer box, inner box, then the 12 box edges against the cylinder) missed a box whose flat face cuts the side of the cylinder less than `(1-1/sqrt(2))R` deep when no box edge is inside the cylinder. Hidden by #4 until then. Now exact: the box clipped to the slab of the cylinder, projected on XY, against the disc. Tests: `FlatFaceAgainstTheSide`, `AgreesWithPointSampling` (400 random poses). | verified | **fixed** b6dfe81 |

## CL-data parsing

| # | Where | Defect | Check | Status |
|---|---|---|---|---|
| 9 | `CutterLocationData.cpp` `ReadDouble` | `errno` is tested but never cleared: one out-of-range number turns every following line into an error. Tests: `CLDataParsingTests.cpp`. | verified | **fixed** 47f7760 |
| 10 | `CutterLocationData.cpp` `ReadDelims` | A leading `.` (or `+`) is a delimiter: `.5` is read as `5`, no error. | verified | **fixed** 47f7760 |
| 11 | `CutterLocationData.cpp` `LoadFile` | The last line is lost when the file does not end with a newline. Fixed together with: CR left on CRLF files read in binary mode; the orientation change summed from a bad first line. | verified | **fixed** 47f7760 |

## Crashes and undefined behaviour

| # | Where | Defect | Check | Status |
|---|---|---|---|---|
| 12 | `Universal5CMillingMachinePostprocessorDoc.cpp` `GetPostprocessor` | Deletes `m_pPostprocessor` and may then throw without nulling it: double delete on the next call. | verified | **fixed** 57df9cc |
| 13 | `PostprocessingDialog.cpp` `fprintf(pFile,GenerateReportHead().c_str())` | Data (file paths) used as the format string. | verified | **fixed** d58f899 |
| 14 | `Universal5CMillingMachinePostprocessorDoc.cpp` `OnSaveDocument` `fprintf(file,sXML.c_str())` | Data (the XML) used as the format string. | verified | **fixed** d58f899 |
| 15 | `KinematicChainPropertiesDialog.cpp` `m_pActiveSegment` | Pointer into a vector; stale after add/delete segment or a replaced chain: the tree rebuild in `SetActiveDocument` fires the selection handler, which dereferenced it and indexed the vector with the old item data. | verified | **fixed** bd982f1 |
| 16 | `PostprocessingDialog.cpp`, `OnOpenDocument` | No try/catch around calls that throw `CGeneratorException`/`_com_error`: the application terminates. `OnOpenDocument` got its catch with the reader move (2605ba7); `OnBnClickedStart` now catches around `Process`/`FindShortestPath` and refuses to start without a milling tool. | verified | **fixed** 869424a |
| 17 | `CLDataPostprocessor.cpp` `InterpolateLocation` + dialog | A zero interpolation limit (also while typing "0.01") gives an endless loop; no locations gives 0/0. | reported | **fixed** 1447c0b |
| 18 | `R3Point.cpp` `toString` | 35-byte static buffer, overflows with coordinates of 4+ digits (mm files). | reported | **fixed** ce75f65 |
| 19 | `MainFrm.cpp` simulation thread | Pause+Stop leaves a suspended thread; tool/material replaced under the worker; pointer nulled after a blind Sleep; exit during simulation. | reported | open |
| 20 | `MainFrm.cpp:380` | Refresh period 0 ends the simulation after one step. | verified | **fixed** b6ee4e1 |

## Solver

| # | Where | Defect | Check | Status |
|---|---|---|---|---|
| 21 | `5CPostprocessor.cpp` `SolveWithKnownAlpha1` | Limits and collisions are checked while alpha1 is still ANY (value 0); the interpolated alpha1 and the recomputed shifts are never checked (after `SetAmbiguousParameter` the shifts from `m_oPositionPostprocessor.Solve` go straight into the result). | verified | open |
| 22 | `OrientationPostprocessor.cpp` `getAlpha1AxisMethod` (`todo: WHY`) | A ratio slightly below -1 is forced to +1: acos gives 0 instead of pi, "no solution" on a boundary orientation (`dDenominator = dNumerator` whenever the magnitude is larger, sign ignored). Proposed: clamp the ratio to [-1,1] within a tolerance, no solution beyond it. | verified | open |
| 23 | `double_tools.h` `static double dEPSILON` | One copy per source file: the precision of the settings dialog has no effect; the working tolerance is 1E-15 everywhere. | verified | open |

## Design issues

| # | Where | Issue | Status |
|---|---|---|---|
| 24 | `CSimulationData::FindShortestPath` | The path length is a sum of absolute joint changes: one 180 deg step costs the same as two 90 deg steps on the pole; the choice depends on the last bits (Win32 and x64 differ for WM308S2, see `MeridianCLDataTests.cpp`). | open |
| 25 | `ChainParameter.h` `difference` + `ExportToGCode` | The angle distance wraps modulo 360 deg, the G-code does not: C359 -> C1 counts as 2 deg but is a 358 deg sweep on an axis without rollover. | open |

## Minor

| # | Where | Defect | Status |
|---|---|---|---|
| 26 | `MillingTool.h` | `~CMillingTool` is not virtual, the tool is deleted through a `CMillingTool*`. | **fixed** e510631 |
| 27 | `CutterLocationData.h` | Owns `char *m_pData` without copy constructor/assignment (a copied object crashed the test run); `LoadFile` throws without `_close`; `GetLine` uses `index>size`. Now a `vector<char>`, `fopen`/`fread`, `>=`. | **fixed** 47f7760 |
| 28 | `ChainSegment.cpp` | `SetPartCollision` uses `n>size`; `HasPartInCollision` prints a leading ", "; `toXML` does not escape the name (a name with a quote gives a file that does not load). Tests: `ChainTests.cpp` `SegmentNamesAreEscapedInXml`. | **fixed** 874d335 |
| 29 | `M4x4.h` `GetAt`/`SetAt` | `i<0` tested twice, `j<0` never. Tests: `M4x4Tests.cpp` (the negative column cases threw nothing before the fix). | **fixed** f82a993 |
| 30 | `Chain.cpp` `CheckAndCorrectSegmentsNames` | `j=0` inside the `for(...;j++)` restarts at 1: duplicate names can survive. | **fixed** e0e981d |
| 31 | `Chain.cpp` `GetActualParameter` | Off by one for the material holder part (unused function). | **fixed** d7a1c87 |
| 32 | `5CPostprocessor.cpp` `SolveSingularOrientation` | `vFirst.front()` on an empty vector when no input is solvable (not reachable from the current caller). | **fixed** 6fa8a61 |
| 33 | `Universal5CMillingMachinePostprocessorView.cpp:82` | A GLU quadric is created on every repaint and never freed. | **fixed** ac6538c |
| 34 | `KinematicChainPropertiesDialog.cpp` | `size_t m_nActivePart`: `GetCurSel()` of -1 and `max(0,n-1)` underflow. | **fixed** c0e8462 |
| 35 | `GLTools.cpp` arrows | A vector (0,0,-1) gives a zero rotation axis: drawn pointing up. | **fixed** 25c7486 |
| 36 | various | `MaterialBlockDialog` accepts voxel size 0; `CMaterial::m_dPixelSize` and `CMillingToolPropertiesDialog::m_pDoc` uninitialised; `"%d"` with `size_t` in three dialogs; also `CMaterialBlockDialog::m_pDoc` uninitialised. | **fixed** a93f8ce |

## Scan of 2026-09-22, second pass over kin5ax (commit 3410951)

Four module reviews (3DGeometry, Chain, Postprocessor/SimulationData/CLDataPostprocessor, MillingTool/CutterLocationData/string_tools/Exceptions) plus a check of the top findings by reading the code. "verified" = read and confirmed by hand, "reported" = from the review only. Nothing here was observed at run time.

### Wrong results

| # | Where | Defect | Check | Status |
|---|---|---|---|---|
| 38 | `Chain.cpp` `DetectCollisions` | The milling tool is tested against the machine only inside the `k` loop after `if(boxes2.empty()) continue;` (both the material-part and the tool-part loops): a chain whose TOOL segment has no box never reports a tool collision, the solver accepts the pose. All eight test chains have a box on the TOOL segment. Tests: `ChainTests.cpp` `Collisions.*` (two of three failed before the fix). | verified | **fixed** 1c99605 |
| 39 | `OrientationPostprocessor.cpp:31` `SetAxes` | `m_r3vM` (the first axis seen from the second frame) is the mirror `(-N1x,-N1y,N1z)` instead of `F2^T*OZ`; equal only when the axis of the middle constant rotation lies in the XY plane (R e3 - R^T e3 = 2 (a x e3) sin theta). Used by `getAlpha2AxisMethod`, i.e. the non-default solving methods (`ANGLES_SOLVING_INDEPENDENTLY`, `ALPHA2_THEN_ALPHA1`): WM308S2 (F2 axis ~(1,1,1)) then finds no solution. The default `ALPHA1_THEN_ALPHA2` path does not use it. | verified | open |
| 40 | `5CPostprocessor.cpp:309` `SolveSingularOrientation` | alpha1 across a singular section is a linear blend of the raw neighbour values, then forced into [0,2pi): not the shortest arc (3 deg -> 355 deg is swept through 180 deg with the tool on the pole), and a jump of 2pi against neighbours that live in [-2pi,0] (holder-side C axis with sign -1); when the data ends on the pole the ANY value 0 is the target. | verified (blend and range); arc consequence by analysis | open |
| 41 | `R3Box.h` `ExpandToPoint` | The all-zero box is the 'empty' sentinel, but it is also the legitimate state after expanding to the origin: `b.ExpandToPoint(0,0,0); b.ExpandToPoint(10,10,10)` gives [10,10]^3. Callers: `CChain::GetBoundingBox` (first part at the origin), `CCutterLocationData` box (first OK line at 0,0,0). Impact today: the view's optimal scale only. | verified | open |
| 42 | `CLDataPostprocessor.cpp:156` `InterpolateLocation` | The tool axis is interpolated as a normalised linear blend: antiparallel axes (0,0,1) -> (0,0,-1) give the zero vector at t=0.5 and `GetUnitVector` throws out of `Process` (whole batch lost); the angular step is not uniform (179 deg change: the middle step is ~74 deg). Now a constant-rate turn in the plane of both axes. Tests: `InterpolationTests.cpp` `AntiparallelAxesAreInterpolated`, `OrientationTurnsAtAConstantRate`. | verified (throw path) | **fixed** dc6abd4 |

### Crashes, undefined behaviour, silent acceptance

| # | Where | Defect | Check | Status |
|---|---|---|---|---|
| 43 | `SimulationData.cpp:115` `FindShortestPath` | `if(m_vIndexes.size()<3) return;` - with exactly two solution sets no choice is made (indexes stay 0,0), although the dynamic programme works for two. Test: `SimulationDataTests.cpp` `TwoSetsOnly`. | verified | **fixed** 2e24a22 |
| 44 | `Chain.cpp:186` `DeleteActiveSegment` | Loop bound `i<=vPart.size()` and no `>=size()` check: a stale index (two deletes of the last segment, index never adjusted) erases `end()`. Nothing stops deleting the TOOL/MATERIAL_HOLDER terminal (only the dialog does). | verified | open |
| 45 | `ChainParameter.h` `difference` | `min(fmod(|2pi-d|,2pi), fmod(d,2pi))` is wrong for d in (3pi,4pi): 350 deg vs -350 deg (both legal equivalents) gives 340 deg instead of 20 deg. Correct: `d=fmod(d,2pi); min(d,2pi-d)`. Moot if #25 removes the wrap. Tests: `ChainParameterTests.cpp`. | verified | **fixed** c5c88d6 |
| 46 | `5CPostprocessor.cpp:200` `Solve` | `static CR3Vector r3vT1,r3vT2,r3vT3; static CR3Point r3pP1;` are shared by all postprocessors and threads (the GUI runs a worker thread). Now locals. | verified | **fixed** 213d219 |
| 47 | `ChainXmlReader.cpp` `ReadChain` | No structural validation: terminal types, their count and position, empty parts (a DTD-less file validates nothing). Downstream `m_vMaterialHolderPart.back()` on an empty vector, `SetParameters` throwing from `Update()` on every repaint, `operator[]` index arithmetic assuming one terminal per part. Now `CChain::CheckStructure` (terminals, empty parts, limit order), applied by both readers; the pugixml reader also checks elements, attributes and numbers. Tests: `ChainXmlReaderTests.cpp`. | verified | **fixed** 40247d0 |
| 48 | `CutterLocationData.cpp` `ParseLine`/`ReadDelims` | Non-numeric tokens between numbers are skipped and a 7th number is ignored, so `GOTO/1,2,3,foo,0,0,1` and `GOTO/1,2,3,0,0,1,99` are OK lines; `-inf`/`-nan` pass `strtod` without ERANGE and reach the solver. | verified | open |
| 49 | `MillingToolBall.cpp` `CauseCollision` | The whole sphere is tested without clipping to `m_dCuttingHeight`, while `IsInOZCuttingSpace` clips: for a tool with H < 2R (accepted by the constructor) collision and cut volumes disagree. | verified | open |
| 50 | `MillingTool.cpp`, `MillingToolBall.cpp` | No validation of the dimensions: R, Rz, H are `fabs`'d, 0 accepted for all, the reference height unchecked, the `R<Rz` check commented out; `AxialCylinderBoundingBox` returns the default box (the origin) for radius 0, which stretches the bounding box to the world origin. | reported | open |

### Weaknesses and portability

| # | Where | Defect | Check | Status |
|---|---|---|---|---|
| 51 | `MillingTool.cpp:70` `SetPositionAndOrientation` | `static bool bFirst` is shared by every tool instance (and thread): the second tool's first placement computes a bogus move vector. Latent: `m_r3vMoveVector` has no users. | verified | open |
| 52 | `Transformation.cpp:88` `CTransformation::Inverted` | Returns the identity (`return CTransformation(1);` then unreachable code); non-virtual, so any rotation/translation held as a `CTransformation` inverts to the identity. No library callers today; throwing would be safer. The implicit `CRotation(const CTransformation&)` lets any transform become a `CRotation` whose `Inverted()` is the transpose. | verified | open |
| 53 | `Rotation.cpp:125` `GetAngle`, `:111` `GetAxis` | `acos` of the trace expression without clamping: NaN when rounding gives a trace above 3 (e.g. `R*R.Inverted()`); `GetAxis` returns the zero vector for 0 and 180 deg rotations (the fallback is commented out). Now clamped, and the axis comes from the symmetric part at a half turn; it is a unit vector. Needed by the .fcm export, which reads the axis and the angle back out of a frame. Tests: `RotationTests.cpp`. | verified | **fixed** b1675b0 |
| 54 | `Quaternion.cpp` (class unused) | Two-vector constructor falls back to the X axis for antiparallel vectors (same bug class as #1); `AngleOfRotation` uses `2.0*3.14` as 2pi; the default quaternion is (0,0,0,0), not the identity. | reported | open |
| 55 | `R3Box.cpp` `BoxFromCylinder`, `R3Point.cpp` `d_MAX` (both unused) | `BoxFromCylinder` grows only Z for a horizontal axis (wrong X/Y extents in every non-axial case); `d_MAX` has no `fabs` (Chebyshev distance of (0,0,0),(5,0,0) is 0). | reported | open |
| 56 | `double_tools.h` | The absolute tolerance 1e-15 is below one ULP for |x| > 8, so `are_equal`, `CR3Point::==`, `Contains`, `is_between_or_equal` are effectively exact comparisons at mm scale (ULP of 1000.0 is 1.1e-13); `is_zero` is strict (`<`) while `are_equal` is inclusive (`<=`). Distinct from the per-TU copy (#23). | verified | open |
| 57 | `Chain.cpp` parameters | Two copies of the joint values: `SetParameters` writes both `m_oActualParameters` and the segments, `ZeroAllParameters` and `CChainSegment::SetParameterValue` (dialog slider) write only the segments; `GetParameters`, `GetActualParameter`, the log header and the restore in `ComputeAndSetError` read the stale copy. `CChainConfiguration(int)` gives every parameter `DISTANCE_UNIT`. | reported | open |
| 58 | `OrientationPostprocessor.cpp` `getAlpha2SquareEquation` | When the t^2 coefficient vanishes the root alpha2 = pi (t = tan(alpha2/2) = inf) is dropped; every root is pushed with a spurious `+pi` companion filtered later (duplicates when the genuine roots differ by pi). | reported | open |
| 59 | `5CPostprocessor.cpp` `ApplyParameterConstraints` | Only the first in-range equivalent of a solution survives (`break`), and a chain without any constraint gets no equivalents at all: `FindShortestPath` never sees the representative nearest to the previous location (relates to #25). | reported | open |
| 60 | portability | `stdafx.h` includes `<io.h>` (MSVC-only, unused); `#pragma once` in `string_tools.cpp` and `R3Vector.cpp`; `R3Box.h` uses `CTransformation` with no declaration (only the friend declaration in `R3Point.h`, a friend-injected name is invisible to conforming lookup); lower-case include names in the tests and the GUI; `kin5ax.vcxproj` sets no `LanguageStandard` (C++14 default) while the tests use C++17; Release runtime /MT (lib) vs /MD (tests); `using namespace std` and `static string sNL` in headers. Done: `<io.h>`/`<fcntl.h>`/`<sys/*.h>` out of the prelude, the MSVC pragmas guarded, the two `#pragma once` in .cpp files, the forward declaration in `R3Box.h`, the include names of the tests, C++17 and one runtime everywhere, no `using namespace std`. Left: the include names of the MFC application (Windows only anyway), `static std::string sNL` in a header. | verified | **fixed** (library) |
| 61 | `Exceptions/` | `CGeneratorException` does not derive from `std::exception` (no `what()`, invisible to `catch(std::exception&)`); `catch (CGeneratorException e)` by value in `5CPostprocessor.cpp` (3x) and `CLDataFileDialog.cpp`; `CMemoryMyException` describes itself as 'Invalid function argument(s)'; `GeneratorException.cpp:19` `sprintf` with an embedded NUL in the format. | reported | open |
| 62 | `CutterLocationData.cpp:59`, `string_tools.cpp:31` | `setlocale(LC_NUMERIC,"C")` is process-global (the simulation thread formats numbers concurrently) and not restored if `LoadFile` throws; `double_to_string` (`%.8G`, used by `toXML`) follows the locale, a Polish locale would write `0,5` into a .kc file. | reported | open |
| 63 | various minor | `GetConstraint` logs revolute limits in radians; `AddSegmentAfterActiveSegment` inserts before the active segment; `SetActiveSegment` overwrites its own -1; XML reader accepts a one-sided limit as unconstrained, min > max, a zero axis becomes OX, a comment inside a part fails to load; `COrientationPostprocessor(F2,V3)` never sets `m_dPiValue`; `CauseCollision` takes two transformations by value in the innermost collision loop; `CR3Vector::m_vdV[3]` and `CTransformation::glM[16]` are uninitialised GUI-only members; editor state (`m_nActiveSegmentIndex`, `m_sFileName`, `m_nHighlightedPart`) travels with every chain copy; `HasIntersectionWithSphere` counts touching at faces but not at corners; `HasIntersectionWithOZCylinder` silently requires dZ >= 0; `CScale::Inverted` divides by zero without a check. | reported | open |
| 64 | `ChainSegment.cpp` `GetName` | Returns the fixed label "Tool holder" or "Material holder" for the terminal segments and ignores `m_sName`: the name written in the file is read into the member and then never used, and `toXML` writes the label back, so a name given to the tool or to the holder is lost on the first save. The same happened to the name of the whole chain until 7f8cd4c. Found while writing the JSON reader; the JSON form has the same behaviour. | verified | open |
| 65 | `string_tools.cpp` `double_to_string`, `CChain::toXML` | `%.8G` in the .kc file: an axis 0.57735026919 read back as 0.57735027, a chain saved as XML was not the chain saved as JSON. The XML writer now uses `double_to_string_exact` (shortest round trip); the logs keep `%.8G`. Found by `ChainFormatPathsTests.cpp`. | verified | **fixed** eb46594 |

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
