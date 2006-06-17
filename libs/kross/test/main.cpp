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

#include "../main/manager.h"

#include "../api/object.h"
#include "../api/class.h"
#include "../api/module.h"
//#include "../api/script.h"
#include "../api/interpreter.h"

#include "../main/scriptcontainer.h"

//#include "../kexidb/kexidbmodule.h"

#include "testobject.h"
#include "testwindow.h"
#include "testplugin.h"

// Qt
#include <QString>
#include <QFile>

// KDE
#include <kinstance.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <ksharedptr.h>

// for std namespace
#include <string>
#include <iostream>

KApplication *app = 0;

static KCmdLineOptions options[] =
{
    { "interpreter <interpretername>", "Name of the interpreter being used", "python" },
    { "scriptfile <filename>", "Script file to execute with the defined interpreter", "testcase.py" },
    { "gui", "Start the GUI; otherwise the command line application is used.", 0 },

    //{ "functionname <functioname>", I18N_NOOP("Execute the function in the defined script file."), "" },
    //{ "functionargs <functioarguments>", I18N_NOOP("List of arguments to pass to the function on execution."), "" },
    { 0, 0, 0 }
};

void runInterpreter(const QString& interpretername, const QString& scriptcode)
{
    try {

        // Return the scriptingmanager instance. The manager is used as main
        // entry point to work with Kross.
        Kross::Api::Manager* manager = Kross::Api::Manager::scriptManager();

        // Add modules that should be accessible by scripting. Those
        // modules are wrappers around functionality you want to be
        // able to access from within scripts. You don't need to take
        // care of freeing them cause that will be done by Kross.
        // Modules are shared between the ScriptContainer instances.
        manager->addModule( Kross::Api::Module::Ptr(new TestPluginModule("krosstestpluginmodule")) );

        // To represent a script that should be executed Kross uses
        // the Script container class. You are able to fill them with
        // what is needed and just execute them.
        Kross::Api::ScriptContainer::Ptr scriptcontainer = manager->getScriptContainer("MyScriptName");

        //scriptcontainer->enableModule("KexiDB");

        scriptcontainer->setInterpreterName(interpretername);
        scriptcontainer->setCode(scriptcode);

        //TESTCASE
        TestObject* testobject = new TestObject(app, scriptcontainer);
        manager->addQObject( testobject );

        //TestAction* testaction = new TestAction(scriptcontainer);
        //manager->addQObject( testaction );

        /*Kross::Api::Object* o =*/ scriptcontainer->execute();

        // Call a function.
        //scriptcontainer->callFunction("testobjectCallback" /*, Kross::Api::List* functionarguments */);

        // Call a class.
        /*
        Kross::Api::Object* testclassinstance = scriptcontainer->classInstance("testClass");
        if(testclassinstance) {
            QValueList<Kross::Api::Object*> ll;
            Kross::Api::Object* instancecallresult = testclassinstance->call("testClassFunction1", Kross::Api::List::create(ll));
            //krossdebug( QString("testClass.testClassFunction1 returnvalue => '%1'").arg( instancecallresult.toString() ) );
        }
        */


/*
        // Connect QObject signal with scriptfunction.
        scriptcontainer->connect(testobject, SIGNAL(testSignal()), "testobjectCallback");
        scriptcontainer->connect(testobject, SIGNAL(testSignalString(const QString&)), "testobjectCallbackWithParams");
        // Call the testSlot to emit the testSignal.
        testobject->testSlot();
*/
    }
    catch(Kross::Api::Exception::Ptr e) {
        std::cout << QString("EXCEPTION %1").arg(e->toString()).toLatin1().data() << std::endl;
    }

/*TESTCASE
    Kross::Api::ScriptContainer* sc2 = manager->getScriptContainer("MyScriptName222");
    sc2->setInterpreterName(interpretername);
    sc2->setCode(scriptcode);
    try {
        sc2->execute();
    }
    catch(Kross::Api::Exception& e) {
        krossdebug( QString("EXCEPTION type='%1' description='%2'").arg(e.type()).arg(e.description()) );
    }
    //delete sc2;
*/

    std::string s; std::cin >> s; // just wait.
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
                     "http://www.dipe.org/kross",
                     "kross@dipe.org");
    about.addAuthor("Sebastian Sauer", "Author", "mail@dipe.org");

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    QString interpretername = args->getOption("interpreter");
    QString scriptfilename = args->getOption("scriptfile");

    QFile f(QFile::encodeName(scriptfilename));
    if(f.exists() && f.open(QIODevice::ReadOnly)) {
        QString scriptcode = f.readAll();
        f.close();

        if( args->isSet("gui") ) {
            app = new KApplication();
            TestWindow *mainWin = new TestWindow(interpretername, scriptcode);
            app->setMainWidget(mainWin);
            mainWin->show();
            args->clear();
            result = app->exec();
        }
        else {
            app = new KApplication(true);
            runInterpreter(interpretername, scriptcode);
        }
    }
    else {
        Kross::krosswarning( QString("Failed to load scriptfile: %1").arg(scriptfilename) );
        result = -1;
    }

    delete app;
    return result;
}
