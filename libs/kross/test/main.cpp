/***************************************************************************
 * main.cpp
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

#include "testobject.h"
#include "testwindow.h"

// Kross
#include "../core/action.h"
#include "../core/interpreter.h"
#include "../core/manager.h"

// Qt
#include <QString>
#include <QFile>
#include <QMetaObject>
#include <QMetaMethod>

// KDE
#include <kdebug.h>
#include <kinstance.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <ksharedptr.h>

// for std namespace
#include <string>
#include <iostream>

#define ERROR_OK 0
#define ERROR_HELP -1
#define ERROR_NOSUCHFILE -2
#define ERROR_OPENFAILED -3
#define ERROR_NOINTERPRETER -4
#define ERROR_EXCEPTION -6

KApplication *app = 0;

static KCmdLineOptions options[] =
{
    { "gui", "Start the GUI; otherwise the command line application is used.", 0 },
    { "+file", "Scriptfile", 0 },

    //{ "functionname <functioname>", I18N_NOOP("Execute the function in the defined script file."), "" },
    //{ "functionargs <functioarguments>", I18N_NOOP("List of arguments to pass to the function on execution."), "" },
    { 0, 0, 0 }
};

QString getInterpreterName(const QString& scriptfile)
{
    Kross::InterpreterInfo* interpreterinfo = Kross::Manager::self().getInterpreterInfo( Kross::Manager::self().getInterpreternameForFile(scriptfile) );
    return interpreterinfo ? interpreterinfo->getInterpretername() : "python";
}

int readFile(const QString& scriptfile, QString& content)
{
    QFile f(QFile::encodeName(scriptfile));
    if(! f.exists()) {
        std::cerr << "No such scriptfile: " << scriptfile.toLatin1().data() << std::endl;
        return ERROR_NOSUCHFILE;
    }
    if(! f.open(QIODevice::ReadOnly)) {
        std::cerr << "Failed to open scriptfile: " << scriptfile.toLatin1().data() << std::endl;
        return ERROR_OPENFAILED;
    }
    content = f.readAll();
    f.close();
    return ERROR_OK;
}

static TestObject* testobj1 = 0;
static TestObject* testobj2 = 0;

void finishTestEnvironment()
{
    delete testobj1; testobj1 = 0;
    delete testobj2; testobj2 = 0;
}

void initTestEnvironment()
{
    // Create the testobject instances.
    testobj1 = new TestObject("TestObject1");
    testobj2 = new TestObject("TestObject2");

    // Publish both testobject instances.
    Kross::Manager::self().addObject( testobj1 );
    Kross::Manager::self().addObject( testobj2 );
}

int runScriptFile(const QString& scriptfile)
{
    // Read the scriptfile
    QString scriptcode;
    int result = readFile(scriptfile, scriptcode);
    if(result != ERROR_OK)
        return result;

    // Determinate the matching interpreter
    QString interpretername = getInterpreterName(scriptfile);

    // Create the testobject instances.
    TestObject* testobj3 = new TestObject("TestObject3");
    TestObject* testobj4 = new TestObject("TestObject4");

    // First we need a Action and fill it.
    Kross::Action::Ptr action = Kross::Manager::self().createAction(scriptfile);
    action->setInterpreterName(interpretername);
    action->setCode(scriptcode);

    // Publish other both testobject instance to the script.
    action->addObject( testobj3 );
    action->addObject( testobj4 );

    // Now execute the Action.
    std::cout << "Execute scriptfile " << scriptfile.toLatin1().data() << " now" << std::endl;
    action->trigger();
    std::cout << "Execution of scriptfile " << scriptfile.toLatin1().data() << " done" << std::endl;

    /*
    if(action->hadException()) {
        // We had an exception.
        QString errormessage = action->getException()->getError();
        QString tracedetails = action->getException()->getTrace();
        std::cerr << QString("%2\n%1").arg(tracedetails).arg(errormessage).toLatin1().data() << std::endl;
        return ERROR_EXCEPTION;
    }
    */

    delete testobj3;
    delete testobj4;

    return ERROR_OK;
}

int main(int argc, char **argv)
{
    int result = 0;

    KAboutData about("krosstest",
                     "KrossTest",
                     "0.1",
                     "KDE application to test the Kross framework.",
                     KAboutData::License_LGPL,
                     "(C) 2005 Sebastian Sauer",
                     "Test the Kross framework!",
                     "http://kross.dipe.org",
                     "kross@dipe.org");
    about.addAuthor("Sebastian Sauer", "Author", "mail@dipe.org");

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    QStringList scriptfiles;
    for(int i = 0; i < args->count(); i++)
        scriptfiles.append( QFile::decodeName(args->arg(i)) );

    //testcase
    if(scriptfiles.count() < 1)
        scriptfiles.append("testcase.py");

    if(scriptfiles.count() < 1) {
        std::cerr << "No scriptfile to execute defined. See --help" << std::endl;
        return ERROR_NOSUCHFILE;
    }

    if( args->isSet("gui") ) {
        app = new KApplication();

        QString interpretername, scriptcode;
        if(scriptfiles.count() > 0) {
            result = readFile( scriptfiles[0], scriptcode );
            if(result == ERROR_OK)
                interpretername = getInterpreterName( scriptfiles[0] );
        }

        if(result == ERROR_OK) {
            initTestEnvironment();

            TestWindow *mainWin = new TestWindow(interpretername, scriptcode);
            app->setMainWidget(mainWin);
            mainWin->show();
            args->clear();
            result = app->exec();
        }
    }
    else {
        app = new KApplication(true);
        initTestEnvironment();

        foreach(QString file, scriptfiles) {
            result = runScriptFile(file);
            if(result != ERROR_OK)
                break;
        }
    }

    finishTestEnvironment();
    delete app;

    kDebug() << "DONE!!!" << endl;
    return result;
}
