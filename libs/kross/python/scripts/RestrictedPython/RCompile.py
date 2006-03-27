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
"""Compiles restricted code using the compiler module from the
Python standard library.
"""

__version__='$Revision: 1.6 $'[11:-2]

from compiler import ast, parse, misc, syntax, pycodegen
from compiler.pycodegen import AbstractCompileMode, Expression, \
     Interactive, Module, ModuleCodeGenerator, FunctionCodeGenerator, findOp

import MutatingWalker
from RestrictionMutator import RestrictionMutator


def niceParse(source, filename, mode):
    try:
        return parse(source, mode)
    except:
        # Try to make a clean error message using
        # the builtin Python compiler.
        try:
            compile(source, filename, mode)
        except SyntaxError:
            raise
        # Some other error occurred.
        raise

class RestrictedCompileMode(AbstractCompileMode):
    """Abstract base class for hooking up custom CodeGenerator."""
    # See concrete subclasses below.

    def __init__(self, source, filename):
        self.rm = RestrictionMutator()
        AbstractCompileMode.__init__(self, source, filename)

    def parse(self):
        return niceParse(self.source, self.filename, self.mode)

    def _get_tree(self):
        tree = self.parse()
        MutatingWalker.walk(tree, self.rm)
        if self.rm.errors:
            raise SyntaxError, self.rm.errors[0]
        misc.set_filename(self.filename, tree)
        syntax.check(tree)
        return tree

    def compile(self):
        tree = self._get_tree()
        gen = self.CodeGeneratorClass(tree)
        self.code = gen.getCode()


def compileAndTuplize(gen):
    try:
        gen.compile()
    except SyntaxError, v:
        return None, (str(v),), gen.rm.warnings, gen.rm.used_names
    return gen.getCode(), (), gen.rm.warnings, gen.rm.used_names

def compile_restricted_function(p, body, name, filename, globalize=None):
    """Compiles a restricted code object for a function.

    The function can be reconstituted using the 'new' module:

    new.function(<code>, <globals>)

    The globalize argument, if specified, is a list of variable names to be
    treated as globals (code is generated as if each name in the list
    appeared in a global statement at the top of the function).
    """
    gen = RFunction(p, body, name, filename, globalize)
    return compileAndTuplize(gen)

def compile_restricted_exec(s, filename='<string>'):
    """Compiles a restricted code suite."""
    gen = RModule(s, filename)
    return compileAndTuplize(gen)

def compile_restricted_eval(s, filename='<string>'):
    """Compiles a restricted expression."""
    gen = RExpression(s, filename)
    return compileAndTuplize(gen)

def compile_restricted(source, filename, mode):
    """Replacement for the builtin compile() function."""
    if mode == "single":
        gen = RInteractive(source, filename)
    elif mode == "exec":
        gen = RModule(source, filename)
    elif mode == "eval":
        gen = RExpression(source, filename)
    else:
        raise ValueError("compile_restricted() 3rd arg must be 'exec' or "
                         "'eval' or 'single'")
    gen.compile()
    return gen.getCode()

class RestrictedCodeGenerator:
    """Mixin for CodeGenerator to replace UNPACK_SEQUENCE bytecodes.

    The UNPACK_SEQUENCE opcode is not safe because it extracts
    elements from a sequence without using a safe iterator or
    making __getitem__ checks.

    This code generator replaces use of UNPACK_SEQUENCE with calls to
    a function that unpacks the sequence, performes the appropriate
    security checks, and returns a simple list.
    """

    # Replace the standard code generator for assignments to tuples
    # and lists.

    def _gen_safe_unpack_sequence(self, num):
        # We're at a place where UNPACK_SEQUENCE should be generated, to
        # unpack num items.  That's a security hole, since it exposes
        # individual items from an arbitrary iterable.  We don't remove
        # the UNPACK_SEQUENCE, but instead insert a call to our _getiter_()
        # wrapper first.  That applies security checks to each item as
        # it's delivered.  codegen is (just) a bit messy because the
        # iterable is already on the stack, so we have to do a stack swap
        # to get things in the right order.
        self.emit('LOAD_GLOBAL', '_getiter_')
        self.emit('ROT_TWO')
        self.emit('CALL_FUNCTION', 1)
        self.emit('UNPACK_SEQUENCE', num)

    def _visitAssSequence(self, node):
        if findOp(node) != 'OP_DELETE':
            self._gen_safe_unpack_sequence(len(node.nodes))
        for child in node.nodes:
            self.visit(child)

    visitAssTuple = _visitAssSequence
    visitAssList = _visitAssSequence

    # Call to generate code for unpacking nested tuple arguments
    # in function calls.

    def unpackSequence(self, tup):
        self._gen_safe_unpack_sequence(len(tup))
        for elt in tup:
            if isinstance(elt, tuple):
                self.unpackSequence(elt)
            else:
                self._nameOp('STORE', elt)

# A collection of code generators that adds the restricted mixin to
# handle unpacking for all the different compilation modes.  They
# are defined here (at the end) so that can refer to RestrictedCodeGenerator.

class RestrictedFunctionCodeGenerator(RestrictedCodeGenerator,
                                      pycodegen.FunctionCodeGenerator):
    pass

class RestrictedExpressionCodeGenerator(RestrictedCodeGenerator,
                                        pycodegen.ExpressionCodeGenerator):
    pass

class RestrictedInteractiveCodeGenerator(RestrictedCodeGenerator,
                                         pycodegen.InteractiveCodeGenerator):
    pass

class RestrictedModuleCodeGenerator(RestrictedCodeGenerator,
                                    pycodegen.ModuleCodeGenerator):

    def initClass(self):
        ModuleCodeGenerator.initClass(self)
        self.__class__.FunctionGen = RestrictedFunctionCodeGenerator


# These subclasses work around the definition of stub compile and mode
# attributes in the common base class AbstractCompileMode.  If it
# didn't define new attributes, then the stub code inherited via
# RestrictedCompileMode would override the real definitions in
# Expression.

class RExpression(RestrictedCompileMode, Expression):
    mode = "eval"
    CodeGeneratorClass = RestrictedExpressionCodeGenerator

class RInteractive(RestrictedCompileMode, Interactive):
    mode = "single"
    CodeGeneratorClass = RestrictedInteractiveCodeGenerator

class RModule(RestrictedCompileMode, Module):
    mode = "exec"
    CodeGeneratorClass = RestrictedModuleCodeGenerator

class RFunction(RModule):
    """A restricted Python function built from parts."""

    CodeGeneratorClass = RestrictedModuleCodeGenerator

    def __init__(self, p, body, name, filename, globals):
        self.params = p
        self.body = body
        self.name = name
        self.globals = globals or []
        RModule.__init__(self, None, filename)

    def parse(self):
        # Parse the parameters and body, then combine them.
        firstline = 'def f(%s): pass' % self.params
        tree = niceParse(firstline, '<function parameters>', 'exec')
        f = tree.node.nodes[0]
        body_code = niceParse(self.body, self.filename, 'exec')
        # Stitch the body code into the function.
        f.code.nodes = body_code.node.nodes
        f.name = self.name
        # Look for a docstring.
        stmt1 = f.code.nodes[0]
        if (isinstance(stmt1, ast.Discard) and
            isinstance(stmt1.expr, ast.Const) and
            isinstance(stmt1.expr.value, str)):
            f.doc = stmt1.expr.value
        # The caller may specify that certain variables are globals
        # so that they can be referenced before a local assignment.
        # The only known example is the variables context, container,
        # script, traverse_subpath in PythonScripts.
        if self.globals:
            f.code.nodes.insert(0, ast.Global(self.globals))
        return tree
