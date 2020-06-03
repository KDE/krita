import os
import re
from ..Config import CONFIG


def exportPath(cfg, path, dirname=""):
    return os.path.join(dirname, subRoot(cfg, path))


def subRoot(cfg, path):
    patF, patR = cfg["rootPat"], CONFIG["outDir"]
    return re.sub(patF, patR, path, count=1)


def sanitize(path):
    ps = path.split(os.path.sep)
    ps = map(lambda p: re.sub(CONFIG["sym"], "_", p), ps)
    ps = os.path.sep.join(ps)
    return ps
