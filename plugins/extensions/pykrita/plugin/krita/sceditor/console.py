#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

from __future__ import print_function

import sys
import traceback
import re

from PyQt5.QtCore import QObject, Qt
from PyQt5.QtGui import QTextCursor
from PyQt5.QtWidgets import qApp, QApplication, QPlainTextEdit


from highlighter import PythonHighlighter,  QtQmlHighlighter


from PyQt5.QtQml import (
    QScriptEngine, QScriptValue, QScriptValueIterator)


class OutputWidget(QPlainTextEdit):

    def __init__(self, parent=None, readonly=True, max_rows=1000, echo=True):
        QPlainTextEdit.__init__(self, parent)
        self.echo = echo
        self.setReadOnly(readonly)
        self.document().setMaximumBlockCount(max_rows)
        self.attach()

    def attach(self):
        sys.stdout = sys.stderr = self

    def __del__(self):
        self.detach()

    def detach(self):
        sys.stdout = sys.__stdout__
        sys.stderr = sys.__stderr__

    def write(self, s):
        if self.echo:
            sys.__stdout__.write(s)
        doc = self.document()
        cursor = QTextCursor(doc)
        cursor.clearSelection()
        cursor.movePosition(QTextCursor.End, QTextCursor.MoveAnchor)
        cursor.insertText(s)
        cursor.movePosition(QTextCursor.End, QTextCursor.MoveAnchor)
        cursor.clearSelection()
        self.ensureCursorVisible()
        qApp.processEvents()

    def writelines(self, lines):
        self.write("\n".join(lines))


class ConsoleWidget(OutputWidget):

    def __init__(self, parent=None, ps1="?", ps2=">"):
        OutputWidget.__init__(self, parent, readonly=False)
        self.setTabChangesFocus(False)
        self.ps1 = ps1
        self.ps2 = ps2
        self.history_index = 0
        self.history = [""]
        self.tab_state = -1
        print(self.ps1, end='')

    def focusInEvent(self, event):
        self.attach()
        OutputWidget.focusInEvent(self, event)

    def mousePressEvent(self, event):
        self.setFocus()

    def push(self, line):
        return True

    def keyPressEvent(self, event):
        def remove_line():
            cursor = self.textCursor()
            cursor.select(QTextCursor.BlockUnderCursor)
            cursor.removeSelectedText()
        key = event.key()
        modifiers = event.modifiers()
        l = len(self.ps1)
        line = unicode(self.document().end().previous().text())
        ps1orps2, line = line[:l - 1], line[l:]

        if not key in [Qt.Key_Tab, Qt.Key_Backtab] and \
                len(event.text()):
            self.tab_state = -1
        if key == Qt.Key_Up:
            if self.history_index + 1 < len(self.history):
                self.history_index += 1
            remove_line()
            print()
            print(ps1orps2, self.history[self.history_index], end='')
        elif key == Qt.Key_Down:
            if self.history_index > 0:
                self.history_index -= 1
            remove_line()
            print()
            print(ps1orps2, self.history[self.history_index], end='')
        elif key == Qt.Key_Tab:
            if modifiers & Qt.ControlModifier:
                print(" " * 4, end='')
            else:
                self.tab_state += 1
                remove_line()
                print()
                print(ps1orps2, end='')
                print(self.completer.complete(line, self.tab_state) or line, end='')
        elif key == Qt.Key_Backtab:
            if self.tab_state >= 0:
                self.tab_state -= 1
            remove_line()
            print()
            print(ps1orps2, end='')
            print(self.completer.complete(line, self.tab_state) or line, end='')
        elif key in [Qt.Key_Backspace, Qt.Key_Left]:
            if self.textCursor().columnNumber() > len(ps1orps2) + 1:
                return OutputWidget.keyPressEvent(self, event)
        elif key == Qt.Key_Return:
            self.moveCursor(QTextCursor.EndOfLine,  QTextCursor.MoveAnchor)
            print()
            if self.push(line):
                print(self.ps2, end='')
            else:
                print(self.ps1, end='')
            if line and line != self.history[self.history_index]:
                self.history.insert(1, line)
            self.history_index = 0
        else:
            return OutputWidget.keyPressEvent(self, event)


class PythonInterpreter(object):

    def __init__(self, name="<pyqtshell>", locals=None):
        self.name = name
        self.locals = locals or {}
        self.locals["__name__"] = self.name
        self.lines = []

    def run(self, source, locals=None):
        if locals == None:
            locals = self.locals
        code = compile(source, self.name, "exec")
        try:
            exec(code, locals)
        except:
            self.showtraceback()
        try:
            Scripter.activeWindow.redraw = True
            Scripter.activeWindow.update()
        except:
            pass

    def push(self, line):
        if self.lines:
            if line:
                self.lines.append(line)
                return 1  # want more!
            else:
                line = "\n".join(self.lines) + "\n"
        else:
            if not line:
                return 0
        try:
            code = compile(line, self.name, "single")
            self.lines = []
        except SyntaxError as why:
            if why[0] == "unexpected EOF while parsing":
                self.lines.append(line)
                return 1  # want more!
            else:
                self.showtraceback()
        except:
            self.showtraceback()
        else:
            try:
                exec(code, self.locals)
            except:
                self.showtraceback()
            try:
                Scripter.activeWindow.redraw = True
                Scripter.activeWindow.update()
            except:
                pass
        return 0

    def showtraceback(self):
        self.lines = []
        if sys.exc_info()[0] == SyntaxError:  # and len(sys.exc_value) == 2:
            print("  File \"%s\", line %d" % (self.name, sys.exc_value[1][1]))
            print(" " * (sys.exc_value[1][2] + 2) + "^")
            print(str(sys.exc_info()[0]) + ":", sys.exc_value[0])
        else:
            traceback.print_tb(sys.exc_info()[2], None)
            print(sys.exc_type.__name__ + ":", sys.exc_info()[1])


class PythonCompleter(object):

    def __init__(self, namespace):
        self.namespace = namespace

    def complete(self, text, state):
        if state == 0:
            if "." in text:
                self.matches = self.attr_matches(text)
            else:
                self.matches = self.global_matches(text)
        try:
            return self.matches[state]
        except IndexError:
            return None

    def global_matches(self, text):
        import keyword
        import __builtin__
        matches = []
        n = len(text)
        for list in [keyword.kwlist,
                     __builtin__.__dict__,
                     self.namespace]:
            for word in list:
                if word[:n] == text and word != "__builtins__":
                    matches.append(word)
        return matches

    def attr_matches(self, text):
        def get_class_members(cls):
            ret = dir(cls)
            if hasattr(cls, '__bases__'):
                for base in cls.__bases__:
                    ret = ret + get_class_members(base)
            return ret
        import re
        m = re.match(r"(\w+(\.\w+)*)\.(\w*)", text)
        if not m:
            return
        expr, attr = m.group(1, 3)
        object = eval(expr, self.namespace)
        words = dir(object)
        if hasattr(object, '__class__'):
            words.append('__class__')
            words = words + get_class_members(object.__class__)
        matches = []
        n = len(attr)
        for word in words:
            if word[:n] == attr and word != "__builtins__":
                matches.append("%s.%s" % (expr, word))
        return matches


class PythonConsole(ConsoleWidget):

    def __init__(self, parent=None, namespace=None):
        ConsoleWidget.__init__(self, parent, ps1=">>> ", ps2="... ")
        self.highlighter = PythonHighlighter(self)
        self.inter = PythonInterpreter(locals=namespace)
        self.namespace = self.inter.locals
        self.completer = PythonCompleter(self.namespace)
        # print("Python", sys.version)
        # print("Autocomplete with (Shift+)Tab, insert spaces with Ctrl+Tab")
        self.push("pass")

    def push(self, line):
        return self.inter.push(line)

    def clear(self):
        doc = self.document()
        doc.setPlainText(self.ps1)


class QtQmlInterpreter(object):

    def __init__(self, locals):
        self.locals = locals
        self.engine = self.newEngine()
        self.code = ""
        self.state = 0

    def newEngine(self):
        engine = QScriptEngine()
        ns = engine.globalObject()
        for name, value in self.locals.items():
            if isinstance(value, QObject):
                value = engine.newQObject(value)
            elif callable(value):
                value = engine.newFunction(value)
            ns.setProperty(name, value)
        return engine

    def execute(self, code):
        self.execute_code(code, self.engine)

    def execute_code(self, code, engine=None):
        engine = engine or self.newEngine()
        result = engine.evaluate(code)
        try:
            Scripter.activeWindow.redraw = True
            Scripter.activeWindow.update()
        except:
            pass
        if engine.hasUncaughtException():
            bt = engine.uncaughtExceptionBacktrace()
            print("Traceback:")
            print("\n".join(["  %s" % l for l in list(bt)]))
            print(engine.uncaughtException().toString())
        else:
            if not result.isUndefined():
                print(result.toString())

    def push(self, line):
        if not line.strip():
            return self.state
        self.code = self.code + line + "\n"
        if self.engine.canEvaluate(self.code):
            self.execute(self.code)
            self.code = ""
            self.state = 0
        else:
            self.state = 1
        return self.state


js_words = [
    'break',
    'for',
    'throw',
    'case',
    'function',
    'try',
    'catch',
    'if',
    'typeof',
    'continue',
    'in',
    'var',
    'default',
    'instanceof',
    'void',
    'delete',
    'new',
    'undefined',
    'do',
    'return',
    'while',
    'else',
    'switch',
    'with',
    'finally',
    'this',
    'NaN',
    'Infinity',
    'undefined',
    'print',
    'parseInt',
    'parseFloat',
    'isNaN',
    'isFinite',
    'decodeURI',
    'decodeURIComponent',
    'encodeURI',
    'encodeURIComponent',
    'escape',
    'unescape',
    'version',
    'gc',
    'Object',
    'Function',
    'Number',
    'Boolean',
    'String',
    'Date',
    'Array',
    'RegExp',
    'Error',
    'EvalError',
    'RangeError',
    'ReferenceError',
    'SyntaxError',
    'TypeError',
    'URIError',
    'eval',
    'Math',
    'Enumeration',
    'Variant',
    'QObject',
    'QMetaObject']


class QtQmlCompleter(object):

    def __init__(self, engine):
        self.engine = engine

    def complete(self, text, state):
        if state == 0:
            if "." in text:
                self.matches = self.attr_matches(text)
            else:
                self.matches = self.global_matches(text)
        try:
            return self.matches[state]
        except IndexError:
            return None

    def attr_matches(self, text):
        return []

    def iter_obj(self, obj):
        it = QScriptValueIterator(self.engine.globalObject())
        while it.hasNext():
            yield str(it.name())
            it.next()

    def global_matches(self, text):
        words = list(self.iter_obj(self.engine.globalObject()))
        words.extend(js_words)
        l = []
        n = len(text)
        for w in words:
            if w[:n] == text:
                l.append(w)
        return l


class QtQmlConsole(ConsoleWidget):

    def __init__(self, parent=None, namespace=None):
        ConsoleWidget.__init__(self, parent, ps1=">>> ", ps2="... ")
        self.highlighter = QtQmlHighlighter(self)
        namespace = namespace or {}

        def console_print(context, engine):
            for i in range(context.argumentCount()):
                print(context.argument(i).toString(), end='')
            print()
            return QScriptValue()

        def dir_context(context, engine):
            if context.argumentCount() == 0:
                obj = context.thisObject()
            else:
                obj = context.argument(0)
            l = []
            it = QScriptValueIterator(obj)
            while it.hasNext():
                it.next()
                l.append(str(it.name()))
            return QScriptValue(engine, repr(l))
        namespace["print"] = console_print
        namespace["dir"] = dir_context
        namespace["Application"] = qApp
        try:
            namespace["Scripter"] = Scripter.qt
        except:
            pass
        self.inter = QtQmlInterpreter(namespace)
        self.completer = QtQmlCompleter(self.inter.engine)

    def push(self, line):
        return self.inter.push(line)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    o = QtQmlConsole()
    # o = PythonConsole()
    o.resize(640, 480)
    o.attach()
    o.show()
    app.exec_()
