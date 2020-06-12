from collections import deque


def flip(f):
    return lambda *a: f(*reversed(a))


def kickstart(it):
    deque(it, maxlen=0)
