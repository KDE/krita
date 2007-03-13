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
#include <kross/core/action.h>

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
    delete d->m_module; d->m_module = 0;
    delete d->m_code; d->m_code = 0;
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

    try {
        if(action()->code().isNull()) {
            setError( QString("Invalid scripting code for script '%1'").arg(action()->objectName()) );
            return false;
        }
        if(action()->objectName().isNull()) {
            setError( QString("Name for the script is invalid!") );
            return false;
        }

        Q_ASSERT( ! action()->objectName().isNull() );
        QFileInfo fi( action()->objectName() );
        QString n = QFileInfo(fi.absolutePath(), fi.baseName()).absoluteFilePath();
        char* name = n.isNull() ? action()->objectName().toLatin1().data() : n.toLatin1().data();
        //krossdebug( QString("------------------> %1").arg(name) );

        { // Each Action uses an own module as scope.
            PyObject* pymod = PyModule_New(name);
            Q_ASSERT(pymod);
            PyModule_AddStringConstant(pymod, "__file__", name);

            //pymod = PyImport_AddModule(name);
            //pymod = PyImport_Import( Py::String(name).ptr() );
//pymod = PyImport_ImportModule(name);
PyObject* m = PyImport_ImportModule(name);
            //PyImport_ImportModuleEx(  char *name, PyObject *globals, PyObject *locals, PyObject *fromlist)

            d->m_module = new Py::Module(pymod, true);
            if(! d->m_module) {
                setError( QString("Failed to initialize local module context for script '%1'").arg(action()->objectName()) );
                return false;
            }

/*
            // Register in module dict to allow such codes like: from whatever import mymodule
            PyObject* importdict = PyImport_GetModuleDict();
            if(! importdict) {
                //finalize();
                setError( QString("Failed to fetch the import dictornary for script '%1'").arg(action()->objectName()) );
                return false;
            }
            PyDict_SetItemString(importdict, name, pymod);
*/

        }

        #ifdef KROSS_PYTHON_SCRIPT_INIT_DEBUG
            krossdebug( QString("PythonScript::initialize() module='%1' refcount='%2'").arg(d->m_module->as_string().c_str()).arg(d->m_module->reference_count()) );
        #endif

        { // Add additional stuff to the modules dictonary of this PythonScript.
            Py::Dict moduledict = d->m_module->getDict();

            // Add the builtin and main module.
            PythonModule* pythonmodule = ((PythonInterpreter*)interpreter())->mainModule();
            moduledict["__builtins__"] = pythonmodule->getDict()["__builtins__"];
            moduledict["__main__"] = pythonmodule->module();

            // Add our Action instance as "self" to the modules dictonary.
            moduledict[ "self" ] = Py::asObject(new PythonExtension(action()));

            // Add the QObject instances to the modules dictonary.
            QHashIterator< QString, QObject* > objectsit( action()->objects() );
            while(objectsit.hasNext()) {
                objectsit.next();
                moduledict[ objectsit.key().toLatin1().data() ] = Py::asObject(new PythonExtension(objectsit.value()));
            }

            // Set up the import hook
            //PyObject* pyrun1 = PyRun_String("__main__._Importer(self)", Py_file_input, moduledict.ptr(), moduledict.ptr());
            //if(! pyrun1) throw Py::Exception(); // throw exception
            //Py_XDECREF(pyrun1); // free the reference.

            // Add the script's directory to the sys.path
            if( ! action()->currentPath().isNull() ) {
                //FIXME: should we remove the dir again after the job is done?
                //FIXME: escape the path + propably do it like in pythoninterpreter.cpp?
                QString s = QString("import sys\nsys.path.append(\"%1\")").arg( action()->currentPath() );
                PyObject* pyrunsyspath = PyRun_String(s.toLatin1(), Py_file_input, moduledict.ptr(), moduledict.ptr());
                if(! pyrunsyspath) throw Py::Exception(); // throw exception
                Py_XDECREF(pyrunsyspath); // free the reference.
            }
        }

        /*
        // Prepare the local context.
        QString s =
            //"import sys\n"
            //"if self.has(\"stdout\"):\n"
            //"  self.stdout = Redirect( self.get(\"stdout\") )\n"
            //"if self.has(\"stderr\"):\n"
            //"  self.stderr = Redirect( self.get(\"stderr\") )\n"
            ;
        Py::Dict mainmoduledict = ((PythonInterpreter*)interpreter())->mainModule()->getDict();
        PyObject* pyrun = PyRun_StringFlags((char*)s.toLatin1().data(), Py_file_input, mainmoduledict.ptr(), moduledict.ptr());
        if(! pyrun)
            throw Py::Exception(); // throw exception
        Py_XDECREF(pyrun); // free the reference.
        */

        #ifdef KROSS_PYTHON_SCRIPT_INIT_DEBUG
            krossdebug( QString("PythonScript::initialize() name=%1").arg(action()->objectName()) );
        #endif

        { // Compile the python script code. It will be later on request executed. That way we cache the compiled code.
            //PyCompilerFlags* cf = new PyCompilerFlags;
            //cf->cf_flags |= PyCF_SOURCE_IS_UTF8;
            PyObject* code = Py_CompileString( //Py_CompileStringFlags(
                (char*) action()->code().toLatin1().data(),
                (char*) action()->objectName().toLatin1().data(),
                Py_file_input
                //,cf
            );
            //delete cf;
            if(! code)
                throw Py::Exception();
            d->m_code = new Py::Object(code, true);
        }
    }
    catch(Py::Exception& e) {
        QStringList trace;
        int lineno;
        PythonInterpreter::extractException(trace, lineno);
        setError(Py::value(e).as_string().c_str(), trace.join("\n"), lineno);
        PyErr_Print(); //e.clear();
        return false;
    }
    return true;
}

void PythonScript::finalize()
{
    #ifdef KROSS_PYTHON_SCRIPT_FINALIZE_DEBUG
        krossdebug( QString("PythonScript::finalize() module=%1").arg(d->m_module ? d->m_module->as_string().c_str() : "NULL") );
    #endif

    PyErr_Clear();
    clearError();
    delete d->m_module; d->m_module = 0;
    delete d->m_code; d->m_code = 0;
}

void PythonScript::execute()
{
    #ifdef KROSS_PYTHON_SCRIPT_EXEC_DEBUG
        krossdebug( QString("PythonScript::execute()") );
    #endif

    if( hadError() ) {
        #ifdef KROSS_PYTHON_SCRIPT_CALLFUNC_DEBUG
            krosswarning( QString("PythonScript::execute() Abort cause of prev error: %1\n%2").arg(errorMessage()).arg(errorTrace()) );
        #endif
        Py::AttributeError(errorMessage());
        return;
    }

    PyErr_Clear();
    //clearError(); // clear previous errors.

    if(! d->m_module) { // initialize if not already done before.
        if(! initialize())
            return;
    }

    Q_ASSERT( d->m_code && d->m_code->ptr() );

    try {

        // the main module dictonary.
        Py::Dict mainmoduledict = ((PythonInterpreter*)interpreter())->mainModule()->getDict();
        // the local context dictonary.
        Py::Dict moduledict( d->m_module->getDict().ptr() );

        /*
        // Initialize context before execution.
        QString s =
            "import sys, os\n"
            //"__import__ = __main__.__import__\n"
            //"if self.has(\"stdout\"):\n"
            //"  sys.stdout = Redirect( self.get(\"stdout\") )\n"
            //"if self.has(\"stderr\"):\n"
            //"  sys.stderr = Redirect( self.get(\"stderr\") )\n"
            ;

        PyObject* pyrun = PyRun_String(
            s.toLatin1().data(),
            Py_file_input,
            mainmoduledict.ptr(),
            moduledict.ptr()
        );
        if(! pyrun)
            throw Py::Exception(); // throw exception
        Py_XDECREF(pyrun); // free the reference.
        */

        // Acquire interpreter lock
        PyGILState_STATE gilstate = PyGILState_Ensure();

        // Evaluate the already compiled code.
        PyObject* pyresult = PyEval_EvalCode(
            (PyCodeObject*)d->m_code->ptr(),
            moduledict.ptr(), //mainmoduledict.ptr(),
            moduledict.ptr()
        );

        // Free interpreter lock
        PyGILState_Release(gilstate);

        // valgrind complains, let's check it explicit ;)
        Q_ASSERT( d->m_code->reference_count() == 1 );

        if(! pyresult)
            throw Py::Exception();
        Py::Object result(pyresult, true);
        if(PyErr_Occurred())
            throw Py::Exception();

        #ifdef KROSS_PYTHON_SCRIPT_EXEC_DEBUG
            krossdebug( QString("PythonScript::execute() result=%1").arg(result.as_string().c_str()) );
        #endif
        //return PythonExtension::toObject(result);
    }
    catch(Py::Exception& e) {
        QStringList trace;
        int lineno;
        PythonInterpreter::extractException(trace, lineno);
        setError(Py::value(e).as_string().c_str(), trace.join("\n"), lineno);
        PyErr_Print(); //e.clear();
/*
        Py::Object errobj = Py::value(e);
        if(errobj.ptr() == Py_None) // e.g. string-exceptions have there errormessage in the type-object
            errobj = Py::type(e);
        setError( errobj.as_string().c_str() );
        //PyErr_Clear(); // exception is handled.
        PyErr_Print();
*/
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
    //TODO do we need to acquire interpreter lock here?

    #ifdef KROSS_PYTHON_SCRIPT_CALLFUNC_DEBUG
        QString s;
        foreach(QVariant v, args)
            s += v.toString() + ',';
        krossdebug( QString("PythonScript::callFunction() name=%1 args=[%2]").arg(name).arg(s) );
    #endif

    if( hadError() ) {
        #ifdef KROSS_PYTHON_SCRIPT_CALLFUNC_DEBUG
            krosswarning( QString("PythonScript::callFunction() Abort cause of prev error: %1\n%2").arg(errorMessage()).arg(errorTrace()) );
        #endif
        Py::AttributeError( errorMessage() );
        return QVariant();
    }

    PyErr_Clear();
    //clearError(); // clear previous errors.

    if(! d->m_module) { // initialize if not already done before.
        if(! initialize())
            return QVariant();
        execute(); // execute if not already done before.
        if( hadError() )
            return QVariant();
    }

    try {
        Py::Dict moduledict = d->m_module->getDict();

        // Try to determinate the function we like to execute.
        PyObject* func = PyDict_GetItemString(moduledict.ptr(), name.toLatin1().data());
        if(! func) {
            Py::AttributeError( ::QString("No such function '%1'.").arg(name).toLatin1().constData() );
            //setError( QString("No such function '%1'.").arg(name) );
            //finalize();
            return QVariant();
        }

        // Check if the object is really a function and callable.
        Py::Callable funcobject(func, true);
        if(! funcobject.isCallable()) {
            Py::AttributeError( ::QString("Function '%1' is not callable.").arg(name).toLatin1().constData() );
            //setError( QString("Function '%1' is not callable.").arg(name) );
            //finalize();
            return QVariant();
        }

        // Finally call the function.
        Py::Object pyresult = funcobject.apply( PythonType<QVariantList,Py::Tuple>::toPyObject(args) );
        QVariant result = PythonType<QVariant>::toVariant(pyresult);
        #ifdef KROSS_PYTHON_SCRIPT_CALLFUNC_DEBUG
            krossdebug( QString("PythonScript::callFunction() result=%1 variant.toString=%2 variant.typeName=%3").arg(pyresult.as_string().c_str()).arg(result.toString()).arg(result.typeName()) );
        #endif
        //finalize();
        return result;
    }
    catch(Py::Exception& e) {
        #ifdef KROSS_PYTHON_SCRIPT_CALLFUNC_DEBUG
            krosswarning( QString("PythonScript::callFunction() Exception: %1").arg(Py::value(e).as_string().c_str()) );
        #endif
        QStringList trace;
        int lineno;
        PythonInterpreter::extractException(trace, lineno);
        setError(Py::value(e).as_string().c_str(), trace.join("\n"), lineno);
        PyErr_Print(); //e.clear();
        //setError( Py::value(e).as_string().c_str() );
        //e.clear(); // exception is handled. clear it now.
        //finalize();
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

