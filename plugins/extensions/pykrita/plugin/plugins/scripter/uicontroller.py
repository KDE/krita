from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from scripter.ui_scripter.syntax import syntax, syntaxstyles
from scripter.ui_scripter import actions
from scripter.ui_scripter.editor import pythoneditor
from scripter.loaders import actionloader, menuloader, widgetloader
import os
import importlib
import inspect

class UIController(object):
    def __init__(self, mainWidget):
        self.mainWidget = mainWidget
        self.actionToolbar = QToolBar('toolBar', self.mainWidget)
        self.menu_bar = QMenuBar(self.mainWidget)

        self.actionToolbar.setObjectName('toolBar')
        self.menu_bar.setObjectName('menuBar')

        self.components = []

        self.mainWidget.setWindowModality(Qt.NonModal)
        self.editor = pythoneditor.CodeEditor()
        self.output = QPlainTextEdit()
        self.highlight = syntax.PythonHighlighter(self.editor.document(), syntaxstyles.DefaultSyntaxStyle())

    def initialize(self, scripter):
        self.scripter = scripter

        self.loadMenus()
        self.loadComponents(actionloader.ActionLoader())

        vbox = QVBoxLayout(self.mainWidget)
        vbox.addWidget(self.menu_bar)
        vbox.addWidget(self.editor)
        vbox.addWidget(self.actionToolbar)
        vbox.addWidget(self.output)

        self.mainWidget.resize(400, 500)
        self.mainWidget.setWindowTitle("Scripter")
        self.mainWidget.setSizeGripEnabled(True)
        self.mainWidget.show()
        self.mainWidget.activateWindow()

    def loadMenus(self):
        self.addMenu('File', 'File')
        self.addMenu('Edit', 'Edit')

    def addMenu(self, menuName, parentName):
        parent = self.menu_bar.findChild(QObject, parentName)
        self.newMenu = None

        if parent:
            self.newMenu = parent.addMenu(menuName)
        else:
            self.newMenu = self.menu_bar.addMenu(menuName)

        self.newMenu.setObjectName(menuName)

        return self.newMenu


    def loadComponents(self, loader):
        component_module = importlib.import_module(loader.importPath)

        modules = [loader.importPath + '.' + str(module).split('.')[0]
                    for module in os.listdir(component_module.__path__[0])
                    if module.endswith('.py') and (not '__init__' in module)]

        for module in modules:
            m = importlib.import_module(module)
            clsmembers = [klass for klass in inspect.getmembers(m, inspect.isclass) if klass[1].__module__ == module]
            print(self.mainWidget.children())
            for klass in clsmembers:
                obj = klass[1](self.scripter)
                parent = self.mainWidget.findChild(QObject, obj.parent)
                print('parent:', parent)
                self.components.append(dict(component=obj, parent=parent))

        loader.addComponents(self.components)
