#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

from itertools import chain
from pathlib import Path


def iterPre(node, maxDepth=-1):
    """
    Visit nodes in pre order.

    Parameters
    ----------
    node: Node
    maxDepth: int
    Maximum depth level at which traversal will stop.

    Returns
    -------
    out: iter(Node)
    """

    def go(nodes, depth=0):
        for n in nodes:
            yield n
            # recursively call the generator if depth < maxDepth
            it = go(n.children, depth + 1) if maxDepth == -1 or depth < maxDepth else iter()
            yield from it

    return go([node])


def iterLevel(node, maxDepth=-1):
    """
    Visit nodes in level order.

    Parameters
    ----------
    node: Node
    maxDepth: int
    Maximum depth level at which traversal will stop.

    Returns
    -------
    out: iter(Node)
    """

    def go(nodes, depth=0):
        yield from nodes
        it = map(lambda n: go(n.children, depth + 1), nodes)
        it = chain(*it) if maxDepth == -1 or depth < maxDepth else iter()
        yield from it

    return go([node])


def iterLevelGroup(node, maxDepth=-1):
    """
    Visit nodes in level order just like `iterLevel`, but group nodes per level in an iterator.

    Parameters
    ----------
    node: Node
    maxDepth: int
    Maximum depth level at which traversal will stop.

    Returns
    -------
    out: iter(iter(Node))
    Returns an iterator that holds an iterator for each depth level.
    """

    def go(nodes, depth=0):
        yield iter(nodes)
        it = map(lambda n: go(n.children, depth + 1), nodes)
        it = chain(*it) if maxDepth == -1 or depth < maxDepth else iter()
        yield from filter(None, it)

    return go([node])


def iterPost(node, maxDepth=-1):
    """
    Visit nodes in post order.

    Parameters
    ----------
    node: Node
    maxDepth: int
    Maximum depth level at which traversal will stop.

    Returns
    -------
    out: iter(Node)
    """

    def go(nodes, depth=0):
        for n in nodes:
            it = go(n.children, depth + 1) if maxDepth == -1 or depth < maxDepth else iter()
            yield from it
            yield n

    return go([node])


def path(node):
    """
    Get the path of the given node.

    Parameters
    ----------
    node: Node

    Return
    ------
    out: list(Node)
    The path of nodes going through all the parents to the given node.
    """

    def go(n, acc=[]):
        acc += [n]
        n.parent and go(n.parent, acc)
        return reversed(acc)

    return list(go(node))


def pathFS(node):
    """
    Get the path of the given node just like `path`, but returns a OS filesystem path based on
    node names.

    Parameters
    ----------
    node: Node
    A node that has a `name` method.

    Return
    ------
    out: str
    The path of nodes going through all the parents to the given node in filesystem-compatile
    string format.
    """
    it = filter(lambda n: n.parent, path(node))
    it = map(lambda n: n.name, it)
    return Path("").joinpath(*it)


def iterDirs(node):
    it = iterPre(node)
    it = filter(lambda n: n.isGroupLayer(), it)
    it = filter(
        lambda n: any(i.isExportable() for i in chain(*map(lambda c: iterPre(c), n.children))), it
    )
    it = map(pathFS, it)
    return it
