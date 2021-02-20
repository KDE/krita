#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

from collections import deque


def flip(f):
    return lambda *a: f(*reversed(a))


def kickstart(it):
    deque(it, maxlen=0)
