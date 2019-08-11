"""
Copyright (c) 2017 Eliakin Costa <eliakim170@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
