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

__version__='$Revision: 1.5 $'[11:-2]

limited_builtins = {}

def limited_range(iFirst, *args):
    # limited range function from Martijn Pieters
    RANGELIMIT = 1000
    if not len(args):
        iStart, iEnd, iStep = 0, iFirst, 1
    elif len(args) == 1:
        iStart, iEnd, iStep = iFirst, args[0], 1
    elif len(args) == 2:
        iStart, iEnd, iStep = iFirst, args[0], args[1]
    else:
        raise AttributeError, 'range() requires 1-3 int arguments'
    if iStep == 0: raise ValueError, 'zero step for range()'
    iLen = int((iEnd - iStart) / iStep)
    if iLen < 0: iLen = 0
    if iLen >= RANGELIMIT: raise ValueError, 'range() too large'
    return range(iStart, iEnd, iStep)
limited_builtins['range'] = limited_range

def limited_list(seq):
    if isinstance(seq, str):
        raise TypeError, 'cannot convert string to list'
    return list(seq)
limited_builtins['list'] = limited_list

def limited_tuple(seq):
    if isinstance(seq, str):
        raise TypeError, 'cannot convert string to tuple'
    return tuple(seq)
limited_builtins['tuple'] = limited_tuple
