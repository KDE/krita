// This file is part of PyKrita, Krita' Python scripting plugin.
//
// A couple of useful macros and functions used inside of pykrita_engine.cpp and pykrita_plugin.cpp.
//
// SPDX-FileCopyrightText: 2006 Paul Giannaros <paul@giannaros.org>
// SPDX-FileCopyrightText: 2012, 2013 Shaheed Haque <srhaque@theiet.org>
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
//

#ifndef __PYKRITA_UTILITIES_H__
# define  __PYKRITA_UTILITIES_H__

#include <cmath>
#include <Python.h>
#include <QString>

class KConfigBase;

/// Save us some ruddy time when printing out QStrings with UTF-8
# define PQ(x) x.toUtf8().constData()

class PythonPluginManager;

namespace PyKrita
{
    enum InitResult {
        INIT_UNINITIALIZED,
        INIT_OK,
        INIT_CANNOT_LOAD_PYTHON_LIBRARY,
        INIT_CANNOT_SET_PYTHON_PATHS,
        INIT_CANNOT_LOAD_PYKRITA_MODULE,
    };

    /**
     * Initialize the Python environment and plugin manager.
     * This should be called first before using the manager
     * or the Python class.
     */
    InitResult initialize();

    /**
     * Gets the instance of the plugin manager.
     * Note: PyKrita::initialize() must be called
     * before using this function.
     */
    PythonPluginManager *pluginManager();

    /**
     * Cleanup after Python.
     * Note: doing this as part of static/global destruction will not
     * work. The call to Py_Finalize() would happen after the Python
     * runtime has already been finalized, leading to a segfault.
     */
    void finalize();

/**
 * Instantiate this class on the stack to automatically get and release the
 * GIL.
 *
 * Also, making all the utility functions members of this class means that in
 * many cases the compiler tells us where the class in needed. In the remaining
 * cases (i.e. bare calls to the Python C API), inspection is used to needed
 * to add the requisite Python() object. To prevent this object being optimised
 * away in these cases due to lack of use, all instances have the form of an
 * assignment, e.g.:
 *
 *      Python py = Python()
 *
 * This adds a little overhead, but this is a small price for consistency.
 */
class Python
{
public:
    Python();
    ~Python();

    /**
     * Load the Python shared library. This does nothing on Windows.
     */
    static bool libraryLoad();

    /**
     * Set the Python paths by calling Py_SetPath. This should be called before
     * initialization to ensure the proper libraries get loaded.
     */
    static bool setPath(const QStringList& scriptPaths);

    /**
     * Make sure the Python interpreter is initialized. Ideally should be only
     * called once.
     */
    static void ensureInitialized();

    /**
     * Finalize the Python interpreter. Not guaranteed to work.
     */
    static void maybeFinalize();

    /**
     * Unload the Python shared library. This does nothing on Windows.
     */
    static void libraryUnload();

    /// Convert a QString to a Python unicode object.
    static PyObject* unicode(const QString& string);

    /// Convert a Python unicode object to a QString.
    static QString unicode(PyObject* string);

    /// Test if a Python object is compatible with a QString.
    static bool isUnicode(PyObject* string);

    /// Prepend a QString to a list as a Python unicode object
    bool prependStringToList(PyObject* list, const QString& value);

    /**
     * Print and save (see @ref lastTraceback()) the current traceback in a
     * form approximating what Python would print:
     *
     * @verbatim
     * Traceback (most recent call last):
     *   File "/home/shahhaqu/.kde/share/apps/krita/pykrita/pluginmgr.py", line 13, in <module>
     *     import kdeui
     * ImportError: No module named kdeui
     * Could not import pluginmgr.
     * @endverbatim
     */
    void traceback(const QString& description);

    /**
     * Store the last traceback we handled using @ref traceback().
     */
    QString lastTraceback(void) const;

    /**
     * Call the named module's named entry point.
     */
    bool functionCall(const char* functionName, const char* moduleName = PYKRITA_ENGINE);
    PyObject* functionCall(const char* functionName, const char* moduleName, PyObject* arguments);

    /**
     * Delete the item from the named module's dictionary.
     */
    bool itemStringDel(const char* item, const char* moduleName = PYKRITA_ENGINE);

    /**
     * Get the item from the named module's dictionary.
     *
     * @return 0 or a borrowed reference to the item.
     */
    PyObject* itemString(const char* item, const char* moduleName = PYKRITA_ENGINE);

    /**
     * Get the item from the given dictionary.
     *
     * @return 0 or a borrowed reference to the item.
     */
    PyObject* itemString(const char* item, PyObject* dict);

    /**
     * Set the item in the named module's dictionary.
     */
    bool itemStringSet(const char* item, PyObject* value, const char* moduleName = PYKRITA_ENGINE);

    /**
     * Get the Actions defined by a module. The returned object is
     * [ { function, ( text, icon, shortcut, menu ) }... ] for each module
     * function decorated with @c action.
     *
     * @return 0 or a new reference to the result.
     */
    PyObject* moduleActions(const char* moduleName);

    /**
     * Get the ConfigPages defined by a module. The returned object is
     * [ { function, callable, ( name, fullName, icon ) }... ] for each module
     * function decorated with @c configPage.
     *
     * @return 0 or a new reference to the result.
     */
    PyObject* moduleConfigPages(const char* moduleName);

    /**
     * Get the named module's dictionary.
     *
     * @return 0 or a borrowed reference to the dictionary.
     */
    PyObject* moduleDict(const char* moduleName = PYKRITA_ENGINE);

    /**
     * Get the help text defined by a module.
     */
    QString moduleHelp(const char* moduleName);

    /**
     * Import the named module.
     *
     * @return 0 or a borrowed reference to the module.
     */
    PyObject* moduleImport(const char* moduleName);

    /**
     * Add a given path to to the front of \c PYTHONPATH
     *
     * @param path      A string (path) to be added
     * @return @c true on success, @c false otherwise.
     */
    bool prependPythonPaths(const QString& path);

    /**
     * Add listed paths to to the front of \c PYTHONPATH
     *
     * @param paths     A string list (paths) to be added
     * @return @c true on success, @c false otherwise.
     */
    bool prependPythonPaths(const QStringList& paths);

    static const char* PYKRITA_ENGINE;

private:
    /// @internal Helper function for @c prependPythonPaths overloads
    bool prependPythonPaths(const QString&, PyObject*);
    PyGILState_STATE m_state;
    mutable QString m_traceback;

    /**
     * Run a handler function supplied by the krita module on another module.
     *
     * @return 0 or a new reference to the result.
     */
    PyObject* kritaHandler(const char* moduleName, const char* handler);
};

}                                                           // namespace PyKrita
#endif                                                      //  __PYKRITA_UTILITIES_H__
// krita: indent-width 4;
