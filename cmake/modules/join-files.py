#!/usr/bin/env python3

import sys
import shutil


def main():
    if len(sys.argv) < 3:
        print("Usage: python join_files.py <input_files>... <output_file>")
        sys.exit(1)

    input_files = sys.argv[1:-1]
    output_file = sys.argv[-1]

    with open(output_file, 'wb') as outfile:
        for fname in input_files:
            with open(fname, 'rb') as infile:
                shutil.copyfileobj(infile, outfile)


if __name__ == "__main__":
    main()

