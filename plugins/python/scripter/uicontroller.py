"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
from PyQt5.QtCore import Qt, QObject, QFileInfo, QRect
from PyQt5.QtGui import QTextCursor, QPalette
from PyQt5.QtWidgets import (QToolBar, QMenuBar, QTabWidget,
                             QLabel, QVBoxLayout, QMessageBox,
                             QSplitter, QSizePolicy)
from .ui_scripter.syntax import syntax, syntaxstyles
from .ui_scripter.editor import pythoneditor
from . import scripterdialog, utils
import importlib
import krita

KEY_GEOMETRY = "geometry"
DEFAULT_GEOMETRY = QRect(600, 200, 400, 500)
# essentially randomly placed


class Elided_Text_Label(QLabel):
    mainText = str()

    def __init__(self, parent=None):
        super(QLabel, self).__init__(parent)
        self.setMinimumWidth(self.fontMetrics().width("..."))
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Minimum)

    def setMainText(self, text=str()):
        self.mainText = text
        self.elideText()

    def elideText(self):
        self.setText(self.fontMetrics().elidedText(self.mainText, Qt.ElideRight, self.width()))

    def resizeEvent(self, event):
        self.elideText()


class UIController(object):
    documentModifiedText = ""
    documentStatusBarText = "untitled"

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
        self.statusBar = Elided_Text_Label()
        self.statusBar.setMainText('untitled')
        self.splitter = QSplitter()
        self.splitter.setOrientation(Qt.Vertical)
        self.highlight = syntax.PythonHighlighter(self.editor.document(), syntaxstyles.DefaultSyntaxStyle())
        p = self.editor.palette()
        p.setColor(QPalette.Base, syntaxstyles.DefaultSyntaxStyle()['background'].foreground().color())
        p.setColor(QPalette.Text, syntaxstyles.DefaultSyntaxStyle()['foreground'].foreground().color())
        self.editor.setPalette(p)

        utils.setNeedDarkIcon(self.mainWidget.palette().window().color())
        self.scripter = scripter

        self.loadMenus()
        self.loadWidgets()
        self.loadActions()
        self._readSettings()  # sets window size

        vbox = QVBoxLayout(self.mainWidget)
        vbox.addWidget(self.menu_bar)
        vbox.addWidget(self.actionToolbar)
        self.splitter.addWidget(self.editor)
        self.splitter.addWidget(self.tabWidget)
        vbox.addWidget(self.splitter)
        vbox.addWidget(self.statusBar)

        self.mainWidget.setWindowTitle(i18n("Scripter"))
        self.mainWidget.setSizeGripEnabled(True)
        self.mainWidget.show()
        self.mainWidget.activateWindow()

        self.editor.undoAvailable.connect(self.setStatusModified)

    def loadMenus(self):
        self.addMenu(i18n('File'), i18n('File'))

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
            _module = class_path[:class_path.rfind(".")]
            _klass = class_path[class_path.rfind(".") + 1:]
            modules.append(dict(module='{0}.{1}'.format(module_path, _module),
                                klass=_klass))

        for module in modules:
            m = importlib.import_module(module['module'])
            action_class = getattr(m, module['klass'])
            obj = action_class(self.scripter)
            obj_parent = obj.parent
            for name in obj_parent:
                parent = self.mainWidget.findChild(QObject, i18n(name))
                self.actions.append(dict(action=obj, parent=parent))

        for action in self.actions:
            action['parent'].addAction(action['action'])

    def loadWidgets(self):
        modulePath = 'scripter.ui_scripter.tabwidgets'
        widgetsModule = importlib.import_module(modulePath)
        modules = []

        for class_path in widgetsModule.widgetClasses:
            _module = class_path[:class_path.rfind(".")]
            _klass = class_path[class_path.rfind(".") + 1:]
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
        QMessageBox.critical(self.editor, i18n("Error Running Script"), str(exception))

    def setDocumentEditor(self, document):
        self.editor.clear()
        self.editor.moveCursor(QTextCursor.Start)
        self.editor._documentModified = False
        self.editor.setPlainText(document.data)
        self.editor.moveCursor(QTextCursor.End)

    def setStatusBar(self, value='untitled'):
        self.documentStatusBarText = value
        self.statusBar.setMainText(self.documentStatusBarText + self.documentModifiedText)

    def setStatusModified(self):
        self.documentModifiedText = ""
        if (self.editor._documentModified is True):
            self.documentModifiedText = " [Modified]"
        self.statusBar.setMainText(self.documentStatusBarText + self.documentModifiedText)

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
        else:
            self.scripter.settings.setValue('activeDocumentPath', '')

        self.scripter.settings.setValue('editorFontSize', self.editor.fontInfo().pointSize())

        for action in self.actions:
            writeSettings = getattr(action['action'], "writeSettings", None)
            if callable(writeSettings):
                writeSettings()

        #  Window Geometry
        rect = self.mainWidget.geometry()
        self.scripter.settings.setValue(KEY_GEOMETRY, rect)

        self.scripter.settings.endGroup()

    def _readSettings(self):
        """ It's similar to _writeSettings, but reading the settings when the ScripterDialog is closed. """

        self.scripter.settings.beginGroup('scripter')

        activeDocumentPath = self.scripter.settings.value('activeDocumentPath', '')

        if activeDocumentPath:
            if QFileInfo(activeDocumentPath).exists():
                document = self.scripter.documentcontroller.openDocument(activeDocumentPath)
                self.setStatusBar(document.filePath)
                self.setDocumentEditor(document)

        for action in self.actions:
            readSettings = getattr(action['action'], "readSettings", None)
            if callable(readSettings):
                readSettings()

        pointSize = self.scripter.settings.value('editorFontSize', str(self.editor.fontInfo().pointSize()))
        self.editor.setFontSize(int(pointSize))

        # Window Geometry
        rect = self.scripter.settings.value(KEY_GEOMETRY, DEFAULT_GEOMETRY)
        self.mainWidget.setGeometry(rect)

        self.scripter.settings.endGroup()

    def _saveSettings(self):
        self.scripter.settings.sync()
