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
"""Restricted Python Expressions

$Id: Eval.py 24763 2004-05-17 05:59:28Z philikon $
"""

__version__='$Revision: 1.6 $'[11:-2]

from RestrictedPython import compile_restricted_eval

from string import translate, strip
import string

nltosp = string.maketrans('\r\n','  ')

default_guarded_getattr = getattr # No restrictions.

def default_guarded_getitem(ob, index):
    # No restrictions.
    return ob[index]

PROFILE = 0

class RestrictionCapableEval:
    """A base class for restricted code."""
    
    globals = {'__builtins__': None}
    rcode = None  # restricted
    ucode = None  # unrestricted
    used = None

    def __init__(self, expr):
        """Create a restricted expression

        where:

          expr -- a string containing the expression to be evaluated.
        """
        expr = strip(expr)
        self.__name__ = expr
        expr = translate(expr, nltosp)
        self.expr = expr
        self.prepUnrestrictedCode()  # Catch syntax errors.

    def prepRestrictedCode(self):
        if self.rcode is None:
            if PROFILE:
                from time import clock
                start = clock()
            co, err, warn, used = compile_restricted_eval(
                self.expr, '<string>')
            if PROFILE:
                end = clock()
                print 'prepRestrictedCode: %d ms for %s' % (
                    (end - start) * 1000, `self.expr`)
            if err:
                raise SyntaxError, err[0]
            self.used = tuple(used.keys())
            self.rcode = co

    def prepUnrestrictedCode(self):
        if self.ucode is None:
            # Use the standard compiler.
            co = compile(self.expr, '<string>', 'eval')
            if self.used is None:
                # Examine the code object, discovering which names
                # the expression needs.
                names=list(co.co_names)
                used={}
                i=0
                code=co.co_code
                l=len(code)
                LOAD_NAME=101
                HAVE_ARGUMENT=90
                while(i < l):
                    c=ord(code[i])
                    if c==LOAD_NAME:
                        name=names[ord(code[i+1])+256*ord(code[i+2])]
                        used[name]=1
                        i=i+3
                    elif c >= HAVE_ARGUMENT: i=i+3
                    else: i=i+1
                self.used=tuple(used.keys())
            self.ucode=co

    def eval(self, mapping):
        # This default implementation is probably not very useful. :-(
        # This is meant to be overridden.
        self.prepRestrictedCode()
        code = self.rcode
        d = {'_getattr_': default_guarded_getattr,
             '_getitem_': default_guarded_getitem}
        d.update(self.globals)
        has_key = d.has_key
        for name in self.used:
            try:
                if not has_key(name):
                    d[name] = mapping[name]
            except KeyError:
                # Swallow KeyErrors since the expression
                # might not actually need the name.  If it
                # does need the name, a NameError will occur.
                pass
        return eval(code, d)

    def __call__(self, **kw):
        return self.eval(kw)
