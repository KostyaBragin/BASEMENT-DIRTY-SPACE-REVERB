# BASEMENT — DIRTY SPACE REVERB
A deliberately rough algorithmic reverb for bedroom rock, garage rock, punk and lo-fi recordings.

## Build
Windows 10/11 x64, Visual Studio 2022 C++ workload and CMake 3.22+.
```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --target BASEMENT_VST3
```
JUCE 8.0.6 is fetched automatically on first configure.

## Controls
Decay, Pre-delay, Size, Mix, Damping, Width, Dirt, Low Cut, High Cut and Freeze.
Factory characters: Basement, Garage, Bunker, Abandoned Factory, Dirty Spring.

## DSP status
First working implementation pass: JUCE algorithmic reverb core, pre-delay buffer, input/return saturation, filtering, width and freeze controls. Distinct spring/garage impulse character and further decay-time refinement remain planned.
