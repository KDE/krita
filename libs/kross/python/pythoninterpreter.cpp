/***************************************************************************
 * pythoninterpreter.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "pythoninterpreter.h"
#include "pythonscript.h"
#include "pythonmodule.h"
//#include "pythonextension.h"

#include <kglobal.h>
#include <kstandarddirs.h>

#if defined(Q_WS_WIN)
  #define PYPATHDELIMITER ";"
#else
  #define PYPATHDELIMITER ":"
#endif

extern "C"
{
    /**
     * Exported and loadable function as entry point to use
     * the \a PythonInterpreter.
     * The krosspython library the \a PythonInterpreter is part
     * will be loaded dynamically at runtime from e.g.
     * \a Kross::Manager::getInterpreter and this exported
     * function will be used to return an instance of the
     * \a PythonInterpreter implementation.
     */
    void* krossinterpreter(Kross::InterpreterInfo* info)
    {
        return new Kross::PythonInterpreter(info);
    }
}

using namespace Kross;

namespace Kross {

    /// \internal
    class PythonInterpreterPrivate
    {
        public:
            /// The __main__ python module.
            PythonModule* mainmodule;

            PythonInterpreterPrivate() : mainmodule(0) {}
    };

}

PythonInterpreter::PythonInterpreter(Kross::InterpreterInfo* info)
    : Kross::Interpreter(info)
    , d(new PythonInterpreterPrivate())
{
    // Initialize the python interpreter.
    initialize();

    // Set name of the program.
    Py_SetProgramName(const_cast<char*>("Kross"));

    /*
    // Set arguments.
    //char* comm[0];
    const char* comm = const_cast<char*>("kross"); // name.
    PySys_SetArgv(1, comm);
    */

    // In the python sys.path are all module-directories are
    // listed in.
    QString path;

    // First import the sys-module to remember it's sys.path
    // list in our path QString.
    Py::Module sysmod( PyImport_ImportModule( (char*)"sys" ), true );
    Py::Dict sysmoddict = sysmod.getDict();
    Py::Object syspath = sysmoddict.getItem("path");
    if(syspath.isList()) {
        Py::List syspathlist = syspath;
        for(Py::List::iterator it = syspathlist.begin(); it != syspathlist.end(); ++it)
            if( (*it).isString() )
                path.append( QString(Py::String(*it).as_string().c_str()) + PYPATHDELIMITER );
    }
    else
        path = Py_GetPath();

    // Determinate additional module-paths we like to add.
    // First add the global Kross modules-path.
    QStringList krossdirs = KGlobal::dirs()->findDirs("data", "kross/python");
    for(QStringList::Iterator krossit = krossdirs.begin(); krossit != krossdirs.end(); ++krossit)
        path.append(*krossit + PYPATHDELIMITER);
    // Then add the application modules-path.
    QStringList appdirs = KGlobal::dirs()->findDirs("appdata", "kross/python");
    for(QStringList::Iterator appit = appdirs.begin(); appit != appdirs.end(); ++appit)
        path.append(*appit + PYPATHDELIMITER);

    // Set the extended sys.path.
    PySys_SetPath( (char*) path.toLatin1().data() );

    krossdebug(QString("Python ProgramName: %1").arg(Py_GetProgramName()));
    krossdebug(QString("Python ProgramFullPath: %1").arg(Py_GetProgramFullPath()));
    krossdebug(QString("Python Version: %1").arg(Py_GetVersion()));
    krossdebug(QString("Python Platform: %1").arg(Py_GetPlatform()));
    krossdebug(QString("Python Prefix: %1").arg(Py_GetPrefix()));
    krossdebug(QString("Python ExecPrefix: %1").arg(Py_GetExecPrefix()));
    //krossdebug(QString("Python Path: %1").arg(Py_GetPath()));
    //krossdebug(QString("Python System Path: %1").arg(path));

    // Initialize the main module.
    d->mainmodule = new PythonModule(this);

    // The main dictonary.
    Py::Dict moduledict = d->mainmodule->getDict();
    //TODO moduledict["KrossPythonVersion"] = Py::Int(KROSS_PYTHON_VERSION);

    // Prepare the interpreter.
    QString s =
        "import sys\n"
        //"sys.setdefaultencoding('latin-1')\n"

        // Dirty hack to get sys.argv defined. Needed for e.g. TKinter.
        "sys.argv = ['']\n"

        // On the try to read something from stdin always return an empty
        // string. That way such reads don't block our script.
        "try:\n"
        "    import cStringIO\n"
        "    sys.stdin = cStringIO.StringIO()\n"
        "except:\n"
        "    pass\n"

        // Class to redirect something. We use this class e.g. to redirect
        // <stdout> and <stderr> to a c++ event.
        //"class Redirect:\n"
        //"  def __init__(self, target):\n"
        //"    self.target = target\n"
        //"  def write(self, s):\n"
        //"    self.target.call(s)\n"

        // Wrap builtin __import__ method. All import requests are
        // first redirected to our PythonModule.import method and
        // if the call returns None, then we call the original
        // python import mechanism.
        "import __builtin__\n"
        "import __main__\n"
        "class _Importer:\n"
        "    def __init__(self):\n"
        "        self.realImporter = __builtin__.__import__\n"
        "        __builtin__.__import__ = self._import\n"
        "    def _import(self, name, globals=None, locals=None, fromlist=[]):\n"
        "        mod = __main__._import(name, globals, locals, fromlist)\n"
        "        if mod != None: return mod\n"
        "        return self.realImporter(name, globals, locals, fromlist)\n"
        "_Importer()\n"
        ;

    PyObject* pyrun = PyRun_String(s.toLatin1().data(), Py_file_input, moduledict.ptr(), moduledict.ptr());
    if(! pyrun) {
        Py::Object errobj = Py::value(Py::Exception()); // get last error
        setError( QString("Failed to prepare the __main__ module: %1").arg(errobj.as_string().c_str()) );
    }
    Py_XDECREF(pyrun); // free the reference.
}

PythonInterpreter::~PythonInterpreter()
{
    // Free the main module.
    delete d->mainmodule; d->mainmodule = 0;
    // Finalize the python interpreter.
    finalize();
    // Delete the private d-pointer.
    delete d;
}

void PythonInterpreter::initialize()
{
    // Initialize python.
    Py_Initialize();

    /* Not needed cause we use the >= Python 2.3 GIL-mechanism.
    PyThreadState* d->globalthreadstate, d->threadstate;
    // First we have to initialize threading if python supports it.
    PyEval_InitThreads();
    // The main thread. We don't use it later.
    d->globalthreadstate = PyThreadState_Swap(NULL);
    d->globalthreadstate = PyEval_SaveThread();
    // We use an own sub-interpreter for each thread.
    d->threadstate = Py_NewInterpreter();
    // Note that this application has multiple threads.
    // It maintains a separate interp (sub-interpreter) for each thread.
    PyThreadState_Swap(d->threadstate);
    // Work done, release the lock.
    PyEval_ReleaseLock();
    */
}

void PythonInterpreter::finalize()
{
    /* Not needed cause we use the >= Python 2.3 GIL-mechanism.
    // Lock threads.
    PyEval_AcquireLock();
    // Free the used thread.
    PyEval_ReleaseThread(d->threadstate);
    // Set back to rememberd main thread.
    PyThreadState_Swap(d->globalthreadstate);
    // Work done, unlock.
    PyEval_ReleaseLock();
    */

    // Finalize python.
    Py_Finalize();
}

Kross::Script* PythonInterpreter::createScript(Kross::Action* Action)
{
    //if(hadError()) return 0;
    return new PythonScript(this, Action);
}


PythonModule* PythonInterpreter::mainModule() const
{
    return d->mainmodule;
}

