#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import subprocess
import threading
import argparse

CXX = "g++"
CXX_STD = "gnu++17"
SRC_INTR = "./intr.cpp"
BIN_INTR = "./intr"
SRC_STD = "./std.cpp"
BIN_STD = "./std"
N_DATA = 1


def fail(msg):
    print(msg, file=sys.stderr)
    exit(-1)


def get_compile_args(debug):
    args = ["-Wall", "-Wextra", f"-std={CXX_STD}"]
    if debug:
        args += ["-fsanitize=address,undefined", "-g", "-Og"]
    else:
        args.append("-O2")
    return args


def compile(cxx, cxx_std, src, bin, args):
    subprocess.run([cxx, *args, "-o", bin, src], check=True)


class SubprocessThread(threading.Thread):
    def __init__(self, args, stdin_pipe, stdout_pipe, stderr_pipe):
        threading.Thread.__init__(self)
        self.p = subprocess.Popen(args, stdin=stdin_pipe, stdout=stdout_pipe, stderr=stderr_pipe)

    def run(self):
        self.p.wait()


def run_test(id, inf, expected_file):
    t_sol = SubprocessThread([BIN_STD], subprocess.PIPE, subprocess.PIPE, subprocess.DEVNULL)
    t_judge = SubprocessThread([BIN_INTR, inf], t_sol.p.stdout, t_sol.p.stdin, subprocess.PIPE)

    t_sol.start()
    t_judge.start()
    t_sol.join()
    t_judge.join()

    result = t_judge.p.stderr.read().decode("utf-8").strip()
    expected = open(expected_file, "rb").read().decode("utf-8").strip()
    if result != expected:
        fail(f"Test {id} failed.\nExpected:\n{expected}\nGot:\n{result}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-r", "--recompile", action="store_true")
    parser.add_argument("-d", "--debug", action="store_true")
    args = parser.parse_args()

    compile_args = get_compile_args(args.debug)

    if args.recompile or not os.path.isfile(BIN_INTR):
        compile(CXX, CXX_STD, SRC_INTR, BIN_INTR, compile_args)

    if args.recompile or not os.path.isfile(BIN_STD):
        compile_args = get_compile_args(args.debug)
        compile(CXX, CXX_STD, SRC_STD, BIN_STD, compile_args)

    for i in range(N_DATA):
        run_test(i, f"data/{i}.in", f"data/{i}.json")


if __name__ == "__main__":
    main()
