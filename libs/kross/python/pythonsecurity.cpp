/***************************************************************************
 * pythonsecurity.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
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

#include "pythonsecurity.h"
#include "pythoninterpreter.h"
#include "pythonmodule.h"

#include <kdebug.h>

using namespace Kross::Python;

PythonSecurity::PythonSecurity(PythonInterpreter* interpreter)
    : Py::ExtensionModule<PythonSecurity>("PythonSecurity")
    , m_interpreter(interpreter)
    , m_pymodule(0)
{
    add_varargs_method("_getattr_", &PythonSecurity::_getattr_, "Secure wapper around the getattr method.");
    initialize("The PythonSecurity module used to wrap the RestrictedPython functionality.");

    /* TESTCASE
    initRestrictedPython();
    compile_restricted(
        "a = 2 + 5\n"
        "import os\n"
        "import sys\n"
        "b = sys.path\n"
        "print \"######### >>>testcase<<< #########\" \n"
        ,
        "mytestcase", // filename
        "exec" // 'exec' or 'eval' or 'single'
    );
    */

}

PythonSecurity::~PythonSecurity()
{
    delete m_pymodule;
}

void PythonSecurity::initRestrictedPython()
{
    try {
        Py::Dict mainmoduledict = ((PythonInterpreter*)m_interpreter)->mainModule()->getDict();
        PyObject* pymodule = PyImport_ImportModuleEx(
            "RestrictedPython", // name of the module being imported (may be a dotted name)
            mainmoduledict.ptr(), // reference to the current global namespace
            mainmoduledict.ptr(), // reference to the local namespace
            0 // PyObject *fromlist
        );
        if(! pymodule)
            throw Py::Exception();
        m_pymodule = new Py::Module(pymodule, true);

        PyObject* pyrun = PyRun_String(
            //"import os\n"
            //"import sys\n"
            "import __main__\n"
            "import PythonSecurity\n"
            "from RestrictedPython import compile_restricted, PrintCollector\n"
            "from RestrictedPython.Eval import RestrictionCapableEval\n"
            "from RestrictedPython.RCompile import RModule\n"

            "setattr(__main__, '_getattr_', PythonSecurity._getattr_)\n"
            "setattr(__main__, '_print_', PrintCollector)\n"
            ,
            Py_file_input,
            m_pymodule->getDict().ptr(),
            m_pymodule->getDict().ptr()
        );
        if(! pyrun)
            throw Py::Exception();

        kDebug()<<"!!!!!!!!!!!!!! PythonSecurity::PythonSecurity SUCCESS !!!!!!!!!!!!!!!!!"<<endl;
    }
    catch(Py::Exception& e) {
        QString err = Py::value(e).as_string().c_str();
        e.clear();
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(QString("Failed to initialize PythonSecurity module: %1").arg(err) ) );
    }
}

Py::Object PythonSecurity::_getattr_(const Py::Tuple& args)
{
    kDebug() << "PythonSecurity::_getattr_" << endl;
    for(uint i = 0; i < args.size(); i++) {
        Py::Object o = args[i];
        kDebug()<<o.as_string().c_str()<<endl;
    }
    return Py::None();
}

PyObject* PythonSecurity::compile_restricted(const QString& source, const QString& filename, const QString& mode)
{
    kDebug()<<"PythonSecurity::compile_restricted"<<endl;
    if(! m_pymodule)
        initRestrictedPython(); // throws exception if failed

    try {
        Py::Dict mainmoduledict = ((PythonInterpreter*)m_interpreter)->mainModule()->getDict();

        PyObject* func = PyDict_GetItemString(m_pymodule->getDict().ptr(), "compile_restricted");
        if(! func)
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(QString("No such function '%1'.").arg("compile_restricted")) );

        Py::Callable funcobject(func, true); // the funcobject takes care of freeing our func pyobject.

        if(! funcobject.isCallable())
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(QString("Function '%1' is not callable.").arg("compile_restricted")) );

        Py::Tuple args(3);
        args[0] = Py::String(source.utf8());
        args[1] = Py::String(filename.utf8());
        args[2] = Py::String(mode.utf8());

        Py::Object result = funcobject.apply(args);

        PyObject* pycode = PyEval_EvalCode(
            (PyCodeObject*)result.ptr(),
            mainmoduledict.ptr(),
            mainmoduledict.ptr()
        );
        if(! pycode)
            throw Py::Exception();

        /*
        kDebug()<<"$---------------------------------------------------"<<endl;
        Py::List ml = mainmoduledict;
        for(Py::List::size_type mi = 0; mi < ml.length(); ++mi) {
            kDebug() << QString("-------------------") << endl;
            kDebug() << QString("dir() = %1").arg( ml[mi].str().as_string().c_str() ) << endl;
            //kDebug() << QString("dir().dir() = %1").arg( Py::Object(ml[mi]).dir().as_string().c_str() ) << endl;
        }
        kDebug()<<"$---------------------------------------------------"<<endl;
        */

        Py::Object code(pycode);
        kDebug()<< code.as_string().c_str() << " callable=" << PyCallable_Check(code.ptr()) << endl;
        Py::List l = code.dir();
        for(Py::List::size_type i = 0; i < l.length(); ++i) {
            kDebug() << QString("dir() = %1").arg( l[i].str().as_string().c_str() ) << endl;
            //kDebug() << QString("dir().dir() = %1").arg( Py::Object(l[i]).dir().as_string().c_str() ) << endl;
        }

        return pycode;
    }
    catch(Py::Exception& e) { 
        QString err = Py::value(e).as_string().c_str();
        e.clear();
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(QString("Function '%1' failed with python exception: %2").arg("compile_restricted").arg(err) ) );
    }
}

#if 0
void PythonSecurity::compile_restricted_function(const Py::Tuple& /*args*/, const QString& /*body*/, const QString& /*name*/, const QString& /*filename*/, const Py::Object& /*globalize*/)
{
    //TODO
}

void PythonSecurity::compile_restricted_exec(const QString& /*source*/, const QString& /*filename*/)
{
    //TODO
}

void PythonSecurity::compile_restricted_eval(const QString& /*source*/, const QString& /*filename*/)
{
    //TODO
}
#endif

