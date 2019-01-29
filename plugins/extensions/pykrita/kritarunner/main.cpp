/*
 * Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdlib.h>

#include <QString>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <KisApplication.h>
#include <resources/KoHashGeneratorProvider.h>
#include "kis_md5_generator.h"
#include "PythonPluginManager.h"
#include <opengl/kis_opengl.h>

extern "C" int main(int argc, char **argv)
{
    // The global initialization of the random generator
    qsrand(time(0));
    KLocalizedString::setApplicationDomain("kritarunner");
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
    KisOpenGL::testingInitializeDefaultSurfaceFormat();


    // first create the application so we can create a pixmap
    KisApplication app("kritarunner", argc, argv);
    app.setApplicationDisplayName("Krita Script Runner");
    app.setApplicationName("kritarunner");
    app.setOrganizationDomain("krita.org");

    QCommandLineParser parser;
    parser.setApplicationDescription("kritarunner executes one python script and then returns.");
    parser.addVersionOption();
    parser.addHelpOption();

    QCommandLineOption scriptOption(QStringList() << "s" << "script", "The script to run. Do not append the .py extension.", "script");
    parser.addOption(scriptOption);

    QCommandLineOption functionOption(QStringList() << "f" << "function",
                                      "The function to call (by default __main__ is called).", "function", "__main__");
    parser.addOption(functionOption);

    parser.addPositionalArgument("[argument(s)]", "The arguments for the script");
    parser.process(app);

    if (!parser.isSet(scriptOption)) {
        qDebug("No script given, aborting.");
        return 1;
    }

    qDebug() << "running:" << parser.value(scriptOption) << parser.value(functionOption);
    qDebug() << parser.positionalArguments();

    KoHashGeneratorProvider::instance()->setGenerator("MD5", new KisMD5Generator());
    app.addResourceTypes();
    app.loadResources();
    app.loadPlugins();


    QByteArray pythonPath = qgetenv("PYTHONPATH");
    qDebug() << "\tPython path:" << pythonPath;

    qDebug() << "Creating engine";

    // TODO: refactor to share common parts with plugin.cpp

    PyKrita::InitResult initResult = PyKrita::initialize();

    switch (initResult) {
        case PyKrita::INIT_OK:
            break;
        case PyKrita::INIT_CANNOT_LOAD_PYTHON_LIBRARY:
            qWarning() << i18n("Cannot load Python library");
            return 1;
        case PyKrita::INIT_CANNOT_SET_PYTHON_PATHS:
            qWarning() << i18n("Cannot set Python paths");
            return 1;
        case PyKrita::INIT_CANNOT_LOAD_PYKRITA_MODULE:
            qWarning() << i18n("Cannot load built-in pykrita module");
            return 1;
        default:
            qWarning() << i18n("Unexpected error initializing python plugin.");
            return 1;
    }

    qDebug() << "Try to import the pykrita module";
    PyKrita::Python py = PyKrita::Python();
    PyObject* pykritaPackage = py.moduleImport("pykrita");
    pykritaPackage = py.moduleImport("krita");

    if (!pykritaPackage) {
        qDebug("Cannot load the PyKrita module, aborting");
        return 1;
    }

    PyObject *argsList = PyList_New(0);
    Q_FOREACH(const QString arg, parser.positionalArguments()) {
        PyObject* const u = py.unicode(arg);
        PyList_Append(argsList, u);
        Py_DECREF(u);
    }

    PyObject *args = PyTuple_New(1);
    PyTuple_SetItem(args, 0, argsList);

    py.functionCall(parser.value(functionOption).toUtf8().constData(), parser.value(scriptOption).toUtf8().constData(), args);

    Py_DECREF(argsList);
    Py_DECREF(args);

    app.quit();
    return 0;
}

