# SPDX-License-Identifier: CC0-1.0

import sys
import krita
from PyQt5.QtGui import QIcon
from . import uitenscripts

if sys.version_info[0] > 2:
    import importlib
else:
    import imp


class TenScriptsExtension(krita.Extension):

    def __init__(self, parent):
        super(TenScriptsExtension, self).__init__(parent)

        self.scripts = []
        self.actionToIndex = {}
        self.indexToAction = {}

    def setup(self):
        self.readSettings()

    def createActions(self, window):
        action = window.createAction("ten_scripts", i18n("Ten Scripts"))
        action.setToolTip(i18n("Assign ten scripts to ten shortcuts."))
        action.triggered.connect(self.initialize)
        self.loadActions(window)

    def initialize(self):
        self.uitenscripts = uitenscripts.UITenScripts()
        self.uitenscripts.initialize(self)

    def readSettings(self):
        self.scripts = Application.readSetting(
            "tenscripts", "scripts", "").split(',')

    def writeSettings(self):

        saved_scripts = self.uitenscripts.saved_scripts()
        self.scripts = saved_scripts

        Application.writeSetting(
            "tenscripts", "scripts", ','.join(map(str, saved_scripts)))

    def loadActions(self, window):
        for index, item in enumerate(['1', '2', '3', '4', '5',
                                      '6', '7', '8', '9', '10']):
            action = window.createAction(
                "execute_script_" + item,
                str(i18n("Execute Script {num}")).format(num=item),
                "")
            action.triggered.connect(self._executeScript)
            self.actionToIndex[action.objectName()] = index
            self.indexToAction[index] = action.objectName()

    def _executeScript(self):
        scriptIndex = self.actionToIndex[self.sender().objectName()]
        script = self.scripts[scriptIndex] if len(self.scripts) > scriptIndex else None
        window = Application.activeWindow()

        if script:
            try:
                if sys.version_info[0] > 2:
                    spec = importlib.util.spec_from_file_location(
                        "users_script", script)
                    users_module = importlib.util.module_from_spec(spec)
                    spec.loader.exec_module(users_module)
                else:
                    users_module = imp.load_source("users_script", script)

                if (hasattr(users_module, 'main')
                        and callable(users_module.main)):
                    users_module.main()

                window.activeView().showFloatingMessage(
                    str(i18n("Script {0} executed")).format(script), QIcon(), 1500, 2)

            except Exception as e:
                window.activeView().showFloatingMessage(str(e), QIcon(), 2000, 1)
        else:
            window.activeView().showFloatingMessage(
                str(i18n(("You did not assign a script to that action"))), QIcon(), 1500, 2)
