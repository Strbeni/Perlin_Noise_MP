# Perlin Noise Flow Field

A C++ visualization of a flow field using Perlin Noise and SDL2.

## How it works
- **Perlin Noise**: Generates smooth, organic-looking random values.
- **Grid**: The screen is divided into a grid. Each cell has a direction vector based on the noise value.
- **Particles**: 3,000 particles move through the grid, following the direction vectors.
- **Trails**: A semi-transparent overlay creates the fading trail effect.

## Prerequisites
- **SDL2**: Required for graphics rendering.
- **C++ Compiler**: (e.g., g++, clang, or MSVC).

## Compilation (MSYS2)
Run this command in your PowerShell to compile:
```powershell
C:\msys64\ucrt64\bin\g++.exe Perlin_Noise.cpp -o Perlin_Noise.exe -I"C:\msys64\ucrt64\include" -L"C:\msys64\ucrt64\lib" -lmingw32 -lSDL2main -lSDL2
```

## Running
I have already copied `SDL2.dll` to this folder for you. Just run:
```powershell
./Perlin_Noise.exe
```