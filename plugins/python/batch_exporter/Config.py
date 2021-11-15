#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

import re
from collections import OrderedDict


CONFIG = {
    "outDir": "export",
    "sym": r"[^a-zA-Z0-9_.-]",
    "error": {"msg": "ERROR: {}", "timeout": 8000},
    "done": {"msg": "DONE: {}", "timeout": 5000},
    "delimiters": OrderedDict((("assign", "="), ("separator", ","))),  # yapf: disable
    "meta": {"c": [""], "e": ["png"], "m": [0], "p": [""], "t": [""], "s": [100], "i": [""]},
}
CONFIG["sym"] = re.compile(CONFIG["sym"])
