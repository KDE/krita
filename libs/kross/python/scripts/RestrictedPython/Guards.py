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
__version__ = '$Revision: 1.14 $'[11:-2]

import exceptions

# This tiny set of safe builtins is extended by users of the module.
# AccessControl.ZopeGuards contains a large set of wrappers for builtins.
# DocumentTemplate.DT_UTil contains a few.

safe_builtins = {}

for name in ['False', 'None', 'True', 'abs', 'basestring', 'bool', 'callable',
             'chr', 'cmp', 'complex', 'divmod', 'float', 'hash',
             'hex', 'id', 'int', 'isinstance', 'issubclass', 'len',
             'long', 'oct', 'ord', 'pow', 'range', 'repr', 'round',
             'str', 'tuple', 'unichr', 'unicode', 'xrange', 'zip']:

    safe_builtins[name] = __builtins__[name]

# Wrappers provided by this module:
# delattr
# setattr

# Wrappers provided by ZopeGuards:
# __import__
# apply
# dict
# enumerate
# filter
# getattr
# hasattr
# iter
# list
# map
# max
# min
# sum

# Builtins that are intentionally disabled
# compile   - don't let them produce new code
# dir       - a general purpose introspector, probably hard to wrap
# execfile  - no direct I/O
# file      - no direct I/O
# globals   - uncontrolled namespace access
# input     - no direct I/O
# locals    - uncontrolled namespace access
# open      - no direct I/O
# raw_input - no direct I/O
# vars      - uncontrolled namespace access

# There are several strings that describe Python.  I think there's no
# point to including these, although they are obviously safe:
# copyright, credits, exit, help, license, quit

# Not provided anywhere.  Do something about these?  Several are
# related to new-style classes, which we are too scared of to support
# <0.3 wink>.  coerce, buffer, and reload are esoteric enough that no
# one should care.

# buffer
# classmethod
# coerce
# eval
# intern
# object
# property
# reload
# slice
# staticmethod
# super
# type

for name in dir(exceptions):
    if name[0] != "_":
        safe_builtins[name] = getattr(exceptions, name)

def _write_wrapper():
    # Construct the write wrapper class
    def _handler(secattr, error_msg):
        # Make a class method.
        def handler(self, *args):
            try:
                f = getattr(self.ob, secattr)
            except AttributeError:
                raise TypeError, error_msg
            f(*args)
        return handler
    class Wrapper:
        def __len__(self):
            # Required for slices with negative bounds.
            return len(self.ob)
        def __init__(self, ob):
            self.__dict__['ob'] = ob
        __setitem__ = _handler('__guarded_setitem__',
          'object does not support item or slice assignment')
        __delitem__ = _handler('__guarded_delitem__',
          'object does not support item or slice assignment')
        __setattr__ = _handler('__guarded_setattr__',
          'attribute-less object (assign or del)')
        __delattr__ = _handler('__guarded_delattr__',
          'attribute-less object (assign or del)')
    return Wrapper

def _full_write_guard():
    # Nested scope abuse!
    # safetype and Wrapper variables are used by guard()
    safetype = {dict: True, list: True}.has_key
    Wrapper = _write_wrapper()
    def guard(ob):
        # Don't bother wrapping simple types, or objects that claim to
        # handle their own write security.
        if safetype(type(ob)) or hasattr(ob, '_guarded_writes'):
            return ob
        # Hand the object to the Wrapper instance, then return the instance.
        return Wrapper(ob)
    return guard
full_write_guard = _full_write_guard()

def guarded_setattr(object, name, value):
    setattr(full_write_guard(object), name, value)
safe_builtins['setattr'] = guarded_setattr

def guarded_delattr(object, name):
    delattr(full_write_guard(object), name)
safe_builtins['delattr'] = guarded_delattr
