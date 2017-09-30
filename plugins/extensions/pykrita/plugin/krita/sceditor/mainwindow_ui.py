# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'mainwindow.ui'
#
# Created: Fri Aug 15 04:25:57 2014
#      by: PyQt5 UI code generator 5.3.1
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_ScriptEditor(object):

    def setupUi(self, ScriptEditor):
        ScriptEditor.setObjectName("ScriptEditor")
        ScriptEditor.resize(624, 449)
        self.centralwidget = QtWidgets.QWidget(ScriptEditor)
        self.centralwidget.setObjectName("centralwidget")
        ScriptEditor.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(ScriptEditor)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 624, 25))
        self.menubar.setObjectName("menubar")
        self.menuFile = QtWidgets.QMenu(self.menubar)
        self.menuFile.setObjectName("menuFile")
        self.menu_New = QtWidgets.QMenu(self.menuFile)
        self.menu_New.setObjectName("menu_New")
        self.menuRun = QtWidgets.QMenu(self.menubar)
        self.menuRun.setObjectName("menuRun")
        ScriptEditor.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(ScriptEditor)
        self.statusbar.setObjectName("statusbar")
        ScriptEditor.setStatusBar(self.statusbar)
        self.actionClose = QtWidgets.QAction(ScriptEditor)
        self.actionClose.setObjectName("actionClose")
        self.actionExit = QtWidgets.QAction(ScriptEditor)
        self.actionExit.setObjectName("actionExit")
        self.actionRun = QtWidgets.QAction(ScriptEditor)
        self.actionRun.setObjectName("actionRun")
        self.actionRunConsole = QtWidgets.QAction(ScriptEditor)
        self.actionRunConsole.setObjectName("actionRunConsole")
        self.actionNewPython = QtWidgets.QAction(ScriptEditor)
        self.actionNewPython.setObjectName("actionNewPython")
        self.actionNewQtQml = QtWidgets.QAction(ScriptEditor)
        self.actionNewQtQml.setObjectName("actionNewQtQml")
        self.actionClear = QtWidgets.QAction(ScriptEditor)
        self.actionClear.setObjectName("actionClear")
        self.actionSave_As = QtWidgets.QAction(ScriptEditor)
        self.actionSave_As.setObjectName("actionSave_As")
        self.actionOpen = QtWidgets.QAction(ScriptEditor)
        self.actionOpen.setObjectName("actionOpen")
        self.actionSave = QtWidgets.QAction(ScriptEditor)
        self.actionSave.setObjectName("actionSave")
        self.menu_New.addAction(self.actionNewPython)
        self.menu_New.addAction(self.actionNewQtQml)
        self.menuFile.addAction(self.menu_New.menuAction())
        self.menuFile.addAction(self.actionOpen)
        self.menuFile.addAction(self.actionSave)
        self.menuFile.addAction(self.actionSave_As)
        self.menuFile.addAction(self.actionClose)
        self.menuFile.addSeparator()
        self.menuFile.addAction(self.actionExit)
        self.menuRun.addAction(self.actionRun)
        self.menuRun.addAction(self.actionRunConsole)
        self.menuRun.addAction(self.actionClear)
        self.menubar.addAction(self.menuFile.menuAction())
        self.menubar.addAction(self.menuRun.menuAction())

        self.retranslateUi(ScriptEditor)
        QtCore.QMetaObject.connectSlotsByName(ScriptEditor)

    def retranslateUi(self, ScriptEditor):
        _translate = QtCore.QCoreApplication.translate
        ScriptEditor.setWindowTitle(_translate("ScriptEditor", "Script Editor"))
        self.menuFile.setTitle(_translate("ScriptEditor", "&File"))
        self.menu_New.setTitle(_translate("ScriptEditor", "&New"))
        self.menuRun.setTitle(_translate("ScriptEditor", "&Run"))
        self.actionClose.setText(_translate("ScriptEditor", "&Close"))
        self.actionClose.setShortcut(_translate("ScriptEditor", "Ctrl+W"))
        self.actionExit.setText(_translate("ScriptEditor", "&Exit"))
        self.actionRun.setText(_translate("ScriptEditor", "&Run"))
        self.actionRun.setShortcut(_translate("ScriptEditor", "Ctrl+R"))
        self.actionRunConsole.setText(_translate("ScriptEditor", "Run script in &console"))
        self.actionRunConsole.setShortcut(_translate("ScriptEditor", "Ctrl+C"))
        self.actionNewPython.setText(_translate("ScriptEditor", "Python"))
        self.actionNewPython.setShortcut(_translate("ScriptEditor", "Ctrl+N"))
        self.actionNewQtQml.setText(_translate("ScriptEditor", "QtQml"))
        self.actionClear.setText(_translate("ScriptEditor", "Clear"))
        self.actionClear.setToolTip(_translate("ScriptEditor", "Clear The Console"))
        self.actionSave_As.setText(_translate("ScriptEditor", "Save &As"))
        self.actionSave_As.setToolTip(_translate("ScriptEditor", "Save the script"))
        self.actionSave_As.setShortcut(_translate("ScriptEditor", "Ctrl+A"))
        self.actionOpen.setText(_translate("ScriptEditor", "&Open"))
        self.actionOpen.setToolTip(_translate("ScriptEditor", "Open a script"))
        self.actionOpen.setShortcut(_translate("ScriptEditor", "Ctrl+O"))
        self.actionSave.setText(_translate("ScriptEditor", "&Save"))
        self.actionSave.setToolTip(_translate("ScriptEditor", "Save the current script"))
        self.actionSave.setShortcut(_translate("ScriptEditor", "Ctrl+S"))
