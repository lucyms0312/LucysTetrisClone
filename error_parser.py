#!/usr/bin/env python3

import re

def parse_errors():
    try:
        with open('compilererror.txt', 'r') as f:
            content = f.read()
    except FileNotFoundError:
        print("compilererror.txt not found.")
        return

    if not content.strip():
        print("No errors found in compilererror.txt.")
        return

    errors = []
    lines = content.split('\n')
    for line in lines:
        if 'error:' in line or 'undefined reference' in line:
            errors.append(line.strip())

    if not errors:
        print("No specific errors identified.")
        return

    # Generate description and steps
    description = "Compilation errors detected:\n"
    for error in errors:
        description += f"- {error}\n"

    steps = "Suggested steps to fix:\n"
    for error in errors:
        if 'undefined reference' in error and 'sfml' in error.lower():
            steps += "- Install SFML libraries: sudo apt-get install libsfml-dev\n"
        elif 'no such file' in error:
            steps += "- Check if all source files are present.\n"
        else:
            steps += "- Review the error message and correct the code accordingly.\n"

    # Append to TODO_tetris_fixes.txt
    with open('../TODO_tetris_fixes.txt', 'a') as f:
        f.write("\n[ ] Fix compilation errors\n")
        f.write("Description:\n" + description + "\n")
        f.write("Steps:\n" + steps + "\n")

    print("Error analysis added to ../TODO_tetris_fixes.txt")

if __name__ == "__main__":
    parse_errors()
