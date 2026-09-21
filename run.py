#!/usr/bin/env python3
"""
Run the micro-benchmarks.

Usage: run.py <target_dir> [--build] [--run] [--clean] [--store]

Examples:
  ./run.py initial --build
  ./run.py initial -r -s
  ./run.py initial -brc
"""
import argparse
from contextlib import nullcontext
import shutil
import subprocess
from pathlib import Path

BENCHMARKS_DIR = Path(__file__).parent.resolve()


def clean_target(target: str) -> None:
    build_dir = BENCHMARKS_DIR / target / "build"
    if build_dir.is_dir():
        shutil.rmtree(build_dir)


def build_target(target: str) -> None:
    target = BENCHMARKS_DIR / target
    if not target.is_dir():
        print(f"Target directory not found: {target}")
        raise SystemExit(1)

    build_dir = target / "build"
    build_dir.mkdir(exist_ok=True)

    # Run cmake .. and make in build dir
    rc = subprocess.run(["cmake", ".."], cwd=build_dir).returncode
    if rc != 0:
        print("cmake failed. Check output for details.")
        raise SystemExit(rc)

    rc = subprocess.run(["make"], cwd=build_dir).returncode
    if rc != 0:
        print("make failed. Check output for details.")
        raise SystemExit(rc)


def run_target(target: str, store: bool) -> None:
    target_path = BENCHMARKS_DIR / target
    build_dir = target_path / "build"
    if not build_dir.is_dir():
        raise SystemExit(
            f"Build directory not found: {build_dir}. Build first with --build."
        )

    exe = build_dir / target
    if not exe.is_file():
        raise SystemExit(f"No executable found in {build_dir}")

    results_path = target_path / "results.csv"
    with open(results_path, "wb") if store else nullcontext(None) as f:
        rc = subprocess.run([exe], cwd=build_dir, stdout=f).returncode
        if rc != 0:
            raise SystemExit(
                f"Micro-benchmark failed with return code {rc}. Check output for details."
            )

    if store:
        print(f"Results stored in {results_path}")


def parse_args():
    parser = argparse.ArgumentParser(
        description="Build/run one of the micro-benchmarks."
    )
    parser.add_argument(
        "target",
        help="target directory (containing CMakeLists.txt)",
    )
    parser.add_argument(
        "-b",
        "--build",
        action="store_true",
        help="run build",
    )
    parser.add_argument(
        "-r",
        "--run",
        action="store_true",
        help="run target",
    )
    parser.add_argument(
        "-c",
        "--clean",
        action="store_true",
        help="clean build directory before building",
    )
    parser.add_argument(
        "-s",
        "--store",
        action="store_true",
        help="store run output to results.csv",
    )
    return parser.parse_args()


def main():
    args = parse_args()

    if args.clean:
        clean_target(args.target)

    if args.build:
        build_target(args.target)

    if args.run:
        run_target(args.target, args.store)


if __name__ == "__main__":
    main()
