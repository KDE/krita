##############################################################################
#
# Copyright (c) 2001 Zope Corporation and Contributors. All Rights Reserved.
#
# This software is subject to the provisions of the Zope Public License,
# Version 2.0 (ZPL).  A copy of the ZPL should accompany this distribution.
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY AND ALL EXPRESS OR IMPLIED
# WARRANTIES ARE DISCLAIMED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF TITLE, MERCHANTABILITY, AGAINST INFRINGEMENT, AND FITNESS
# FOR A PARTICULAR PURPOSE
#
##############################################################################

__version__='$Revision: 1.6 $'[11:-2]

from SelectCompiler import ast

ListType = type([])
TupleType = type(())
SequenceTypes = (ListType, TupleType)

class MutatingWalker:

    def __init__(self, visitor):
        self.visitor = visitor
        self._cache = {}

    def defaultVisitNode(self, node, walker=None, exclude=None):
        for name, child in node.__dict__.items():
            if exclude is not None and name in exclude:
                continue
            v = self.dispatchObject(child)
            if v is not child:
                # Replace the node.
                node.__dict__[name] = v
        return node

    def visitSequence(self, seq):
        res = seq
        for idx in range(len(seq)):
            child = seq[idx]
            v = self.dispatchObject(child)
            if v is not child:
                # Change the sequence.
                if type(res) is ListType:
                    res[idx : idx + 1] = [v]
                else:
                    res = res[:idx] + (v,) + res[idx + 1:]
        return res

    def dispatchObject(self, ob):
        '''
        Expected to return either ob or something that will take
        its place.
        '''
        if isinstance(ob, ast.Node):
            return self.dispatchNode(ob)
        elif type(ob) in SequenceTypes:
            return self.visitSequence(ob)
        else:
            return ob

    def dispatchNode(self, node):
        klass = node.__class__
        meth = self._cache.get(klass, None)
        if meth is None:
            className = klass.__name__
            meth = getattr(self.visitor, 'visit' + className,
                           self.defaultVisitNode)
            self._cache[klass] = meth
        return meth(node, self)

def walk(tree, visitor):
    return MutatingWalker(visitor).dispatchNode(tree)
