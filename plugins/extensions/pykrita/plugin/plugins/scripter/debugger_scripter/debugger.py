import bdb
import asyncio
import inspect
import multiprocessing
import re


from . import debuggerformatter


class Debugger(bdb.Bdb):

    def __init__(self, scripter, cmd):
        bdb.Bdb.__init__(self)

        self.quit = False
        self.debugq = multiprocessing.Queue()
        self.scripter = scripter
        self.applicationq = multiprocessing.Queue()
        self.filePath = self.scripter.documentcontroller.activeDocument.filePath
        # Create the debug process
        self.debugprocess = multiprocessing.Process(target=self.run, args=(cmd,))
        self.application_data = {}
        self.currentLine = 0
        # initialize parent
        bdb.Bdb.reset(self)

    def run(self, cmd):
        try:
            super(Debugger, self).run(cmd)
        except:
            print('terminado')

    def user_call(self, frame, args):
        name = frame.f_code.co_name or "<unknown>"

    def user_line(self, frame):
        """Handler that executes with every line of code"""
        co = frame.f_code

        if self.filePath!=co.co_filename:
            return

        self.currentLine = frame.f_lineno
        self.applicationq.put({ "code": { "file": co.co_filename,
                                          "name": co.co_name,
                                          "lineNumber": str(frame.f_lineno)
                                        },
                                "frame": { "firstLineNumber": co.co_firstlineno,
                                           "locals": debuggerformatter.format_data(frame.f_locals),
                                           "globals": debuggerformatter.format_data(frame.f_globals)
                                          },
                                "trace": "line"
                              })

        if self.quit:
            return self.set_quit()

        if self.currentLine==0:
            return
        else:
            # Wait for a debug command
            cmd = self.debugq.get()

            if cmd == "step":
                # If stepping through code, return this handler
                return

            if cmd == "stop":
                # If stopping execution, raise an exception
                return self.set_quit()

    def user_return(self, frame, value):
        name = frame.f_code.co_name or "<unknown>"
        if name == '<module>':
            self.applicationq.put({ "quit": True})

    def user_exception(self, frame, exception):
        name = frame.f_code.co_name or "<unknown>"
        print(str(exception))
        self.applicationq.put({ "exception": str(exception)})


    async def display(self):
        """Coroutine for updating the UI"""

        # Wait for the application queue to have an update to the GUI
        while True:
            if self.applicationq.empty():
                await asyncio.sleep(0.3)
            else:
                # The application queue has at least one item, let's act on every item that's in it
                while not self.applicationq.empty():
                    # Get info to the GUI
                     self.application_data = self.applicationq.get()
                     self.scripter.uicontroller.repaintDebugArea()
                return

    async def start(self):
        await self.display()

    async def step(self):
        # Tell the debugger we want to step in
        self.debugq.put("step")

        await self.display()

    async def stop(self):
        # Tell the debugger we're stopping execution
        self.debugq.put("stop")
        self.applicationq.put({ "quit": True})
        await self.display()
