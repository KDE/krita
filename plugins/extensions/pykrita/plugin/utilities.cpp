// This file is part of PyKrita, Krita' Python scripting plugin.
//
// Copyright (C) 2006 Paul Giannaros <paul@giannaros.org>
// Copyright (C) 2012, 2013 Shaheed Haque <srhaque@theiet.org>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) version 3, or any
// later version accepted by the membership of KDE e.V. (or its
// successor approved by the membership of KDE e.V.), which shall
// act as a proxy defined in Section 6 of version 3 of the license.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library.  If not, see <http://www.gnu.org/licenses/>.
//

// config.h defines PYKRITA_PYTHON_LIBRARY, the path to libpython.so
// on the build system

#include "config.h"
#include "utilities.h"

#include <algorithm>

#include <cmath>
#include <Python.h>


#include <QDir>
#include <QLibrary>
#include <QString>
#include <QStringList>
#include <QVector>

#include <kconfigbase.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <KoResourcePaths.h>

#include <kis_debug.h>

#define THREADED 1

namespace PyKrita
{
namespace
{
#ifndef Q_OS_WIN
QLibrary* s_pythonLibrary = 0;
#endif
PyThreadState* s_pythonThreadState = 0;
}                                                           // anonymous namespace

const char* Python::PYKRITA_ENGINE = "pykrita";

Python::Python()
{
#if THREADED
    m_state = PyGILState_Ensure();
#endif
}

Python::~Python()
{
#if THREADED
    PyGILState_Release(m_state);
#endif
}

bool Python::prependStringToList(PyObject* const list, const QString& value)
{
    PyObject* const u = unicode(value);
    bool result = !PyList_Insert(list, 0, u);
    Py_DECREF(u);
    if (!result)
        traceback(QString("Failed to prepend %1").arg(value));
    return result;
}

bool Python::functionCall(const char* const functionName, const char* const moduleName)
{
    PyObject* const result = functionCall(functionName, moduleName, PyTuple_New(0));
    if (result)
        Py_DECREF(result);
    return bool(result);
}

PyObject* Python::functionCall(
    const char* const functionName
    , const char* const moduleName
    , PyObject* const arguments
)
{
    if (!arguments) {
        errScript << "Missing arguments for" << moduleName << functionName;
        return 0;
    }
    PyObject* const func = itemString(functionName, moduleName);
    if (!func) {
        errScript << "Failed to resolve" << moduleName << functionName;
        return 0;
    }
    if (!PyCallable_Check(func)) {
        traceback(QString("Not callable %1.%2").arg(moduleName).arg(functionName));
        return 0;
    }
    PyObject* const result = PyObject_CallObject(func, arguments);
    Py_DECREF(arguments);
    if (!result)
        traceback(QString("No result from %1.%2").arg(moduleName).arg(functionName));

    return result;
}

bool Python::itemStringDel(const char* const item, const char* const moduleName)
{
    PyObject* const dict = moduleDict(moduleName);
    const bool result = dict && PyDict_DelItemString(dict, item);
    if (!result)
        traceback(QString("Could not delete item string %1.%2").arg(moduleName).arg(item));
    return result;
}

PyObject* Python::itemString(const char* const item, const char* const moduleName)
{
    if (PyObject* const value = itemString(item, moduleDict(moduleName)))
        return value;

    errScript << "Could not get item string" << moduleName << item;
    return 0;
}

PyObject* Python::itemString(const char* item, PyObject* dict)
{
    if (dict)
        if (PyObject* const value = PyDict_GetItemString(dict, item))
            return value;
    traceback(QString("Could not get item string %1").arg(item));
    return 0;
}

bool Python::itemStringSet(const char* const item, PyObject* const value, const char* const moduleName)
{
    PyObject* const dict = moduleDict(moduleName);
    const bool result = dict && !PyDict_SetItemString(dict, item, value);
    if (!result)
        traceback(QString("Could not set item string %1.%2").arg(moduleName).arg(item));
    return result;
}

PyObject* Python::kritaHandler(const char* const moduleName, const char* const handler)
{
    if (PyObject* const module = moduleImport(moduleName))
        return functionCall(handler, "krita", Py_BuildValue("(O)", module));
    return 0;
}

QString Python::lastTraceback() const
{
    QString result;
    result.swap(m_traceback);
    return result;
}

bool Python::libraryLoad()
{
    // no-op on Windows
#ifndef Q_OS_WIN
    if (!s_pythonLibrary) {
        dbgScript << "Creating s_pythonLibrary" << PYKRITA_PYTHON_LIBRARY;
        s_pythonLibrary = new QLibrary(PYKRITA_PYTHON_LIBRARY);
        if (!s_pythonLibrary) {
            errScript << "Could not create" << PYKRITA_PYTHON_LIBRARY;
            return false;
        }

        s_pythonLibrary->setLoadHints(QLibrary::ExportExternalSymbolsHint);
        if (!s_pythonLibrary->load()) {
            errScript << QString("Could not load %1 -- Reason: %2").arg(PYKRITA_PYTHON_LIBRARY).arg(s_pythonLibrary->errorString());
            return false;
        }
    }
#endif
    return true;
}

bool Python::setPath(const QStringList& paths)
{
    if (Py_IsInitialized()) {
        warnScript << "Setting paths when Python interpreter is already initialized";
    }
#ifdef Q_OS_WIN
    constexpr char pathSeparator = ';';
#else
    constexpr char pathSeparator = ':';
#endif
    QString joinedPaths = paths.join(pathSeparator);
    // Append the default search path
    // TODO: Properly handle embedded Python
#ifdef Q_OS_WIN
    QString currentPaths;
    // Find embeddable Python
    // TODO: Don't hard-code the paths
    QDir pythonDir(KoResourcePaths::getApplicationRoot());
    if (pythonDir.cd("python")) {
        dbgScript << "Found embeddable Python at" << pythonDir.absolutePath();
        currentPaths = pythonDir.absolutePath() + pathSeparator
                     + pythonDir.absoluteFilePath("python36.zip");
    } else {
# if 1
        // Use local Python???
        currentPaths = QString::fromWCharArray(Py_GetPath());
        warnScript << "Embeddable Python not found.";
        warnScript << "Default paths:" << currentPaths;
# else
        // Or should we fail?
        errScript << "Embeddable Python not found, not setting Python paths";
        return false;
# endif
    }
#else
    QString currentPaths = QString::fromLocal8Bit(qgetenv("PYTHONPATH"));
#endif
    if (!currentPaths.isEmpty()) {
        joinedPaths = joinedPaths + pathSeparator + currentPaths;
    }
    dbgScript << "Setting paths:" << joinedPaths;
#ifdef Q_OS_WIN
    QVector<wchar_t> joinedPathsWChars(joinedPaths.size() + 1, 0);
    joinedPaths.toWCharArray(joinedPathsWChars.data());
    Py_SetPath(joinedPathsWChars.data());
#else
    qputenv("PYTHONPATH", joinedPaths.toLocal8Bit());
#endif
    return true;
}

void Python::ensureInitialized()
{
    if (Py_IsInitialized()) {
        warnScript << "Python interpreter is already initialized, not initializing again";
    } else {
        dbgScript << "Initializing Python interpreter";
        Py_InitializeEx(0);
        if (!Py_IsInitialized()) {
            errScript << "Could not initialise Python interpreter";
        }
#if THREADED
        PyEval_InitThreads();
        s_pythonThreadState = PyGILState_GetThisThreadState();
        PyEval_ReleaseThread(s_pythonThreadState);
#endif
    }
}

void Python::maybeFinalize()
{
    if (!Py_IsInitialized()) {
        warnScript << "Python interpreter not initialized, no need to finalize";
    } else {
#if THREADED
        PyEval_AcquireThread(s_pythonThreadState);
#endif
        Py_Finalize();
    }
}

void Python::libraryUnload()
{
    // no-op on Windows
#ifndef Q_OS_WIN
    if (s_pythonLibrary) {
        // Shut the interpreter down if it has been started.
        if (s_pythonLibrary->isLoaded()) {
            s_pythonLibrary->unload();
        }
        delete s_pythonLibrary;
        s_pythonLibrary = 0;
    }
#endif
}

PyObject* Python::moduleActions(const char* moduleName)
{
    return kritaHandler(moduleName, "moduleGetActions");
}

PyObject* Python::moduleConfigPages(const char* const moduleName)
{
    return kritaHandler(moduleName, "moduleGetConfigPages");
}

QString Python::moduleHelp(const char* moduleName)
{
    QString r;
    PyObject* const result = kritaHandler(moduleName, "moduleGetHelp");
    if (result) {
        r = unicode(result);
        Py_DECREF(result);
    }
    return r;
}

PyObject* Python::moduleDict(const char* const moduleName)
{
    PyObject* const module = moduleImport(moduleName);
    if (module)
        if (PyObject* const dictionary = PyModule_GetDict(module))
            return dictionary;

    traceback(QString("Could not get dict %1").arg(moduleName));
    return 0;
}

PyObject* Python::moduleImport(const char* const moduleName)
{
    PyObject* const module = PyImport_ImportModule(moduleName);
    if (module)
        return module;

    traceback(QString("Could not import %1").arg(moduleName));
    return 0;
}

void* Python::objectUnwrap(PyObject* o)
{
    PyObject* const arguments = Py_BuildValue("(O)", o);
    PyObject* const result = functionCall("unwrapinstance", "sip", arguments);
    if (!result)
        return 0;

    void* const r = reinterpret_cast<void*>(ptrdiff_t(PyLong_AsLongLong(result)));
    Py_DECREF(result);
    return r;
}

PyObject* Python::objectWrap(void* const o, const QString& fullClassName)
{
    const QString classModuleName = fullClassName.section('.', 0, -2);
    const QString className = fullClassName.section('.', -1);
    PyObject* const classObject = itemString(PQ(className), PQ(classModuleName));
    if (!classObject)
        return 0;

    PyObject* const arguments = Py_BuildValue("NO", PyLong_FromVoidPtr(o), classObject);
    PyObject* const result = functionCall("wrapinstance", "sip", arguments);

    return result;
}

// Inspired by http://www.gossamer-threads.com/lists/python/python/150924.
void Python::traceback(const QString& description)
{
    m_traceback.clear();
    if (!PyErr_Occurred())
        // Return an empty string on no error.
        // NOTE "Return a string?" really??
        return;

    PyObject* exc_typ;
    PyObject* exc_val;
    PyObject* exc_tb;
    PyErr_Fetch(&exc_typ, &exc_val, &exc_tb);
    PyErr_NormalizeException(&exc_typ, &exc_val, &exc_tb);

    // Include the traceback.
    if (exc_tb) {
        m_traceback = "Traceback (most recent call last):\n";
        PyObject* const arguments = PyTuple_New(1);
        PyTuple_SetItem(arguments, 0, exc_tb);
        PyObject* const result = functionCall("format_tb", "traceback", arguments);
        if (result) {
            for (int i = 0, j = PyList_Size(result); i < j; i++) {
                PyObject* const tt = PyList_GetItem(result, i);
                PyObject* const t = Py_BuildValue("(O)", tt);
                char* buffer;
                if (!PyArg_ParseTuple(t, "s", &buffer))
                    break;
                m_traceback += buffer;
            }
            Py_DECREF(result);
        }
        Py_DECREF(exc_tb);
    }

    // Include the exception type and value.
    if (exc_typ) {
        PyObject* const temp = PyObject_GetAttrString(exc_typ, "__name__");
        if (temp) {
            m_traceback += unicode(temp);
            m_traceback += ": ";
        }
        Py_DECREF(exc_typ);
    }

    if (exc_val) {
        PyObject* const temp = PyObject_Str(exc_val);
        if (temp) {
            m_traceback += unicode(temp);
            m_traceback += "\n";
        }
        Py_DECREF(exc_val);
    }
    m_traceback += description;

    QStringList l = m_traceback.split("\n");
    Q_FOREACH(const QString &s, l) {
        errScript << s;
    }
    /// \todo How about to show it somewhere else than "console output"?
}

PyObject* Python::unicode(const QString& string)
{
#if PY_MAJOR_VERSION < 3
    /* Python 2.x. http://docs.python.org/2/c-api/unicode.html */
    PyObject* s = PyString_FromString(PQ(string));
    PyObject* u = PyUnicode_FromEncodedObject(s, "utf-8", "strict");
    Py_DECREF(s);
    return u;
#elif PY_MINOR_VERSION < 3
    /* Python 3.2 or less. http://docs.python.org/3.2/c-api/unicode.html#unicode-objects */
# ifdef Py_UNICODE_WIDE
    return PyUnicode_DecodeUTF16((const char*)string.constData(), string.length() * 2, 0, 0);
# else
    return PyUnicode_FromUnicode(string.constData(), string.length());
# endif
#else /* Python 3.3 or greater. http://docs.python.org/3.3/c-api/unicode.html#unicode-objects */
    return PyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, string.constData(), string.length());
#endif
}

QString Python::unicode(PyObject* const string)
{
#if PY_MAJOR_VERSION < 3
    /* Python 2.x. http://docs.python.org/2/c-api/unicode.html */
    if (PyString_Check(string))
        return QString(PyString_AsString(string));
    else if (PyUnicode_Check(string)) {
        const int unichars = PyUnicode_GetSize(string);
# ifdef HAVE_USABLE_WCHAR_T
        return QString::fromWCharArray(PyUnicode_AsUnicode(string), unichars);
# else
#   ifdef Py_UNICODE_WIDE
        return QString::fromUcs4((const unsigned int*)PyUnicode_AsUnicode(string), unichars);
#   else
        return QString::fromUtf16(PyUnicode_AsUnicode(string), unichars);
#   endif
# endif
    } else return QString();
#elif PY_MINOR_VERSION < 3
    /* Python 3.2 or less. http://docs.python.org/3.2/c-api/unicode.html#unicode-objects */
    if (!PyUnicode_Check(string))
        return QString();

    const int unichars = PyUnicode_GetSize(string);
# ifdef HAVE_USABLE_WCHAR_T
    return QString::fromWCharArray(PyUnicode_AsUnicode(string), unichars);
# else
#   ifdef Py_UNICODE_WIDE
    return QString::fromUcs4(PyUnicode_AsUnicode(string), unichars);
#   else
    return QString::fromUtf16(PyUnicode_AsUnicode(string), unichars);
#   endif
# endif
#else /* Python 3.3 or greater. http://docs.python.org/3.3/c-api/unicode.html#unicode-objects */
    if (!PyUnicode_Check(string))
        return QString();

    const int unichars = PyUnicode_GetLength(string);
    if (0 != PyUnicode_READY(string))
        return QString();

    switch (PyUnicode_KIND(string)) {
    case PyUnicode_1BYTE_KIND:
        return QString::fromLatin1((const char*)PyUnicode_1BYTE_DATA(string), unichars);
    case PyUnicode_2BYTE_KIND:
        return QString::fromUtf16(PyUnicode_2BYTE_DATA(string), unichars);
    case PyUnicode_4BYTE_KIND:
        return QString::fromUcs4(PyUnicode_4BYTE_DATA(string), unichars);
    default:
        break;
    }
    return QString();
#endif
}

bool Python::isUnicode(PyObject* const string)
{
#if PY_MAJOR_VERSION < 3
    return PyString_Check(string) || PyUnicode_Check(string);
#else
    return PyUnicode_Check(string);
#endif
}

void Python::updateConfigurationFromDictionary(KConfigBase* const config, PyObject* const dictionary)
{
    PyObject* groupKey;
    PyObject* groupDictionary;
    Py_ssize_t position = 0;
    while (PyDict_Next(dictionary, &position, &groupKey, &groupDictionary)) {
        if (!isUnicode(groupKey)) {
            traceback(QString("Configuration group name not a string"));
            continue;
        }
        QString groupName = unicode(groupKey);
        if (!PyDict_Check(groupDictionary)) {
            traceback(QString("Configuration group %1 top level key not a dictionary").arg(groupName));
            continue;
        }

        // There is a group per module.
        KConfigGroup group = config->group(groupName);
        PyObject* key;
        PyObject* value;
        Py_ssize_t x = 0;
        while (PyDict_Next(groupDictionary, &x, &key, &value)) {
            if (!isUnicode(key)) {
                traceback(QString("Configuration group %1 itemKey not a string").arg(groupName));
                continue;
            }
            PyObject* arguments = Py_BuildValue("(Oi)", value, 0);
            PyObject* pickled = functionCall("dumps", "pickle", arguments);
            if (pickled) {
#if PY_MAJOR_VERSION < 3
                QString ascii(unicode(pickled));
#else
                QString ascii(PyBytes_AsString(pickled));
#endif
                group.writeEntry(unicode(key), ascii);
                Py_DECREF(pickled);
            } else {
                errScript << "Cannot write" << groupName << unicode(key) << unicode(PyObject_Str(value));
            }
        }
    }
}

void Python::updateDictionaryFromConfiguration(PyObject* const dictionary, const KConfigBase* const config)
{
    qDebug() << config->groupList();
    Q_FOREACH(QString groupName, config->groupList()) {
        KConfigGroup group = config->group(groupName);
        PyObject* groupDictionary = PyDict_New();
        PyDict_SetItemString(dictionary, PQ(groupName), groupDictionary);
        Q_FOREACH(QString key, group.keyList()) {
            QString pickled = group.readEntry(key);
#if PY_MAJOR_VERSION < 3
            PyObject* arguments = Py_BuildValue("(s)", PQ(pickled));
#else
            PyObject* arguments = Py_BuildValue("(y)", PQ(pickled));
#endif
            PyObject* value = functionCall("loads", "pickle", arguments);
            if (value) {
                PyDict_SetItemString(groupDictionary, PQ(key), value);
                Py_DECREF(value);
            } else {
                errScript << "Cannot read" << groupName << key << pickled;
            }
        }
        Py_DECREF(groupDictionary);
    }
}

bool Python::prependPythonPaths(const QString& path)
{
    PyObject* sys_path = itemString("path", "sys");
    return bool(sys_path) && prependPythonPaths(path, sys_path);
}

bool Python::prependPythonPaths(const QStringList& paths)
{
    PyObject* sys_path = itemString("path", "sys");
    if (!sys_path)
        return false;

    /// \todo Heh, boosts' range adaptors would be good here!
    QStringList reversed_paths;
    std::reverse_copy(
        paths.begin()
        , paths.end()
        , std::back_inserter(reversed_paths)
    );

    Q_FOREACH(const QString & path, reversed_paths)
    if (!prependPythonPaths(path, sys_path))
        return false;

    return true;
}

bool Python::prependPythonPaths(const QString& path, PyObject* sys_path)
{
    Q_ASSERT("Dir entry expected to be valid" && sys_path);
    return bool(prependStringToList(sys_path, path));
}

}                                                           // namespace PyKrita

// krita: indent-width 4;
