# kin5ax — universal 5-axis inverse kinematics and postprocessor

Analytic inverse kinematics for any five-axis milling machine built from three linear and two
rotary joints in any order: head/head, head/table, table/table, nutating, mill-turn. One solver,
no per-machine postprocessor. Reads and writes the machine definitions of FreeCAD 1.2 CAM (`.fcm`).

```
tool path in the workpiece frame            machine trajectory
GOTO/x,y,z,i,j,k  ──►  kin5ax  ──►  N.. G1 X.. Y.. Z.. A.. C..
                          │
              every solution per location (0, 1, 2, 4 or ∞),
              joint limits, collision boxes, singular sections,
              shortest feasible trajectory
```

## Why it exists

A 5-axis postprocessor has to solve, for every cutter location, a trigonometric equation with
five unknowns that has 0, 1, 2, 4 or infinitely many solutions, and then pick one solution per
location so that the whole trajectory is feasible and short. Commercial posts solve this for one
machine type each; numerical solvers find one solution and are slow. kin5ax solves it in closed
form for the whole class of 3-prismatic + 2-revolute chains and enumerates all solutions, which
is what makes the trajectory selection possible.

The method is from the paper

> K. Marciniak, J. Porter-Sobieraj, P. Zdroik, *Analytical methods of the inverse problem
> solving for 5C milling machines*, MMSS'06, Kraków 2006 — Faculty of Mathematics and
> Information Science, Warsaw University of Technology.

and the code from P. Zdroik's Master's thesis (2004). On the hardware of the time it solved
5.8–6.6 kHz locations per second with position errors below 1e-12 on a 0.6 × 0.6 × 0.3 workpiece.

## Why now

FreeCAD 1.2 introduced machine definitions (`.fcm`, see [FreeCAD/Machines](https://github.com/FreeCAD/Machines))
and multi-axis operations, but the kinematic solver and the 5-axis postprocessor behind them are
still open ([discussion #23042](https://github.com/FreeCAD/FreeCAD/discussions/23042)). kin5ax
imports and exports `.fcm` and solves the inverse problem for those machines today.

The second reason is the one this repository is a demonstration of: an AI design loop
(generate a part → generate a toolpath → *is it machinable on this machine?* → revise) needs a
fast, exact, machine-aware judge of the toolpath. That judge is this library. See
[Roadmap](#roadmap).

## What is in the repository

| directory | what | platform |
|---|---|---|
| [`kin5ax/`](kin5ax/) | the library: C++17, standard library only (pugixml and nlohmann/json vendored), no Windows dependency | Windows (MSVC) and Linux (GCC) built and tested; macOS untried |
| [`kin5axTests/`](kin5axTests/) | GoogleTest suite, 709 tests, with 11 machine models under `TestChains/` | same |
| [`Universal5CMillingMachinePostprocessor/`](Universal5CMillingMachinePostprocessor/) | MFC application: kinematic chain editor, 3D viewer, single-location solver with a step-by-step log, batch postprocessing of CL files, a rudimentary voxel simulation that checks the geometry of the postprocessor on a hemisphere sized for the machine | Windows |

Machine models shipped and round-trip tested (catalogue travels and swivel ranges; head and
pivot offsets estimated and marked as such in the file):

| file | machine |
|---|---|
| `AC_TiltRotaryTable` | trunnion table, A tilts, C rotates the workpiece |
| `AC45_NutatingTable` | nutating table, A axis at 45° |
| `BC_TiltRotaryTable`, `..._XYonTable` | B/C table, the second with X and Y under the table |
| `WM308S2_BC_SwivelHead_RotaryTable` | swivel head B + rotary table C |
| `WM508MT2_AB_MillTurn` | mill-turn: A and B in the head, workpiece on the linear axes |
| `SHW_UniForce7000_AC_ForkHead` | travelling column, continuous fork head, no table axis |
| `Reiden_RX10_A_SwivelHead_C_RotaryTable` | A swivel head (vertical to horizontal) over an X/Y cross table with a C rotary table |
| `Mazak_VTC800-30SR_B_SwivelHead_C_RotaryTable` | travelling column, B swivel head, C rotary table |
| `3CWithSphericalWrist_*` | three linear axes with a spherical wrist (the original sample) |

The last three catalogue machines also come as `.kc.json`; the tests check that both forms are the
same machine.

## Build

**Library and tests, any platform** (CMake 3.20+, a C++17 compiler; GoogleTest is fetched on the first
configure):

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

On Windows add the generator and the toolset of the application project, `-G "Visual Studio 18 2026" -A x64 -T v143`,
and `-C Release` to `ctest`. Builds warning-free with MSVC 19.44 (v143) and GCC 15; the tests pass on both.
In WSL keep the build tree in the Linux file system (`cmake -S /mnt/c/... -B ~/kin5ax-build -G Ninja`): the
GoogleTest sub-build fails on a mounted Windows drive.

**The application** (Windows, Visual Studio 2022 or later with MFC, CMake on the PATH): open
`Universal5CMillingMachinePostprocessor/Universal5CMillingMachinePostprocessor.sln`. Its `kin5ax`
project is a Makefile project that builds the library with CMake into `build/` and the application
links that `kin5ax.lib`; the source list of the library lives in `kin5ax/CMakeLists.txt` only. The
tests are CMake only. One script does everything on Windows:

```powershell
powershell -ExecutionPolicy Bypass -File kin5axTests/build_and_test.ps1 [-Start]
```

CMake Debug and Release of library and tests, the test run, both application builds and, with
`-Start`, a launch of each.

## Use

From C++:

```cpp
#include "ChainFileReader.h"      // kin5ax/Chain
#include "5CPostprocessor.h"      // kin5ax/Postprocessor

kin5ax::CChain chain = kin5ax::CChainFileReader::ReadFile("machine.kc");   // .kc or .kc.json
kin5ax::CPostprocessorSettings settings;             // default: alpha1 then alpha2, axis method
settings.m_bCheckCollisions = false;                 // no tool model: solve without the collision test
kin5ax::C5CPostprocessor post(chain, settings);

std::string log;
kin5ax::CSolveReport report;
kin5ax::tChainConfigurationVector solutions = post.Solve(
    kin5ax::CR3Point(0.10, 0.20, 0.05), kin5ax::CR3Vector(0.0, 0.7071, 0.7071),
    chain, nullptr, log, report);
// every feasible (alpha1, alpha2, t1, t2, t3) for this tool position and axis;
// report.Status() says why the vector is empty: no orientation, no position, limits, collision
```

A whole CL file goes through `CCLDataPostprocessor::Process`, the trajectory through
`CSimulationData::FindShortestPath` and `ExportToGCode`; the application's postprocessing dialog
does exactly that. A command-line tool for the same is on the roadmap.

The library reference, the machine model and the file formats are in [`kin5ax/README.md`](kin5ax/README.md).

## State of the code, honestly

The library was written in 2004 and revived in September 2026 on Visual Studio 2022 / x64,
using agentic coding tools throughout: a scan of the code produced 69 numbered findings
([`KnownBugs.md`](KnownBugs.md)); 51 are fixed, each with a test and a commit; 2 are half fixed
and 16 are open and triaged by effect. Everything in that file was found by reading the code, not by a machine
crash, so read it before trusting an edge case.

What works and is tested: the default solving path (`alpha1 then alpha2`, axis method) on all
11 machines with random round trips and hemisphere/meridian paths; limits, equivalents,
collision boxes, singular-section interpolation, trajectory selection, G-code export, the three
file formats and every path between them, independence from the numeric locale of the process,
the voxel material and the cutting simulation (library code, the application only draws and drives it),
the tool assembly of holder, shank and cutting part with its collisions against the machine and
against the material that is left.

What is not: the two non-default orientation methods are wrong on some chains (#39, #22);
rotary axes are not unwound, so C359→C1 is scored as 2° but programmed as a 358° sweep (#25,
#40, #59); interpolated singular sections skip the feasibility checks (#21); the working
tolerance is one absolute constant for lengths, angles and degeneracy alike (#56); the generated machine models still have boxes and link
positions in the part area outside the tool branch (#67). Feed rates, tool changes and
machine-specific words are not written to the G-code.

## Roadmap

1. Linux/macOS build (CMake, CI).
2. A C ABI (`kin5axc`) and a `kin5ax-post` command-line tool with a JSON machinability report.
3. Rollover-aware axis model (#25) and the singular-section feasibility check (#21).
4. Python bindings for use from FreeCAD and from agent tools.
5. **Machinability-in-the-loop demo**: an agent generates a part and a toolpath in FreeCAD,
   kin5ax judges it on a real machine, the agent revises. Separate repository, linked here when
   it exists.

Contributions on any of these are welcome; open an issue first for 3 and 4.

## Licence

MIT. See [`LICENSE`](LICENSE). pugixml (MIT) and nlohmann/json (MIT) are vendored under
`kin5ax/3rdParty/` with their own licence files.

## Author

Przemysław Zdroik — code (2004, 2026). Method with K. Marciniak and J. Porter-Sobieraj.
