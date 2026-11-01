# SM Production GLUE
A psychoacoustic vocal-to-mix integration VST3 prototype for Windows x64 / FL Studio.

## Signal routing
- Main input: vocal
- Auxiliary stereo input: full instrumental mix (external sidechain)
- Output: processed vocal

## Processing concept
This prototype uses six perceptual frequency regions, smoothed main/sidechain energy comparison, gentle spectral contouring, temporal envelope integration and subtle mid/side cohesion. It is intentionally not a conventional broadband sidechain compressor.

## Build
Windows 10/11 x64, Visual Studio 2022 C++ workload and CMake 3.22+.
```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --target SMProductionGlue_VST3
```
JUCE 8.0.6 is fetched automatically.

## Important
This is an original DSP prototype, not a reverse-engineered copy of Three-Body Technology UNMASK. Its psychoacoustic behavior requires listening tests and refinement against real vocal/mix material.