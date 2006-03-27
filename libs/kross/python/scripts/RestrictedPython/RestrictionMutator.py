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
"""Modify AST to include security checks.

RestrictionMutator modifies a tree produced by
compiler.transformer.Transformer, restricting and enhancing the
code in various ways before sending it to pycodegen.

$Revision: 1.13 $
"""

from SelectCompiler import ast, parse, OP_ASSIGN, OP_DELETE, OP_APPLY

# These utility functions allow us to generate AST subtrees without
# line number attributes.  These trees can then be inserted into other
# trees without affecting line numbers shown in tracebacks, etc.
def rmLineno(node):
    """Strip lineno attributes from a code tree."""
    if node.__dict__.has_key('lineno'):
        del node.lineno
    for child in node.getChildren():
        if isinstance(child, ast.Node):
            rmLineno(child)

def stmtNode(txt):
    """Make a "clean" statement node."""
    node = parse(txt).node.nodes[0]
    rmLineno(node)
    return node

# The security checks are performed by a set of six functions that
# must be provided by the restricted environment.

_apply_name = ast.Name("_apply_")
_getattr_name = ast.Name("_getattr_")
_getitem_name = ast.Name("_getitem_")
_getiter_name = ast.Name("_getiter_")
_print_target_name = ast.Name("_print")
_write_name = ast.Name("_write_")

# Constants.
_None_const = ast.Const(None)
_write_const = ast.Const("write")

_printed_expr = stmtNode("_print()").expr
_print_target_node = stmtNode("_print = _print_()")

class FuncInfo:
    print_used = False
    printed_used = False

class RestrictionMutator:

    def __init__(self):
        self.warnings = []
        self.errors = []
        self.used_names = {}
        self.funcinfo = FuncInfo()

    def error(self, node, info):
        """Records a security error discovered during compilation."""
        lineno = getattr(node, 'lineno', None)
        if lineno is not None and lineno > 0:
            self.errors.append('Line %d: %s' % (lineno, info))
        else:
            self.errors.append(info)

    def checkName(self, node, name):
        """Verifies that a name being assigned is safe.

        This is to prevent people from doing things like:

          __metatype__ = mytype (opens up metaclasses, a big unknown
                                 in terms of security)
          __path__ = foo        (could this confuse the import machinery?)
          _getattr = somefunc   (not very useful, but could open a hole)

        Note that assigning a variable is not the only way to assign
        a name.  def _badname, class _badname, import foo as _badname,
        and perhaps other statements assign names.  Special case:
        '_' is allowed.
        """
        if name.startswith("_") and name != "_":
            # Note: "_" *is* allowed.
            self.error(node, '"%s" is an invalid variable name because'
                       ' it starts with "_"' % name)
        if name == "printed":
            self.error(node, '"printed" is a reserved name.')

    def checkAttrName(self, node):
        """Verifies that an attribute name does not start with _.

        As long as guards (security proxies) have underscored names,
        this underscore protection is important regardless of the
        security policy.  Special case: '_' is allowed.
        """
        name = node.attrname
        if name.startswith("_") and name != "_":
            # Note: "_" *is* allowed.
            self.error(node, '"%s" is an invalid attribute name '
                       'because it starts with "_".' % name)

    def prepBody(self, body):
        """Insert code for print at the beginning of the code suite."""

        if self.funcinfo.print_used or self.funcinfo.printed_used:
            # Add code at top for creating _print_target
            body.insert(0, _print_target_node)
            if not self.funcinfo.printed_used:
                self.warnings.append(
                    "Prints, but never reads 'printed' variable.")
            elif not self.funcinfo.print_used:
                self.warnings.append(
                    "Doesn't print, but reads 'printed' variable.")

    def visitFunction(self, node, walker):
        """Checks and mutates a function definition.

        Checks the name of the function and the argument names using
        checkName().  It also calls prepBody() to prepend code to the
        beginning of the code suite.
        """
        self.checkName(node, node.name)
        for argname in node.argnames:
            if isinstance(argname, str):
                self.checkName(node, argname)
            else:
                for name in argname:
                    self.checkName(node, name)
        walker.visitSequence(node.defaults)

        former_funcinfo = self.funcinfo
        self.funcinfo = FuncInfo()
        node = walker.defaultVisitNode(node, exclude=('defaults',))
        self.prepBody(node.code.nodes)
        self.funcinfo = former_funcinfo
        return node

    def visitLambda(self, node, walker):
        """Checks and mutates an anonymous function definition.

        Checks the argument names using checkName().  It also calls
        prepBody() to prepend code to the beginning of the code suite.
        """
        for argname in node.argnames:
            self.checkName(node, argname)
        return walker.defaultVisitNode(node)

    def visitPrint(self, node, walker):
        """Checks and mutates a print statement.

        Adds a target to all print statements.  'print foo' becomes
        'print >> _print, foo', where _print is the default print
        target defined for this scope.

        Alternatively, if the untrusted code provides its own target,
        we have to check the 'write' method of the target.
        'print >> ob, foo' becomes
        'print >> (_getattr(ob, 'write') and ob), foo'.
        Otherwise, it would be possible to call the write method of
        templates and scripts; 'write' happens to be the name of the
        method that changes them.
        """
        node = walker.defaultVisitNode(node)
        self.funcinfo.print_used = True
        if node.dest is None:
            node.dest = _print_target_name
        else:
            # Pre-validate access to the "write" attribute.
            # "print >> ob, x" becomes
            # "print >> (_getattr(ob, 'write') and ob), x"
            node.dest = ast.And([
                ast.CallFunc(_getattr_name, [node.dest, _write_const]),
                node.dest])
        return node

    visitPrintnl = visitPrint

    def visitName(self, node, walker):
        """Prevents access to protected names as defined by checkName().

        Also converts use of the name 'printed' to an expression.
        """
        if node.name == 'printed':
            # Replace name lookup with an expression.
            self.funcinfo.printed_used = True
            return _printed_expr
        self.checkName(node, node.name)
        self.used_names[node.name] = True
        return node

    def visitCallFunc(self, node, walker):
        """Checks calls with *-args and **-args.

        That's a way of spelling apply(), and needs to use our safe
        _apply_ instead.
        """
        walked = walker.defaultVisitNode(node)
        if node.star_args is None and node.dstar_args is None:
            # This is not an extended function call
            return walked
        # Otherwise transform foo(a, b, c, d=e, f=g, *args, **kws) into a call
        # of _apply_(foo, a, b, c, d=e, f=g, *args, **kws).  The interesting
        # thing here is that _apply_() is defined with just *args and **kws,
        # so it gets Python to collapse all the myriad ways to call functions
        # into one manageable form.
        #
        # From there, _apply_() digs out the first argument of *args (it's the
        # function to call), wraps args and kws in guarded accessors, then
        # calls the function, returning the value.
        # Transform foo(...) to _apply(foo, ...)
        walked.args.insert(0, walked.node)
        walked.node = _apply_name
        return walked

    def visitAssName(self, node, walker):
        """Checks a name assignment using checkName()."""
        self.checkName(node, node.name)
        return node

    def visitFor(self, node, walker):
        # convert
        #   for x in expr:
        # to
        #   for x in _getiter(expr):
        #
        # Note that visitListCompFor is the same thing.  Exactly the same
        # transformation is needed to convert
        #   [... for x in expr ...]
        # to
        #   [... for x in _getiter(expr) ...]
        node = walker.defaultVisitNode(node)
        node.list = ast.CallFunc(_getiter_name, [node.list])
        return node

    visitListCompFor = visitFor

    def visitGetattr(self, node, walker):
        """Converts attribute access to a function call.

        'foo.bar' becomes '_getattr(foo, "bar")'.

        Also prevents augmented assignment of attributes, which would
        be difficult to support correctly.
        """
        self.checkAttrName(node)
        node = walker.defaultVisitNode(node)
        if getattr(node, 'in_aug_assign', False):
            # We're in an augmented assignment
            # We might support this later...
            self.error(node, 'Augmented assignment of '
                       'attributes is not allowed.')
        return ast.CallFunc(_getattr_name,
                            [node.expr, ast.Const(node.attrname)])

    def visitSubscript(self, node, walker):
        """Checks all kinds of subscripts.

        'foo[bar] += baz' is disallowed.
        'a = foo[bar, baz]' becomes 'a = _getitem(foo, (bar, baz))'.
        'a = foo[bar]' becomes 'a = _getitem(foo, bar)'.
        'a = foo[bar:baz]' becomes 'a = _getitem(foo, slice(bar, baz))'.
        'a = foo[:baz]' becomes 'a = _getitem(foo, slice(None, baz))'.
        'a = foo[bar:]' becomes 'a = _getitem(foo, slice(bar, None))'.
        'del foo[bar]' becomes 'del _write(foo)[bar]'.
        'foo[bar] = a' becomes '_write(foo)[bar] = a'.

        The _write function returns a security proxy.
        """
        node = walker.defaultVisitNode(node)
        if node.flags == OP_APPLY:
            # Set 'subs' to the node that represents the subscript or slice.
            if getattr(node, 'in_aug_assign', False):
                # We're in an augmented assignment
                # We might support this later...
                self.error(node, 'Augmented assignment of '
                           'object items and slices is not allowed.')
            if hasattr(node, 'subs'):
                # Subscript.
                subs = node.subs
                if len(subs) > 1:
                    # example: ob[1,2]
                    subs = ast.Tuple(subs)
                else:
                    # example: ob[1]
                    subs = subs[0]
            else:
                # Slice.
                # example: obj[0:2]
                lower = node.lower
                if lower is None:
                    lower = _None_const
                upper = node.upper
                if upper is None:
                    upper = _None_const
                subs = ast.Sliceobj([lower, upper])
            return ast.CallFunc(_getitem_name, [node.expr, subs])
        elif node.flags in (OP_DELETE, OP_ASSIGN):
            # set or remove subscript or slice
            node.expr = ast.CallFunc(_write_name, [node.expr])
        return node

    visitSlice = visitSubscript

    def visitAssAttr(self, node, walker):
        """Checks and mutates attribute assignment.

        'a.b = c' becomes '_write(a).b = c'.
        The _write function returns a security proxy.
        """
        self.checkAttrName(node)
        node = walker.defaultVisitNode(node)
        node.expr = ast.CallFunc(_write_name, [node.expr])
        return node

    def visitExec(self, node, walker):
        self.error(node, 'Exec statements are not allowed.')

    def visitYield(self, node, walker):
        self.error(node, 'Yield statements are not allowed.')

    def visitClass(self, node, walker):
        """Checks the name of a class using checkName().

        Should classes be allowed at all?  They don't cause security
        issues, but they aren't very useful either since untrusted
        code can't assign instance attributes.
        """
        self.checkName(node, node.name)
        return walker.defaultVisitNode(node)

    def visitModule(self, node, walker):
        """Adds prep code at module scope.

        Zope doesn't make use of this.  The body of Python scripts is
        always at function scope.
        """
        node = walker.defaultVisitNode(node)
        self.prepBody(node.node.nodes)
        return node

    def visitAugAssign(self, node, walker):
        """Makes a note that augmented assignment is in use.

        Note that although augmented assignment of attributes and
        subscripts is disallowed, augmented assignment of names (such
        as 'n += 1') is allowed.

        This could be a problem if untrusted code got access to a
        mutable database object that supports augmented assignment.
        """
        node.node.in_aug_assign = True
        return walker.defaultVisitNode(node)

    def visitImport(self, node, walker):
        """Checks names imported using checkName()."""
        for name, asname in node.names:
            self.checkName(node, name)
            if asname:
                self.checkName(node, asname)
        return node

