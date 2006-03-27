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

__version__='$Revision: 1.7 $'[11:-2]

import string, math, random
import DocumentTemplate.sequence

utility_builtins = {}

utility_builtins['string'] = string
utility_builtins['math'] = math
utility_builtins['random'] = random
utility_builtins['whrandom'] = random
utility_builtins['sequence'] = DocumentTemplate.sequence

try:
    import DateTime
    utility_builtins['DateTime']= DateTime.DateTime
except: pass

def same_type(arg1, *args):
    '''Compares the class or type of two or more objects.'''
    t = getattr(arg1, '__class__', type(arg1))
    for arg in args:
        if getattr(arg, '__class__', type(arg)) is not t:
            return 0
    return 1
utility_builtins['same_type'] = same_type

def test(*args):
    l=len(args)
    for i in range(1, l, 2):
        if args[i-1]: return args[i]

    if l%2: return args[-1]
utility_builtins['test'] = test

def reorder(s, with=None, without=()):
    # s, with, and without are sequences treated as sets.
    # The result is subtract(intersect(s, with), without),
    # unless with is None, in which case it is subtract(s, without).
    if with is None: with=s
    d={}
    tt=type(())
    for i in s:
        if type(i) is tt and len(i)==2: k, v = i
        else:                           k= v = i
        d[k]=v
    r=[]
    a=r.append
    h=d.has_key

    for i in without:
        if type(i) is tt and len(i)==2: k, v = i
        else:                           k= v = i
        if h(k): del d[k]

    for i in with:
        if type(i) is tt and len(i)==2: k, v = i
        else:                           k= v = i
        if h(k):
            a((k,d[k]))
            del d[k]

    return r
utility_builtins['reorder'] = reorder
