# Fachprojekt Performance in Scientific Computing

This repository contains the micro benchmarks for the Fachprojekt Performance in Scientific Computing. The goal is to find one or multiple bottlenecks in performance for each of the benchmarks and try to rewrite the code in a way that improves performance.

## Requirements

Your system needs to have the following tools available:
- `cmake`
- GNU `make`
- A C++ compiler like `g++` or `clang`
- `perf`
- Python 3

## How to build and run a benchmark

The `run.py` script is used to both build and run.
The first argument to the script should always be the name of the folder of the benchmark.
E.g. to run benchmark `initial`, invoke the script like this:
```sh
./run.py initial
```

For building the benchmark the flag `--build` (`-b`) is used.
An additional flag `--clean` (`-c`) can be used to first clean the build folder and rebuild everything.
This can be a useful first step in debugging.
E.g. when building `initial` without a cleaning first, the invokation would be this:
```sh
./run.py initial --build
```

To run a benchmark the flag `--run` (`-r`) is used.
Per default the results printed to the terminal.
If the results should instead be stored in a `result.csv` file in the benchmark folder, the flag `--store` (`-s`) can be used.
All flags can be combined.
E.g. first clean building and then running the benchmark without storing to the file, perform the following invokation:
```sh
./run.py initial --build --clean --run 
```
The order of the flags is irrelevant.
