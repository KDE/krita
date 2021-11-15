#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

import re
from pathlib import Path


def exportPath(cfg, path, dirname, userDefined):
    return dirname / (Path(path) if userDefined else (cfg["outDir"] / Path(path)))


def sanitize(cfg, path):
    ps = map(lambda p: cfg["sym"].sub("_", p), Path(path).parts)
    return str(Path(*ps))
