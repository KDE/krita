/***************************************************************************
 * pythonscript.cpp
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

#include "pythonscript.h"
#include "pythonmodule.h"
#include "pythoninterpreter.h"
#include "pythonextension.h"
#include "pythonvariant.h"
#include "../core/action.h"

using namespace Kross;

namespace Kross {

    /// \internal
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

            PythonScriptPrivate() : m_module(0), m_code(0) {}
    };

}

PythonScript::PythonScript(Kross::Interpreter* interpreter, Kross::Action* action)
    : Kross::Script(interpreter, action)
    , d(new PythonScriptPrivate())
{
    #ifdef KROSS_PYTHON_SCRIPT_CTOR_DEBUG
        krossdebug("PythonScript::Constructor.");
    #endif
}

PythonScript::~PythonScript()
{
    #ifdef KROSS_PYTHON_SCRIPT_DTOR_DEBUG
        krossdebug("PythonScript::Destructor.");
    #endif
    finalize();
    delete d;
}

Py::Dict PythonScript::moduleDict() const
{
    Q_ASSERT(d->m_module);
    return d->m_module->getDict();
}

bool PythonScript::initialize()
{
    finalize(); // finalize before initialize
    clearError(); // clear previous errors.

    try {
        if(m_action->getCode().isNull()) {
            setError( QString("Invalid scripting code for script '%1'").arg(m_action->objectName()) );
            return false;
        }
        if(m_action->objectName().isNull()) {
            setError( QString("Name for the script is invalid!") );
            return false;
        }

        { // Each Action uses an own module as scope.
            PyObject* pymod = PyModule_New( const_cast< char* >( m_action->objectName().toLatin1().data() ) );
            d->m_module = new Py::Module(pymod, true);
            if(! d->m_module) {
                setError( QString("Failed to initialize local module context for script '%1'").arg(m_action->objectName()) );
                return false;
            }
        }

        #ifdef KROSS_PYTHON_SCRIPT_INIT_DEBUG
            krossdebug( QString("PythonScript::initialize() module='%1' refcount='%2'").arg(d->m_module->as_string().c_str()).arg(d->m_module->reference_count()) );
        #endif

        { // Add the Object instances to the modules dictonary.
            Py::Dict moduledict = d->m_module->getDict();
            QHashIterator< QString, QObject* > objectsit( m_action->objects() );
            while(objectsit.hasNext()) {
                objectsit.next();
                moduledict[ objectsit.key().toLatin1().data() ] = Py::asObject(new PythonExtension(objectsit.value()));
            }
        }

#if 0
        // Set the "self" variable to point to the Action
        // we are using for the script. That way we are able to
        // simply access the Action itself from within
        // python scripting code.
        Py::Dict moduledict = d->m_module->getDict();
        moduledict["self"] = PythonExtension::toPyObject( m_action );
        //moduledict["parent"] = PythonExtension::toPyObject( m_manager );
#endif

        /*
        // Prepare the local context.
        QString s =
            //"import sys\n"
            //"if self.has(\"stdout\"):\n"
            //"  self.stdout = Redirect( self.get(\"stdout\") )\n"
            //"if self.has(\"stderr\"):\n"
            //"  self.stderr = Redirect( self.get(\"stderr\") )\n"
            ;
        Py::Dict mainmoduledict = ((PythonInterpreter*)m_interpreter)->mainModule()->getDict();
        PyObject* pyrun = PyRun_StringFlags((char*)s.toLatin1().data(), Py_file_input, mainmoduledict.ptr(), moduledict.ptr());
        if(! pyrun)
            throw Py::Exception(); // throw exception
        Py_XDECREF(pyrun); // free the reference.
        */

        krossdebug( QString("PythonScript::initialize() name=%1").arg(m_action->objectName()) );
        //PyCompilerFlags* cf = new PyCompilerFlags;
        //cf->cf_flags |= PyCF_SOURCE_IS_UTF8;

        { // Compile the python script code. It will be later on request executed. That way we cache the compiled code.
            PyObject* code = Py_CompileString(
                (char*) m_action->getCode().toLatin1().data(),
                (char*) m_action->objectName().toLatin1().data(),
                Py_file_input
            );
            if(! code)
                throw Py::Exception();
            d->m_code = new Py::Object(code, true);
        }
    }
    catch(Py::Exception& e) {
        setErrorFromException( Py::value(e).as_string().c_str() );
        e.clear(); // exception is handled. clear it now.
        return false;
    }
    return true;
}

void PythonScript::finalize()
{
    #ifdef KROSS_PYTHON_SCRIPT_FINALIZE_DEBUG
        krossdebug( QString("PythonScript::finalize() module=%1").arg(d->m_module ? d->m_module->as_string().c_str() : "NULL") );
    #endif

    delete d->m_module; d->m_module = 0;
    delete d->m_code; d->m_code = 0;
}

void PythonScript::setErrorFromException(const QString& error)
{
    long lineno = -1;
    QStringList errorlist;

    PyObject *type, *value, *traceback;
    PyErr_Fetch(&type, &value, &traceback);
    Py_FlushLine();
    PyErr_NormalizeException(&type, &value, &traceback);

    if(traceback) {
        Py::List tblist;
        try {
            Py::Module tbmodule( PyImport_Import(Py::String("traceback").ptr()), true );
            Py::Dict tbdict = tbmodule.getDict();
            Py::Callable tbfunc(tbdict.getItem("format_tb"));
            Py::Tuple args(1);
            args.setItem(0, Py::Object(traceback));
            tblist = tbfunc.apply(args);
            uint length = tblist.length();
            for(Py::List::size_type i = 0; i < length; ++i)
                errorlist.append( Py::Object(tblist[i]).as_string().c_str() );
        }
        catch(Py::Exception& e) {
            QString err = Py::value(e).as_string().c_str();
            e.clear(); // exception is handled. clear it now.
            krosswarning( QString("Kross::PythonScript::toException() Failed to fetch a traceback: %1").arg(err) );
        }

        PyObject *next;
        while (traceback && traceback != Py_None) {
            PyFrameObject *frame = (PyFrameObject*)PyObject_GetAttrString(traceback, const_cast< char* >("tb_frame"));
            Py_DECREF(frame);
            {
                PyObject *getobj = PyObject_GetAttrString(traceback, const_cast< char* >("tb_lineno") );
                lineno = PyInt_AsLong(getobj);
                Py_DECREF(getobj);
            }
            if(Py_OptimizeFlag) {
                PyObject *getobj = PyObject_GetAttrString(traceback, const_cast< char* >("tb_lasti") );
                int lasti = PyInt_AsLong(getobj);
                Py_DECREF(getobj);
                lineno = PyCode_Addr2Line(frame->f_code, lasti);
            }

            //const char* filename = PyString_AsString(frame->f_code->co_filename);
            //const char* name = PyString_AsString(frame->f_code->co_name);
            //errorlist.append( QString("%1#%2: \"%3\"").arg(filename).arg(lineno).arg(name) );

            next = PyObject_GetAttrString(traceback, const_cast< char* >("tb_next") );
            Py_DECREF(traceback);
            traceback = next;
        }
    }

    if(lineno < 0) {
        if(value) {
            PyObject *getobj = PyObject_GetAttrString(value, const_cast< char* >("lineno") );
            if(getobj) {
                lineno = PyInt_AsLong(getobj);
                Py_DECREF(getobj);
            }
        }
        if(lineno < 0)
            lineno = 0;
    }

    //PyErr_Restore(type, value, traceback);
    setError(error, errorlist.join("\n"), lineno - 1);
}

void PythonScript::execute(const QVariant& args)
{
    #ifdef KROSS_PYTHON_SCRIPT_EXEC_DEBUG
        krossdebug( QString("PythonScript::execute() args=%1").arg(args.toString()) );
    #endif

    clearError(); // clear previous errors.

    if(! d->m_module) { // initialize if not already done before.
        if(! initialize())
            return;
    }

    Q_ASSERT( d->m_code && d->m_code->ptr() );

    try {

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

        PyObject* pyrun = PyRun_String(s.toLatin1().data(), Py_file_input, mainmoduledict.ptr(), moduledict.ptr());
        if(! pyrun)
            throw Py::Exception(); // throw exception
        Py_XDECREF(pyrun); // free the reference.

        // Acquire interpreter lock
        PyGILState_STATE gilstate = PyGILState_Ensure();

        // Evaluate the already compiled code.
        PyObject* pyresult = PyEval_EvalCode(
            (PyCodeObject*)d->m_code->ptr(),
            mainmoduledict.ptr(),
            moduledict.ptr()
        );

        // Free interpreter lock
        PyGILState_Release(gilstate);

        if(! pyresult)
            throw Py::Exception();
        Py::Object result(pyresult, true);
        if(PyErr_Occurred())
            throw Py::Exception();

        #ifdef KROSS_PYTHON_SCRIPT_EXEC_DEBUG
            krossdebug( QString("PythonScript::execute() result=%1").arg(result.as_string().c_str()) );
        #endif
        //Kross::Object* r = PythonExtension::toObject(result);
        //return Kross::Object::Ptr(r);
    }
    catch(Py::Exception& e) {
        Py::Object errobj = Py::value(e);
        if(errobj.ptr() == Py_None) // e.g. string-exceptions have there errormessage in the type-object
            errobj = Py::type(e);
        setError( errobj.as_string().c_str() );
        PyErr_Clear(); // exception is handled.
    }
}

QStringList PythonScript::functionNames()
{
    if(! d->m_module) { // initialize if not already done before.
        if(! initialize())
            return QStringList();
    }
    QStringList functions;
    Py::Dict moduledict = d->m_module->getDict();
    for(Py::Dict::iterator it = moduledict.begin(); it != moduledict.end(); ++it) {
        Py::Dict::value_type vt(*it);
        //if(PyClass_Check( vt.second.ptr() )) continue; // ignore classes since they are callable too
        if(vt.second.isCallable())
            functions.append( vt.first.as_string().c_str() );
    }
    return functions;
}

QVariant PythonScript::callFunction(const QString& name, const QVariantList& args)
{
    #ifdef KROSS_PYTHON_SCRIPT_CALLFUNC_DEBUG
        QString s;
        foreach(QVariant v, args)
            s += v.toString() + ",";
        krossdebug( QString("PythonScript::callFunction() name=%1 args=[%2]").arg(name).arg(s) );
    #endif

    clearError(); // clear previous errors.

    if(! d->m_module) { // initialize if not already done before.
        if(! initialize())
            return QStringList();
    }

    if(hadError()) {
        #ifdef KROSS_PYTHON_SCRIPT_CALLFUNC_DEBUG
            krossdebug( QString("PythonScript::callFunction() name=%1 had errors: %2").arg(name).arg(errorMessage()) );
        #endif
        return QVariant();
    }

    try {
        Py::Dict moduledict = d->m_module->getDict();

        // Try to determinate the function we like to execute.
        PyObject* func = PyDict_GetItemString(moduledict.ptr(), name.toLatin1().data());
        if(! func) {
            setError( QString("No such function '%1'.").arg(name) );
            return QVariant();
        }

        // Check if the object is really a function and callable.
        Py::Callable funcobject(func, true);
        if(! funcobject.isCallable()) {
            setError( QString("Function '%1' is not callable.").arg(name) );
            return QVariant();
        }

        // Finally call the function.
        Py::Object result = funcobject.apply( PythonType<QVariantList,Py::Tuple>::toPyObject(args) );
        //TODO return Kross::Object::Ptr( PythonExtension::toObject(result) );
        krossdebug( QString("PythonScript::callFunction() result=%1").arg(result.as_string().c_str()) );
    }
    catch(Py::Exception& e) {
        setError( Py::value(e).as_string().c_str() );
        e.clear(); // exception is handled. clear it now.
    }
    return QVariant();
}

#if 0
const QStringList& PythonScript::getClassNames()
{
    if(! d->m_module) if(! initialize()) return false; //TODO catch exception
    return d->m_classes;
}
Kross::Object::Ptr PythonScript::classInstance(const QString& name)
{
    if(hadException()) return Kross::Object::Ptr(0); // abort if we had an unresolved exception.
    if(! d->m_module) {
        setException( new Kross::Exception(QString("Script not initialized.")) );
        return Kross::Object::Ptr(0);
    }
    try {
        Py::Dict moduledict = d->m_module->getDict();
        // Try to determinate the class.
        PyObject* pyclass = PyDict_GetItemString(moduledict.ptr(), name.toLatin1().data());
        if( (! d->m_classes.contains(name)) || (! pyclass) ) throw Kross::Exception::Ptr( new Kross::Exception(QString("No such class '%1'.").arg(name)) );
        PyObject *pyobj = PyInstance_New(pyclass, 0, 0);//aclarg, 0);
        if(! pyobj) throw Kross::Exception::Ptr( new Kross::Exception(QString("Failed to create instance of class '%1'.").arg(name)) );
        Py::Object classobject(pyobj, true);
        #ifdef KROSS_PYTHON_SCRIPT_CLASSINSTANCE_DEBUG
            krossdebug( QString("PythonScript::classInstance() inst='%1'").arg(classobject.as_string().c_str()) );
        #endif
        return Kross::Object::Ptr( PythonExtension::toObject(classobject) );
    }
    catch(Py::Exception& e) {
        QString err = Py::value(e).as_string().c_str();
        e.clear(); // exception is handled. clear it now.
        setException( new Kross::Exception(err) );
    }
    catch(Kross::Exception::Ptr e) {
        setException(e.data());
    }
    return Kross::Object::Ptr(0); // return nothing if exception got thrown.
}
#endif

