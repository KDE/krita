from PyQt5.QtGui import QTextCursor
from PyQt5.QtWidgets import (QToolBar, QMenuBar, QTabWidget,
                             QLabel, QVBoxLayout, QMessageBox,
                             QSplitter)
from PyQt5.QtCore import Qt, QObject
from scripter.ui_scripter.syntax import syntax, syntaxstyles
from scripter.ui_scripter.editor import pythoneditor
from scripter import scripterdialog
import os
import importlib


class UIController(object):

    def __init__(self):
        self.mainWidget = scripterdialog.ScripterDialog(self)
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
        self.splitter = QSplitter()
        self.splitter.setOrientation(Qt.Vertical)
        self.highlight = syntax.PythonHighlighter(self.editor.document(), syntaxstyles.DefaultSyntaxStyle())

        self.scripter = scripter

        self.loadMenus()
        self.loadWidgets()
        self.loadActions()
        self._readSettings()

        vbox = QVBoxLayout(self.mainWidget)
        vbox.addWidget(self.menu_bar)
        vbox.addWidget(self.actionToolbar)
        self.splitter.addWidget(self.editor)
        self.splitter.addWidget(self.tabWidget)
        vbox.addWidget(self.splitter)
        vbox.addWidget(self.statusBar)

        self.mainWidget.resize(400, 500)
        self.mainWidget.setWindowTitle("Scripter")
        self.mainWidget.setSizeGripEnabled(True)
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
            _module, _klass = class_path.rsplit('.', maxsplit=1)
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
        modulePath = 'scripter.ui_scripter.tabwidgets'
        widgetsModule = importlib.import_module(modulePath)
        modules = []

        for classPath in widgetsModule.widgetClasses:
            _module, _klass = classPath.rsplit('.', maxsplit=1)
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

    def findTabWidget(self, widgetName, childName=''):
        for index in range(self.tabWidget.count()):
            widget = self.tabWidget.widget(index)
            if widget.objectName() == widgetName:
                if childName:
                    widget = widget.findChild(QObject, childName)
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
        widget = self.findTabWidget(widgetName)

        if widget:
            self.tabWidget.setCurrentWidget(widget)

    def setStepped(self, status):
        self.editor.setStepped(status)

    def clearEditor(self):
        self.editor.clear()

    def repaintDebugArea(self):
        self.editor.repaintDebugArea()

    def closeScripter(self):
        self.mainWidget.close()

    def _writeSettings(self):
        """ _writeSettings is a method invoked when the scripter starts, making
            control inversion. Actions can implement a writeSettings method to
            save your own settings without this method to know about it. """

        self.scripter.settings.beginGroup('scripter')

        document = self.scripter.documentcontroller.activeDocument
        if document:
            self.scripter.settings.setValue('activeDocumentPath', document.filePath)

        for action in self.actions:
            writeSettings = getattr(action['action'], "writeSettings", None)
            if callable(writeSettings):
                writeSettings()

        self.scripter.settings.endGroup()

    def _readSettings(self):
        """ It's similar to _writeSettings, but reading the settings when the ScripterDialog is closed. """

        self.scripter.settings.beginGroup('scripter')

        activeDocumentPath = self.scripter.settings.value('activeDocumentPath', '')

        if activeDocumentPath:
            document = self.scripter.documentcontroller.openDocument(activeDocumentPath)
            self.setStatusBar(document.filePath)
            self.setDocumentEditor(document)

        for action in self.actions:
            readSettings = getattr(action['action'], "readSettings", None)
            if callable(readSettings):
                readSettings()

        self.scripter.settings.endGroup()

    def _saveSettings(self):
        self.scripter.settings.sync()
