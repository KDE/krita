/***************************************************************************
 * pythonscript.cpp
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

#include "pythonscript.h"
#include "pythonmodule.h"
#include "pythoninterpreter.h"
#include "pythonsecurity.h"
#include "../main/scriptcontainer.h"

//#include <kapplication.h>

using namespace Kross::Python;

namespace Kross { namespace Python {

    /// @internal
    class PythonScriptPrivate
    {
        public:

            /**
            * The \a Py::Module instance this \a PythonScript
            * has as local context.
            */
            Py::Module* m_module;

            /**
            * The PyCodeObject object representing the
            * compiled python code. Internaly we first
            * compile the python code and later execute
            * it.
            */
            Py::Object* m_code;

            /**
            * A list of functionnames.
            */
            QStringList m_functions;

            /**
            * A list of classnames.
            */
            QStringList m_classes;
    };

}}

PythonScript::PythonScript(Kross::Api::Interpreter* interpreter, Kross::Api::ScriptContainer* scriptcontainer)
    : Kross::Api::Script(interpreter, scriptcontainer)
    , d(new PythonScriptPrivate())
{
#ifdef KROSS_PYTHON_SCRIPT_CTOR_DEBUG
    krossdebug("PythonScript::PythonScript() Constructor.");
#endif
    d->m_module = 0;
    d->m_code = 0;
}

PythonScript::~PythonScript()
{
#ifdef KROSS_PYTHON_SCRIPT_DTOR_DEBUG
    krossdebug("PythonScript::~PythonScript() Destructor.");
#endif
    finalize();
    delete d;
}

void PythonScript::initialize()
{
    finalize();
    clearException(); // clear previously thrown exceptions.

    try {
        if(m_scriptcontainer->getCode().isNull())
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(QString("Invalid scripting code for script '%1'").arg( m_scriptcontainer->getName() )) );

        if(m_scriptcontainer->getName().isNull())
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(QString("Name for the script is invalid!")) );

        PyObject* pymod = PyModule_New( (char*) m_scriptcontainer->getName().latin1() );
        d->m_module = new Py::Module(pymod, true);
        if(! d->m_module)
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(QString("Failed to initialize local module context for script '%1'").arg( m_scriptcontainer->getName() )) );

#ifdef KROSS_PYTHON_SCRIPT_INIT_DEBUG
        krossdebug( QString("PythonScript::initialize() module='%1' refcount='%2'").arg(d->m_module->as_string().c_str()).arg(d->m_module->reference_count()) );
#endif

        // Set the "self" variable to point to the ScriptContainer
        // we are using for the script. That way we are able to
        // simply access the ScriptContainer itself from within
        // python scripting code.
        Py::Dict moduledict = d->m_module->getDict();
        moduledict["self"] = PythonExtension::toPyObject( m_scriptcontainer );
        //moduledict["parent"] = PythonExtension::toPyObject( m_manager );

/*
        // Prepare the local context.
        QString s =
            //"import sys\n"
            "if self.has(\"stdout\"):\n"
            "  self.stdout = Redirect( self.get(\"stdout\") )\n"
            "if self.has(\"stderr\"):\n"
            "  self.stderr = Redirect( self.get(\"stderr\") )\n"
            ;
        Py::Dict mainmoduledict = ((PythonInterpreter*)m_interpreter)->mainModule()->getDict();
        PyObject* pyrun = PyRun_StringFlags((char*)s.latin1(), Py_file_input, mainmoduledict.ptr(), moduledict.ptr());
        if(! pyrun)
            throw Py::Exception(); // throw exception
        Py_XDECREF(pyrun); // free the reference.
*/

        // Compile the python script code. It will be later on request
        // executed. That way we cache the compiled code.
        PyObject* code = 0;
        bool restricted = m_scriptcontainer->getOption("restricted", QVariant(false,0), true).toBool();

        krossdebug( QString("PythonScript::initialize() name=%1 restricted=%2").arg(m_scriptcontainer->getName()).arg(restricted) );
        if(restricted) {

            // Use the RestrictedPython module wrapped by the PythonSecurity class.
            code = dynamic_cast<PythonInterpreter*>(m_interpreter)->securityModule()->compile_restricted(
                m_scriptcontainer->getCode(),
                m_scriptcontainer->getName(),
                "exec"
            );

        }
        else {
            //PyCompilerFlags* cf = new PyCompilerFlags;
            //cf->cf_flags |= PyCF_SOURCE_IS_UTF8;

            // Just compile the code without any restrictions.
            code = Py_CompileString(
                (char*) m_scriptcontainer->getCode().latin1(),
                (char*) m_scriptcontainer->getName().latin1(),
                Py_file_input
            );
        }

        if(! code)
            throw Py::Exception();
        d->m_code = new Py::Object(code, true);
    }
    catch(Py::Exception& e) {
        QString err = Py::value(e).as_string().c_str();
        Kross::Api::Exception* exception = toException( QString("Failed to compile python code: %1").arg(err) );
        e.clear(); // exception is handled. clear it now.
        throw Kross::Api::Exception::Ptr(exception);
    }
}

void PythonScript::finalize()
{
#ifdef KROSS_PYTHON_SCRIPT_FINALIZE_DEBUG
    if(d->m_module)
        krossdebug( QString("PythonScript::finalize() module='%1' refcount='%2'").arg(d->m_module->as_string().c_str()).arg(d->m_module->reference_count()) );
#endif

    delete d->m_module; d->m_module = 0;
    delete d->m_code; d->m_code = 0;
    d->m_functions.clear();
    d->m_classes.clear();
}

Kross::Api::Exception* PythonScript::toException(const QString& error)
{
    PyObject *type, *value, *traceback;
    PyObject *lineobj = 0;
    Py::List tblist;

    PyErr_Fetch(&type, &value, &traceback);
    Py_FlushLine();
    PyErr_NormalizeException(&type, &value, &traceback);

    if(traceback) {
        lineobj = PyObject_GetAttrString(traceback, "tb_lineno");

        try {
            Py::Module tbmodule( PyImport_Import(Py::String("traceback").ptr()), true );
            Py::Dict tbdict = tbmodule.getDict();
            Py::Callable tbfunc(tbdict.getItem("format_tb"));
            Py::Tuple args(1);
            args.setItem(0, Py::Object(traceback));
            tblist = tbfunc.apply(args);
            uint length = tblist.length();
            for(Py::List::size_type i = 0; i < length; ++i)
                krossdebug( Py::Object(tblist[i]).as_string().c_str() );
        }
        catch(Py::Exception& e) {
            QString err = Py::value(e).as_string().c_str();
            e.clear(); // exception is handled. clear it now.
            krosswarning( QString("Kross::Python::PythonScript::toException() Failed to fetch a traceback: %1").arg(err) );
        }
    }

    if((! lineobj) && value)
        lineobj = PyObject_GetAttrString(value, "lineno"); //['args', 'filename', 'lineno', 'msg', 'offset', 'print_file_and_line', 'text']

    PyErr_Restore(type, value, traceback);

    long line = -1;
    if(lineobj) {
        Py::Object o(lineobj, true);
        if(o.isNumeric())
            line = long(Py::Long(o)) - 1; // python linecount starts with 1..
    }

    QStringList tb;
    uint tblength = tblist.length();
    for(Py::List::size_type i = 0; i < tblength; ++i)
        tb.append( Py::Object(tblist[i]).as_string().c_str() );

    Kross::Api::Exception* exception = new Kross::Api::Exception(error, line);;
    if(tb.count() > 0)
        exception->setTrace( tb.join("\n") );
    return exception;
}

const QStringList& PythonScript::getFunctionNames()
{
    if(! d->m_module)
        initialize(); //TODO catch exception
    return d->m_functions;
    /*
    QStringList list;
    Py::List l = d->m_module->getDict().keys();
    int length = l.length();
    for(Py::List::size_type i = 0; i < length; ++i)
        list.append( l[i].str().as_string().c_str() );
    return list;
    */
}

Kross::Api::Object::Ptr PythonScript::execute()
{
#ifdef KROSS_PYTHON_SCRIPT_EXEC_DEBUG
    krossdebug( QString("PythonScript::execute()") );
#endif

    try {
        if(! d->m_module)
            initialize();

        // the main module dictonary.
        Py::Dict mainmoduledict = ((PythonInterpreter*)m_interpreter)->mainModule()->getDict();
        // the local context dictonary.
        Py::Dict moduledict( d->m_module->getDict().ptr() );

        // Initialize context before execution.
        QString s =
            "import sys\n"
            //"if self.has(\"stdout\"):\n"
            //"  sys.stdout = Redirect( self.get(\"stdout\") )\n"
            //"if self.has(\"stderr\"):\n"
            //"  sys.stderr = Redirect( self.get(\"stderr\") )\n"
            ;

        PyObject* pyrun = PyRun_String(s.latin1(), Py_file_input, mainmoduledict.ptr(), moduledict.ptr());
        if(! pyrun)
            throw Py::Exception(); // throw exception
        Py_XDECREF(pyrun); // free the reference.

        // Acquire interpreter lock*/
        PyGILState_STATE gilstate = PyGILState_Ensure();

        // Evaluate the already compiled code.
        PyObject* pyresult = PyEval_EvalCode(
            (PyCodeObject*)d->m_code->ptr(),
            mainmoduledict.ptr(),
            moduledict.ptr()
        );

        // Free interpreter lock
        PyGILState_Release(gilstate);

        if(! pyresult) {
            krosswarning("Kross::Python::PythonScript::execute(): Failed to PyEval_EvalCode");
            throw Py::Exception();
        }
        Py::Object result(pyresult, true);

#ifdef KROSS_PYTHON_SCRIPT_EXEC_DEBUG
        krossdebug( QString("PythonScript::execute() result=%1").arg(result.as_string().c_str()) );
#endif

        for(Py::Dict::iterator it = moduledict.begin(); it != moduledict.end(); ++it) {
            Py::Dict::value_type vt(*it);
            if(PyClass_Check( vt.second.ptr() )) {
#ifdef KROSS_PYTHON_SCRIPT_EXEC_DEBUG
                krossdebug( QString("PythonScript::execute() class '%1' added.").arg(vt.first.as_string().c_str()) );
#endif
                d->m_classes.append( vt.first.as_string().c_str() );
            }
            else if(vt.second.isCallable()) {
#ifdef KROSS_PYTHON_SCRIPT_EXEC_DEBUG
                krossdebug( QString("PythonScript::execute() function '%1' added.").arg(vt.first.as_string().c_str()) );
#endif
                d->m_functions.append( vt.first.as_string().c_str() );
            }
        }

        Kross::Api::Object* r = PythonExtension::toObject(result);
        return Kross::Api::Object::Ptr(r);
    }
    catch(Py::Exception& e) {
        try {
            Py::Object errobj = Py::value(e);
            if(errobj.ptr() == Py_None) // at least string-exceptions have there errormessage in the type-object
                errobj = Py::type(e);
            QString err = errobj.as_string().c_str();

            Kross::Api::Exception* exception = toException( QString("Failed to execute python code: %1").arg(err) );
            e.clear(); // exception is handled. clear it now.
            setException( exception );
        }
        catch(Py::Exception& e) {
            QString err = Py::value(e).as_string().c_str();
            Kross::Api::Exception* exception = toException( QString("Failed to execute python code: %1").arg(err) );
            e.clear(); // exception is handled. clear it now.
            setException( exception );
        }
    }
    catch(Kross::Api::Exception::Ptr e) {
        setException(e.data());
    }

    return Kross::Api::Object::Ptr(0); // return nothing if exception got thrown.
}

Kross::Api::Object::Ptr PythonScript::callFunction(const QString& name, Kross::Api::List::Ptr args)
{
#ifdef KROSS_PYTHON_SCRIPT_CALLFUNC_DEBUG
    krossdebug( QString("PythonScript::callFunction(%1, %2)")
                 .arg(name)
                 .arg(args ? QString::number(args->count()) : QString("NULL")) );
#endif

    if(hadException()) // abort if we had an unresolved exception.
        return Kross::Api::Object::Ptr(0);

    if(! d->m_module) {
        setException( new Kross::Api::Exception(QString("Script not initialized.")) );
        return Kross::Api::Object::Ptr(0);
    }

    try {
        Py::Dict moduledict = d->m_module->getDict();

        // Try to determinate the function we like to execute.
        PyObject* func = PyDict_GetItemString(moduledict.ptr(), name.latin1());

        if( (! d->m_functions.contains(name)) || (! func) )
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(QString("No such function '%1'.").arg(name)) );

        Py::Callable funcobject(func, true); // the funcobject takes care of freeing our func pyobject.

        // Check if the object is really a function and therefore callable.
        if(! funcobject.isCallable())
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(QString("Function is not callable.")) );

        // Call the function.
        Py::Object result = funcobject.apply(PythonExtension::toPyTuple(args.data()));
        return Kross::Api::Object::Ptr( PythonExtension::toObject(result) );
    }
    catch(Py::Exception& e) {
        QString err = Py::value(e).as_string().c_str();
        e.clear(); // exception is handled. clear it now.
        setException( new Kross::Api::Exception(QString("Python Exception: %1").arg(err)) );
    }
    catch(Kross::Api::Exception::Ptr e) {
        setException(e.data());
    }

    return Kross::Api::Object::Ptr(0); // return nothing if exception got thrown.
}

const QStringList& PythonScript::getClassNames()
{
    if(! d->m_module)
        initialize(); //TODO catch exception
    return d->m_classes;
}

Kross::Api::Object::Ptr PythonScript::classInstance(const QString& name)
{
    if(hadException()) // abort if we had an unresolved exception.
        return Kross::Api::Object::Ptr(0);

    if(! d->m_module) {
        setException( new Kross::Api::Exception(QString("Script not initialized.")) );
        return Kross::Api::Object::Ptr(0);
    }

    try {
        Py::Dict moduledict = d->m_module->getDict();

        // Try to determinate the class.
        PyObject* pyclass = PyDict_GetItemString(moduledict.ptr(), name.latin1());
        if( (! d->m_classes.contains(name)) || (! pyclass) )
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(QString("No such class '%1'.").arg(name)) );

        PyObject *pyobj = PyInstance_New(pyclass, 0, 0);//aclarg, 0);
        if(! pyobj)
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(QString("Failed to create instance of class '%1'.").arg(name)) );

        Py::Object classobject(pyobj, true);

#ifdef KROSS_PYTHON_SCRIPT_CLASSINSTANCE_DEBUG
        krossdebug( QString("PythonScript::classInstance() inst='%1'").arg(classobject.as_string().c_str()) );
#endif
        return Kross::Api::Object::Ptr( PythonExtension::toObject(classobject) );
    }
    catch(Py::Exception& e) {
        QString err = Py::value(e).as_string().c_str();
        e.clear(); // exception is handled. clear it now.
        setException( new Kross::Api::Exception(err) );
    }
    catch(Kross::Api::Exception::Ptr e) {
        setException(e.data());
    }

    return Kross::Api::Object::Ptr(0); // return nothing if exception got thrown.
}

