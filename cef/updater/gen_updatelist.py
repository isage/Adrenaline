#!/usr/bin/env python3

import re
from pathlib import Path

updater_dir = Path(__file__).resolve().parent

header_path = updater_dir / "../../adrenaline_version.h"
template_path = updater_dir / "psp-updatelist.template"
output_path = updater_dir / "psp-updatelist.txt"

values = {}

with open(header_path, "r") as f:
    for line in f:
        match = re.match(r"#define\s+(\w+)\s+(\d+)", line)
        if match:
            name, value = match.groups()
            values[name] = int(value)

version = (
    (values["ADRENALINE_VERSION_MAJOR"] << 24)
    | (values["ADRENALINE_VERSION_MINOR"] << 16)
    | (values["ADRENALINE_VERSION_MICRO"] << 8)
)

version_str = f"0x{version:08x}"

print(version_str)

with open(template_path, "r") as f:
    content = f.read()

content = content.replace("{{ADRENALINE_VERSION}}", version_str)

with open(output_path, "w") as f:
    f.write(content)