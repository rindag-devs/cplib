#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import subprocess
import sys
import os

CXX = "g++"
CXX_STD = "gnu++17"
SRC_VAL = "./val.cpp"
BIN_VAL = "./val"
N_DATA = 4


def fail(msg):
    print(msg, file=sys.stderr)
    exit(-1)


def get_compile_args(debug):
    args = ["-Wall", "-Wextra", f"-std={CXX_STD}"]
    if debug:
        args += ["-fsanitize=address,undefined", "-g", '-Og']
    else:
        args.append("-O2")
    return args


def compile(cxx, cxx_std, src, bin, args):
    subprocess.run([cxx, *args, "-o", bin, src], check=True)


def run_test(id, inf, expected_file):
    result = subprocess.run([BIN_VAL, "--report-format=json"],
                            encoding='utf-8',
                            capture_output=True,
                            stdin=open(inf, "rb"),
                            check=True).stderr.strip()
    expected = open(expected_file, "rb").read().decode('utf-8').strip()
    if result != expected:
        fail(f"Test {id} failed.\nExpected:\n{expected}\nGot:\n{result}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-r", "--recompile", action="store_true")
    parser.add_argument("-d", "--debug", action="store_true")
    args = parser.parse_args()

    if args.recompile or not os.path.isfile(BIN_VAL):
        compile_args = get_compile_args(args.debug)
        compile(CXX, CXX_STD, SRC_VAL, BIN_VAL, compile_args)

    for i in range(N_DATA):
        run_test(i, f"data/{i}.in", f"data/{i}.json")


if __name__ == '__main__':
    main()
