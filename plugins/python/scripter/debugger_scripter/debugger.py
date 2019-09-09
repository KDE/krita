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
import bdb
import multiprocessing
import sys
if sys.version_info[0] > 2:
    import asyncio
else:
    import trollius as asyncio
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

    @asyncio.coroutine
    def display(self):
        """Coroutine for updating the UI"""

        while True:
            if self.applicationq.empty():
                # 'yield from' is not available in Python 2.
                for i in asyncio.sleep(0.3):
                    yield i
            else:
                while not self.applicationq.empty():
                    self.application_data.update(self.applicationq.get())
                    self.scripter.uicontroller.repaintDebugArea()
                    return

    @asyncio.coroutine
    def start(self):
        yield from self.display()

    @asyncio.coroutine
    def step(self):
        self.debugq.put("step")
        yield from self.display()

    @asyncio.coroutine
    def stop(self):
        self.debugq.put("stop")
        self.applicationq.put({"quit": True})
        yield from self.display()
