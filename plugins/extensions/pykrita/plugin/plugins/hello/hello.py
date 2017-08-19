"""
This is a simple example of a Python script for Krita.
It demonstrates how to set up a custom extension and a custom docker!
"""

from PyQt5.QtCore import qDebug
from PyQt5.QtWidgets import QWidget, QLabel, QMessageBox
from krita import Krita, Extension, DockWidget, DockWidgetFactory, DockWidgetFactoryBase


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
        super().__init__(parent)

    def setup(self):
        """
        This is where most of the setup takes place!
        """
        qDebug("Hello Setup")
        action = Krita.instance().createAction("hello_python", "hello")
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
        super().__init__()

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
        """
        self._label.setText("HelloDocker: canvas changed")


# Register the docker so Krita can use it!
Application.addDockWidgetFactory(DockWidgetFactory("hello", DockWidgetFactoryBase.DockRight, HelloDocker))
