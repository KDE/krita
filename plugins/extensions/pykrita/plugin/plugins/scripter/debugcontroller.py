from scripter.debugger_scripter import debugger
from code import InteractiveConsole
import asyncio


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
        except:
            return False

    @property
    def currentLine(self):
        try:
            if self._debugger:
                return int(self.debuggerData['code']['lineNumber'])
        except:
            return 0

    def updateUIDebugger(self):
        widget = self.scripter.uicontroller.findTabWidget('Debugger')
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
        except:
            return

    def _quitDebugger(self):
        try:
            return self.debuggerData['quit']
        except:
            return False

    def _debuggerException(self):
        try:
            return self.debuggerData['exception']
        except:
            return False
