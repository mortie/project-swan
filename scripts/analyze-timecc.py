#!/usr/bin/env python3

import sys
import os

if len(sys.argv) != 2:
    print("Usage:", sys.argv[0], "<dir>")

class File:
    def __init__(self, name, time):
        self.name = name
        self.time = time

class Project:
     def __init__(self, name):
         self.name = name
         self.files = []
         self.time = 0

projects: dict[str, Project] = {}

for name in os.listdir(sys.argv[1]):
    path = f"{sys.argv[1]}/{name}"
    parts = name.split("__")
    fname = parts[-1]
    fname = fname.replace("meson-generated_", "@@").replace(".time", "")
    projname = "/".join(parts[:-1])
    if projname == "":
        projname = "@"

    with open(path) as f:
        ns = int(f.read().strip().split(" ")[0])
    file = File(fname, ns / 1000000000)
    if projname in projects:
        proj = projects[projname]
    else:
        proj = Project(projname)
        projects[projname] = proj
    proj.files.append(file)
    proj.time += file.time

total = 0
for proj in sorted(projects.values(), key=lambda x: x.time):
    print(f"{proj.name}: {proj.time:.02f}s")
    for file in sorted(proj.files, key=lambda x: x.time):
        print(f"  {file.time:.02f}s {file.name}")
    total += proj.time
print(f"Total: {total:.02f}s")
