"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
from .debugger_scripter import debugger
import sys
if sys.version_info[0] > 2:
    import asyncio
else:
    # trollius is a port of asyncio for python2.
    import trollius as asyncio


class DebugController (object):

    def __init__(self, scripter):
        self._debugger = None
        self._cmd = None
        self.scripter = scripter

    def start(self, document):
        self.setCmd(compile(document.data, document.filePath, "exec"))
        self._debugger = debugger.Debugger(self.scripter, self._cmd)
        self._debugger.debugprocess.start()
        loop = asyncio.get_event_loop()
        loop.run_until_complete(self._debugger.start())
        self.updateUIDebugger()

    def step(self):
        loop = asyncio.get_event_loop()
        loop.run_until_complete(self._debugger.step())
        self.scripter.uicontroller.setStepped(True)
        self.updateUIDebugger()

    def stop(self):
        loop = asyncio.get_event_loop()
        loop.run_until_complete(self._debugger.stop())
        self.updateUIDebugger()
        self._debugger = None

    def setCmd(self, cmd):
        self._cmd = cmd

    @property
    def isActive(self):
        try:
            if self._debugger:
                return self._debugger.debugprocess.is_alive()
            return False
        except Exception:
            return False

    @property
    def currentLine(self):
        try:
            if self._debugger:
                return int(self.debuggerData['code']['lineNumber'])
        except Exception:
            return 0

    def updateUIDebugger(self):
        widget = self.scripter.uicontroller.findTabWidget(i18n('Debugger'))
        exception = self._debuggerException()

        if exception:
            self.scripter.uicontroller.showException(exception)
        if not self.isActive or self._quitDebugger():
            widget.disableToolbar(True)

        self.scripter.uicontroller.repaintDebugArea()
        widget.updateWidget()

    @property
    def debuggerData(self):
        try:
            if self._debugger:
                return self._debugger.application_data
        except Exception:
            return

    def _quitDebugger(self):
        try:
            return self.debuggerData['quit']
        except Exception:
            return False

    def _debuggerException(self):
        try:
            return self.debuggerData['exception']
        except Exception:
            return False
