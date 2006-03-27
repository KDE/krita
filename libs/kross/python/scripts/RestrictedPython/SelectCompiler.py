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
"""Compiler selector.

$Id: SelectCompiler.py 24763 2004-05-17 05:59:28Z philikon $
"""

# Use the compiler from the standard library.
import compiler
from compiler import ast
from compiler.transformer import parse
from compiler.consts import OP_ASSIGN, OP_DELETE, OP_APPLY

from RCompile import \
     compile_restricted, \
     compile_restricted_function, \
     compile_restricted_exec, \
     compile_restricted_eval
