"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
import bdb
import multiprocessing
import sys
import asyncio
from . import debuggerformatter


class Debugger(bdb.Bdb):

    def __init__(self, scripter, cmd):
        bdb.Bdb.__init__(self)

        self.quit = False
        self.debugq = multiprocessing.Queue()
        self.scripter = scripter
        self.applicationq = multiprocessing.Queue()
        self.filePath = self.scripter.documentcontroller.activeDocument.filePath
        self.application_data = {}
        self.exception_data = {}
        self.debugprocess = multiprocessing.Process(target=self._run, args=(self.filePath,))
        self.currentLine = 0

        bdb.Bdb.reset(self)

    def _run(self, filename):
        try:
            self.mainpyfile = self.canonic(filename)
            with open(filename, "rb") as fp:
                statement = "exec(compile(%r, %r, 'exec'))" % \
                            (fp.read(), self.mainpyfile)
            self.run(statement)
        except Exception as e:
            raise e

    def user_call(self, frame, args):
        name = frame.f_code.co_name or "<unknown>"

    def user_line(self, frame):
        """Handler that executes with every line of code"""
        co = frame.f_code

        if self.filePath != co.co_filename:
            return

        self.currentLine = frame.f_lineno
        self.applicationq.put({"code": {"file": co.co_filename,
                                        "name": co.co_name,
                                        "lineNumber": str(frame.f_lineno)
                                        },
                               "frame": {"firstLineNumber": co.co_firstlineno,
                                         "locals": debuggerformatter.format_data(frame.f_locals),
                                         "globals": debuggerformatter.format_data(frame.f_globals)
                                         },
                               "trace": "line"
                               })

        if self.quit:
            return self.set_quit()
        if self.currentLine == 0:
            return
        else:
            cmd = self.debugq.get()

            if cmd == "step":
                return
            if cmd == "stop":
                return self.set_quit()

    def user_return(self, frame, value):
        name = frame.f_code.co_name or "<unknown>"

        if name == '<module>':
            self.applicationq.put({"quit": True})

    def user_exception(self, frame, exception):
        self.applicationq.put({"exception": str(exception[1])})

    async def display(self):
        """Coroutine for updating the UI"""

        while True:
            if self.applicationq.empty():
                await asyncio.sleep(0.3)
            else:
                while not self.applicationq.empty():
                    self.application_data.update(self.applicationq.get())
                    self.scripter.uicontroller.repaintDebugArea()
                    return

    async def start(self):
        await self.display()

    async def step(self):
        self.debugq.put("step")
        await self.display()

    async def stop(self):
        self.debugq.put("stop")
        self.applicationq.put({"quit": True})
        await self.display()
