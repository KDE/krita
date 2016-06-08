// This file is part of PyKrita, Krita' Python scripting plugin.
//
// A couple of useful macros and functions used inside of pykrita_engine.cpp and pykrita_plugin.cpp.
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

#ifndef __PYKRITA_UTILITIES_H__
# define  __PYKRITA_UTILITIES_H__

# include <Python.h>
# include <QString>

class KConfigBase;

/// Save us some ruddy time when printing out QStrings with UTF-8
# define PQ(x) x.toUtf8().constData()

namespace PyKrita
{

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
     * Load the Python interpreter.
     */
    static void libraryLoad();

    /**
     * Unload the Python interpreter.
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
     * Traceback (most recent call last):
     *   File "/home/shahhaqu/.kde/share/apps/krita/pykrita/pluginmgr.py", line 13, in <module>
     *     import kdeui
     * ImportError: No module named kdeui
     * Could not import pluginmgr.
     */
    void traceback(const QString& description);

    /**
     * Store the last traceback we handled using @ref traceback().
     */
    QString lastTraceback(void) const;

    /**
     * Create a Python dictionary from a KConfigBase instance, writing the
     * string representation of the values.
     */
    void updateDictionaryFromConfiguration(PyObject* dictionary, const KConfigBase* config);

    /**
     * Write a Python dictionary to a configuration object, converting objects
     * to their string representation along the way.
     */
    void updateConfigurationFromDictionary(KConfigBase* config, PyObject* dictionary);

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
     * function decorated with @action.
     *
     * @return 0 or a new reference to the result.
     */
    PyObject* moduleActions(const char* moduleName);

    /**
     * Get the ConfigPages defined by a module. The returned object is
     * [ { function, callable, ( name, fullName, icon ) }... ] for each module
     * function decorated with @configPage.
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
     * A void * for an arbitrary Qt/KDE object that has been wrapped by SIP. Nifty.
     *
     * @param o         The object to be unwrapped. The reference is borrowed.
     */
    void* objectUnwrap(PyObject* o);

    /**
     * A PyObject * for an arbitrary Qt/KDE object using SIP wrapping. Nifty.
     *
     * @param o         The object to be wrapped.
     * @param className The full class name of o, e.g. "PyQt4.QtGui.QWidget".
     * @return @c 0 or a new reference to the object.
     */
    PyObject* objectWrap(void* o, const QString& className);

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
