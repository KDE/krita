from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from scripter.ui_scripter.syntax import syntax, syntaxstyles
from scripter.ui_scripter.editor import pythoneditor
import os
import importlib


class UIController(object):

    def __init__(self, mainWidget):
        self.mainWidget = mainWidget
        self.actionToolbar = QToolBar('toolBar', self.mainWidget)
        self.menu_bar = QMenuBar(self.mainWidget)

        self.actionToolbar.setObjectName('toolBar')
        self.menu_bar.setObjectName('menuBar')

        self.actions = []

        self.mainWidget.setWindowModality(Qt.NonModal)

    def initialize(self, scripter):
        self.editor = pythoneditor.CodeEditor(scripter)
        self.tabWidget = QTabWidget()
        self.statusBar = QLabel('untitled')
        self.highlight = syntax.PythonHighlighter(self.editor.document(), syntaxstyles.DefaultSyntaxStyle())

        self.scripter = scripter

        self.loadMenus()
        self.loadWidgets()
        self.loadActions()

        vbox = QVBoxLayout(self.mainWidget)
        vbox.addWidget(self.menu_bar)
        vbox.addWidget(self.actionToolbar)
        vbox.addWidget(self.editor)
        vbox.addWidget(self.tabWidget)
        vbox.addWidget(self.statusBar)

        self.mainWidget.resize(400, 500)
        self.mainWidget.setWindowTitle("Scripter")
        self.mainWidget.setSizeGripEnabled(True)
        self.addMenu('Edit', 'Edit')
        self.mainWidget.show()
        self.mainWidget.activateWindow()

    def loadMenus(self):
        self.addMenu('File', 'File')

    def addMenu(self, menuName, parentName):
        parent = self.menu_bar.findChild(QObject, parentName)
        self.newMenu = None

        if parent:
            self.newMenu = parent.addMenu(menuName)
        else:
            self.newMenu = self.menu_bar.addMenu(menuName)

        self.newMenu.setObjectName(menuName)

        return self.newMenu


    def loadActions(self):
        module_path = 'scripter.ui_scripter.actions'
        actions_module = importlib.import_module(module_path)
        modules = []

        for class_path in actions_module.action_classes:
            _module, _klass =  class_path.rsplit('.', maxsplit=1)
            modules.append(dict(module='{0}.{1}'.format(module_path, _module),
                                klass=_klass))

        for module in modules:
            m = importlib.import_module(module['module'])
            action_class = getattr(m, module['klass'])
            obj = action_class(self.scripter)
            parent = self.mainWidget.findChild(QObject, obj.parent)
            self.actions.append(dict(action=obj, parent=parent))

        for action in self.actions:
            action['parent'].addAction(action['action'])

    def loadWidgets(self):
        modulePath = 'scripter.ui_scripter.stackwidgets'
        widgetsModule = importlib.import_module(modulePath)
        modules = []

        for classPath in widgetsModule.widgetClasses:
            _module, _klass =  classPath.rsplit('.', maxsplit=1)
            modules.append(dict(module='{0}.{1}'.format(modulePath, _module),
                                klass=_klass))

        for module in modules:
            m = importlib.import_module(module['module'])
            widgetClass = getattr(m, module['klass'])
            obj = widgetClass(self.scripter)
            self.tabWidget.addTab(obj, obj.objectName())

    def invokeAction(self, actionName):
        for action in self.actions:
            if action['action'].objectName() == actionName:
                method = getattr(action['action'], actionName)
                if method:
                    return method()

    def findStackWidget(self, widgetName):
        for index in range(self.tabWidget.count()):
            widget = self.tabWidget.widget(index)
            if widget.objectName() == widgetName:
                return widget

    def showException(self, exception):
        QMessageBox.critical(self.editor, "Error running script", str(exception))

    def setDocumentEditor(self, document):
        self.editor.clear()
        self.editor.moveCursor(QTextCursor.Start)
        self.editor.insertPlainText(document.data)
        self.editor.moveCursor(QTextCursor.End)

    def setStatusBar(self, value='untitled'):
        self.statusBar.setText(value)

    def setActiveWidget(self, widgetName):
        widget = self.findStackWidget(widgetName)

        if widget:
            self.tabWidget.setCurrentWidget(widget)

    def setStepped(self, status):
        self.editor.setStepped(status)

    def clearEditor(self):
        self.editor.clear()

    def repaintDebugArea(self):
        self.editor.repaintDebugArea()
