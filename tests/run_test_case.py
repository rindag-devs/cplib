#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import json
import subprocess
import sys


def compare_json(expected, actual):
    """
    Compares two JSON objects. Ignores fields that are not present in 'expected'.
    """
    if isinstance(expected, dict):
        if not isinstance(actual, dict):
            return False, f"Type mismatch: expected dict, got {type(actual).__name__}"
        for key, value in expected.items():
            if key not in actual:
                return False, f"Missing key: '{key}'"
            match, reason = compare_json(value, actual[key])
            if not match:
                return False, f"Mismatch at key '{key}': {reason}"
    elif isinstance(expected, list):
        if not isinstance(actual, list):
            return False, f"Type mismatch: expected list, got {type(actual).__name__}"
        if len(expected) != len(actual):
            return (
                False,
                f"List length mismatch: expected {len(expected)}, got {len(actual)}",
            )
        for i, item in enumerate(expected):
            match, reason = compare_json(item, actual[i])
            if not match:
                return False, f"Mismatch at index {i}: {reason}"
    else:
        if expected != actual:
            return False, f"Value mismatch: expected '{expected}', got '{actual}'"
        else:
            return True, ""
    return True, ""


def run_simple_test(command, stdin_path, expected_stdout_path, expected_stderr_path):
    """
    Runs a simple command and compares stdout/stderr.
    """
    stdin_content = None
    if stdin_path:
        try:
            with open(stdin_path, "r", encoding="utf-8") as f:
                stdin_content = f.read()
        except FileNotFoundError:
            print(f"Error: Stdin file not found: {stdin_path}", file=sys.stderr)
            sys.exit(1)

    try:
        result = subprocess.run(
            command,
            input=stdin_content,
            capture_output=True,
            text=True,
            encoding="utf-8",
        )
    except FileNotFoundError:
        print(f"Error: Command not found: {command}", file=sys.stderr)
        sys.exit(1)

    # Check stdout if required
    if expected_stdout_path:
        try:
            with open(expected_stdout_path, "r", encoding="utf-8") as f:
                expected_stdout = f.read()
        except FileNotFoundError:
            print(
                f"Error: Expected stdout file not found: {expected_stdout_path}",
                file=sys.stderr,
            )
            sys.exit(1)

        if result.stdout != expected_stdout:
            print("FAILED: Stdout did not match expected stdout.")
            print("--- Expected Stdout ---")
            print(expected_stdout)
            print("--- Actual Stdout ---")
            print(result.stdout)
            sys.exit(1)

    # Check stderr if required
    if expected_stderr_path:
        try:
            with open(expected_stderr_path, "r", encoding="utf-8") as f:
                expected_stderr_json = json.load(f)
        except (FileNotFoundError, json.JSONDecodeError) as e:
            print(
                f"Error reading or parsing expected JSON '{expected_stderr_path}': {e}",
                file=sys.stderr,
            )
            sys.exit(1)

        try:
            actual_stderr_json = json.loads(result.stderr)
        except json.JSONDecodeError:
            print("FAILED: Stderr was not valid JSON.", file=sys.stderr)
            print("--- Expected Stderr (from file) ---", file=sys.stderr)
            print(json.dumps(expected_stderr_json, indent=2), file=sys.stderr)
            print("--- Actual Stderr ---", file=sys.stderr)
            print(result.stderr, file=sys.stderr)
            sys.exit(1)

        match, reason = compare_json(expected_stderr_json, actual_stderr_json)
        if not match:
            print(
                f"FAILED: Stderr JSON did not match expected JSON. Reason: {reason}",
                file=sys.stderr,
            )
            print("--- Expected Stderr (from file) ---", file=sys.stderr)
            print(json.dumps(expected_stderr_json, indent=2), file=sys.stderr)
            print("--- Actual Stderr (from program) ---", file=sys.stderr)
            print(json.dumps(actual_stderr_json, indent=2), file=sys.stderr)
            sys.exit(1)

    print("PASSED")
    sys.exit(0)


def run_interactive_test(interactor_cmd, solution_cmd, expected_stderr_path):
    """
    Runs an interactive test by piping the solution's stdin/stdout to the interactor.
    """
    try:
        # Start the interactor.
        # It writes to stdout (which pipes to solution stdin)
        # It reads from stdin (which comes from solution stdout)
        p_interactor = subprocess.Popen(
            interactor_cmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,  # Capture stderr to check JSON later
            text=True,
            encoding="utf-8",
        )

        # Start the solution.
        # stdin comes from interactor's stdout
        # stdout goes to interactor's stdin
        p_solution = subprocess.Popen(
            solution_cmd,
            stdin=p_interactor.stdout,
            stdout=p_interactor.stdin,
            stderr=subprocess.PIPE,  # We generally ignore solution stderr, or print it
            text=True,
            encoding="utf-8",
        )

        # Allow p_solution to receive EOF if p_interactor closes its stdout
        p_interactor.stdout.close()

        # Wait for valid completion
        # We capture interactor's stderr here
        _, interactor_stderr_data = p_interactor.communicate()
        p_solution.communicate()

    except FileNotFoundError as e:
        print(f"Error executing command: {e}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"An error occurred during interaction: {e}", file=sys.stderr)
        sys.exit(1)

    # After interaction, compare the interactor's stderr with the expected JSON
    if expected_stderr_path:
        try:
            with open(expected_stderr_path, "r", encoding="utf-8") as f:
                expected_stderr_json = json.load(f)
        except (FileNotFoundError, json.JSONDecodeError) as e:
            print(
                f"Error reading or parsing expected JSON '{expected_stderr_path}': {e}",
                file=sys.stderr,
            )
            sys.exit(1)

        try:
            actual_stderr_json = json.loads(interactor_stderr_data)
        except json.JSONDecodeError:
            print("FAILED: Interactor stderr was not valid JSON.", file=sys.stderr)
            print(f"--- Raw Stderr ---\n{interactor_stderr_data}", file=sys.stderr)
            sys.exit(1)

        match, reason = compare_json(expected_stderr_json, actual_stderr_json)
        if not match:
            print(
                f"FAILED: Interactor stderr JSON did not match. Reason: {reason}",
                file=sys.stderr,
            )
            print("--- Expected Stderr (from file) ---", file=sys.stderr)
            print(json.dumps(expected_stderr_json, indent=2), file=sys.stderr)
            print("--- Actual Stderr (from program) ---", file=sys.stderr)
            print(json.dumps(actual_stderr_json, indent=2), file=sys.stderr)
            sys.exit(1)

    print("PASSED")
    sys.exit(0)


def parse_args():
    parser = argparse.ArgumentParser(description="Integration test runner.")
    subparsers = parser.add_subparsers(dest="mode", required=True, help="Test mode")

    # --- Simple Mode ---
    # Used for checkers or simple binaries.
    p_simple = subparsers.add_parser("simple", help="Run a standalone command.")
    p_simple.add_argument("--stdin", help="File to pipe to stdin.")
    p_simple.add_argument("--expected-stdout", help="File containing expected stdout.")
    p_simple.add_argument(
        "--expected-stderr", help="File containing expected stderr JSON."
    )
    p_simple.add_argument(
        "command", nargs=argparse.REMAINDER, help="The command line to execute."
    )

    # --- Interactive Mode ---
    # Used for Interactors.
    p_inter = subparsers.add_parser(
        "interactive", help="Run an interactor against a solution."
    )
    p_inter.add_argument(
        "--solution", required=True, help="Command to run the solution executable."
    )
    p_inter.add_argument(
        "--expected-stderr",
        help="File containing expected stderr JSON (from interactor).",
    )
    p_inter.add_argument(
        "command", nargs=argparse.REMAINDER, help="The interactor command line."
    )

    return parser.parse_args()


def main():
    args = parse_args()

    # Clean up REMAINDER capture.
    # If the user invoked as "... -- ./binary", argparse includes the '--' in the list.
    cmd_args = args.command
    if cmd_args and cmd_args[0] == "--":
        cmd_args = cmd_args[1:]

    if not cmd_args:
        print(f"Error: No command specified for mode '{args.mode}'.", file=sys.stderr)
        sys.exit(2)

    if args.mode == "simple":
        run_simple_test(
            command=cmd_args,
            stdin_path=args.stdin,
            expected_stdout_path=args.expected_stdout,
            expected_stderr_path=args.expected_stderr,
        )
    elif args.mode == "interactive":
        run_interactive_test(
            interactor_cmd=cmd_args,
            solution_cmd=args.solution,
            expected_stderr_path=args.expected_stderr,
        )


if __name__ == "__main__":
    main()
