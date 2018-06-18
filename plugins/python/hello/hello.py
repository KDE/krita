'''
This script is licensed CC 0 1.0, so that you can learn from it.

------ CC 0 1.0 ---------------

The person who associated a work with this deed has dedicated the work to the public domain by waiving all of his or her rights to the work worldwide under copyright law, including all related and neighboring rights, to the extent allowed by law.

You can copy, modify, distribute and perform the work, even for commercial purposes, all without asking permission.

https://creativecommons.org/publicdomain/zero/1.0/legalcode
'''
#
# This is a simple example of a Python script for Krita.
# It demonstrates how to set up a custom extension and a custom docker!
#

from PyQt5.QtWidgets import QWidget, QLabel, QMessageBox
from krita import (Krita, Extension, DockWidget,
                   DockWidgetFactory, DockWidgetFactoryBase)


def hello():
    """
    Show a test message box.
    """
    QMessageBox.information(QWidget(), i18n("Test"), i18n("Hello! This is Krita version %s") % Application.version())


class HelloExtension(Extension):
    """
    HelloExtension is a small example extension demonstrating basic Python scripting support in Krita!
    """

    def __init__(self, parent):
        """
        Standard Krita Python extension constructor.
        Most of the initialization happens in :func:`setup`

        :param parent: Parent widget
        :type parent: :class:`QWidget` or None
        """
        super(HelloExtension, self).__init__(parent)

    def setup(self):
        pass

    def createActions(self, window):
        """
        This is where most of the setup takes place!
        """
        action = window.createAction("hello_python", i18n("Hello"))
        action.triggered.connect(hello)


# Initialize and add the extension
Scripter.addExtension(HelloExtension(Krita.instance()))


class HelloDocker(DockWidget):
    """
    The HelloDocker is an example of a simple Python-based docker.
    """

    def __init__(self):
        """
        Constructs an instance of HelloDocker and the widget it contains
        """
        super(HelloDocker, self).__init__()

        # The window title is also used in the Docker menu,
        # so it should be set to something sensible!
        self.setWindowTitle("HelloDocker")
        label = QLabel("Hello", self)
        self.setWidget(label)
        self._label = label

    def canvasChanged(self, canvas):
        """
        Override canvasChanged from :class:`DockWidget`.
        This gets called when the canvas changes.
        You can also access the active canvas via :func:`DockWidget.canvas`
        Parameter `canvas` can be null if the last document is closed
        """
        self._label.setText("HelloDocker: canvas changed")


# Register the docker so Krita can use it!
Application.addDockWidgetFactory(DockWidgetFactory("hello", DockWidgetFactoryBase.DockRight, HelloDocker))
