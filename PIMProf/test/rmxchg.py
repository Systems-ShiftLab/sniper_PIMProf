import os, argparse, subprocess
import re

# parse arguments
parser = argparse.ArgumentParser()
parser.add_argument("fasm", metavar="test.all.s")
args = parser.parse_args()

fasm = args.fasm

# parse the file
output = []
with open(fasm, "r") as f:
    lines = f.readlines()

    i = 0
    while i < len(lines):
        # print(i)
        splitted = lines[i].strip().split()
        if len(splitted) >= 3 and splitted[0] == "xchgq" and splitted[1] == "%rcx," and splitted[2] == "%rcx":
            # print("skip")
            i = i + 4
        else:
            output.append(lines[i])
            i = i + 1


with open(fasm, "w") as f:
    f.writelines(output)