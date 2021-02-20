#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

import re
from pathlib import Path

from ..Config import CONFIG


def exportPath(cfg, path, dirname=""):
    return dirname / subRoot(cfg, path)


def subRoot(cfg, path):
    patF, patR = cfg["rootPat"], CONFIG["outDir"]
    original = Path(path)
    rootless = (
        original.relateive_to(patF)
        if original.parents and original.parents[0] == patF
        else original
    )
    return patR / rootless


_sanitizer_re = re.compile(CONFIG["sym"])


def sanitize(path):
    ps = map(lambda p: _sanitizer_re.sub("_", p), Path(path).parts)
    return str(Path(*ps))
