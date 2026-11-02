# SM Production CRT

Independent first-pass VST3 project. It does not alter the BASEMENT source tree or its existing build.

## Controls
DEPTH, RATE, NOISE, FILTER, CRUSH, MIX, STEREO, TAPE WEAR, POWER.

## Build
Windows x64 / JUCE 8.0.6 / C++17 / CMake. The GitHub Actions workflow builds and packages the VST3.

## DSP note
This is an original, adjustable first-pass recreation architecture inferred from the supplied demonstration: modulated short delay, tape-style wow/flutter and soft saturation, envelope-following low-pass, sample-rate/bit-depth reduction, noise, stereo width and equal-power mix. It is not claimed to be a bit-identical copy of the proprietary CRT DSP. Parameter ranges and interactions will be tuned against aligned reference audio.
