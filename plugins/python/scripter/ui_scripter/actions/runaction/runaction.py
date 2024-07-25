"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
from PyQt5.QtWidgets import QAction
from PyQt5.QtGui import QIcon, QKeySequence
from PyQt5.QtCore import Qt
import sys
import traceback
import inspect
from . import docwrapper
from .... import utils
import krita

if sys.version_info[0] > 2:
    import importlib
    from importlib.machinery import SourceFileLoader
else:
    import imp

PYTHON27 = sys.version_info.major == 2 and sys.version_info.minor == 7
PYTHON33 = sys.version_info.major == 3 and sys.version_info.minor == 3
PYTHON34 = sys.version_info.major == 3 and sys.version_info.minor == 4
EXEC_NAMESPACE = "__main__"  # namespace that user scripts will run in


class RunAction(QAction):

    def __init__(self, scripter, parent=None):
        super(RunAction, self).__init__(parent)
        self.scripter = scripter

        self.editor = self.scripter.uicontroller.editor
        self.output = self.scripter.uicontroller.findTabWidget(i18n('Output'), 'OutPutTextEdit')

        self.triggered.connect(self.run)

        self.setText(i18n("Run"))
        self.setToolTip(i18n('Run Ctrl+R'))
        self.setShortcut(QKeySequence(Qt.CTRL + Qt.Key_R))
        self.setIcon(utils.getThemedIcon(':/icons/run.svg'))

    @property
    def parent(self):
        return 'toolBar',

    def run(self):
        """ This method execute python code from an activeDocument (file) or direct
            from editor (ui_scripter/editor/pythoneditor.py). When executing code
            from a file, we use importlib to load this module/file and with
            "users_script" name. That's implementation seeks for a "main()" function in the script.
            When executing code from editor without creating a file, we compile
            this script to bytecode and we execute this in an empty scope. That's
            faster than use exec directly and cleaner, because we are using an empty scope. """

        self.scripter.uicontroller.setActiveWidget(i18n('Output'))
        stdout = sys.stdout
        stderr = sys.stderr
        output = docwrapper.DocWrapper(self.output.document())
        if (self.editor._documentModified is True):
            output.write("==== Warning: Script not saved! ====\n")
        else:
            output.write("======================================\n")
        sys.stdout = output
        sys.stderr = output

        script = self.editor.document().toPlainText()
        document = self.scripter.documentcontroller.activeDocument

        try:
            if document and self.editor._documentModified is False:
                if PYTHON27:
                    users_module = self.run_py2_document(document)
                else:
                    users_module = self.run_py3_document(document)

                # maybe script is to be execed, maybe main needs to be invoked
                # if there is a main() then execute it, otherwise don't worry...
                if hasattr(users_module, "main") and inspect.isfunction(users_module.main):
                    users_module.main()
            else:
                code = compile(script, '<string>', 'exec')
                globals_dict = {"__name__": EXEC_NAMESPACE}
                exec(code, globals_dict)

        except SystemExit:
            # user typed quit() or exit()
            self.scripter.uicontroller.closeScripter()
        except Exception:
            # Provide context (line number and text) for an error that is caught.
            # Ordinarily, syntax and Indent errors are caught during initial
            # compilation in exec(), and the traceback traces back to this file.
            # So these need to be treated separately.
            # Other errors trace back to the file/script being run.
            type_, value_, traceback_ = sys.exc_info()
            if type_ == SyntaxError:
                errorMessage = "%s\n%s" % (value_.text.rstrip(), " " * (value_.offset - 1) + "^")
                # rstrip to remove trailing \n, output needs to be fixed width font for the ^ to align correctly
                errorText = "Syntax Error on line %s" % value_.lineno
            elif type_ == IndentationError:
                # (no offset is provided for an IndentationError
                errorMessage = value_.text.rstrip()
                errorText = "Unexpected Indent on line %s" % value_.lineno
            else:
                errorText = traceback.format_exception_only(type_, value_)[0]
                format_string = "In file: {0}\nIn function: {2} at line: {1}. Line with error:\n{3}"
                tbList = traceback.extract_tb(traceback_)
                tb = tbList[-1]
                errorMessage = format_string.format(*tb)
            m = "\n**********************\n%s\n%s\n**********************\n" % (errorText, errorMessage)
            output.write(m)

        sys.stdout = stdout
        sys.stderr = stderr

        # scroll to bottom of output
        bottom = self.output.verticalScrollBar().maximum()
        self.output.verticalScrollBar().setValue(bottom)

    def run_py2_document(self, document):
        """ Loads and executes an external script using Python 2 specific operations
        and returns the loaded module for further execution if needed.
        """
        try:
            user_module = imp.load_source(EXEC_NAMESPACE, document.filePath)
        except Exception as e:
            raise e

        return user_module

    def run_py3_document(self, document):
        """ Loads and executes an external script using Python 3 specific operations
        and returns the loaded module for further execution if needed.
        """
        spec = importlib.util.spec_from_file_location(EXEC_NAMESPACE, document.filePath)
        try:
            users_module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(users_module)

        except AttributeError as e:  # no module from spec
            if PYTHON34 or PYTHON33:
                loader = SourceFileLoader(EXEC_NAMESPACE, document.filePath)
                users_module = loader.load_module()
            else:
                raise e

        return users_module
