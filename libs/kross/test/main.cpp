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
    TestObject* testobj1 = new TestObject("TestObject1");
    TestObject* testobj2 = new TestObject("TestObject2");
    TestObject* testobj3 = new TestObject("TestObject3");
    TestObject* testobj4 = new TestObject("TestObject4");

    // Publish first both testobject instance to the manager.
    Kross::Manager::self().addObject( testobj1 );
    Kross::Manager::self().addObject( testobj2 );

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

    delete testobj1;
    delete testobj2;
    delete testobj3;
    delete testobj4;

    return ERROR_OK;
}

#if 0
void runInterpreter(const QString& interpretername, const QString& scriptcode)
{
    TestObject* testobj1 = new TestObject("MyTestObject1");
    TestObject* testobj2 = new TestObject("MyTestObject2");

    Kross::Action::Ptr script1 = Kross::Manager::self().createAction("MyAction1");
    script1->addObject( testobj1 );
    script1->addObject( testobj2 );
    script1->setInterpreterName(interpretername);
    script1->setCode(scriptcode);

    kDebug()<<"----------------------------------------------------------------------------------------"<<endl;
    script1->trigger();
    kDebug()<<"----------------------------------------------------------------------------------------"<<endl;

    kDebug()<<"Functionnames: "<<script1->functionNames().join(",")<<endl;
    QVariant arg1 = "MyFirstArgument";
    QVariant result = script1->callFunction("testobjectCallback",QVariantList() << arg1);
    kDebug()<<"===============> result="<<result.toString()<<endl;



    TestObject* testobj1 = new TestObject("TestObject1");
    TestObject* testobj2 = new TestObject("TestObject2");
    try {
        TestPluginObject* pluginobj1 = new TestPluginObject("TestPluginObject1");
        TestPluginObject* pluginobj2 = new TestPluginObject("TestPluginObject2");

        foreach(QString s, Kross::classFunctionNames( pluginobj1 ))
            kDebug() << "pluginobj1 function name=" << s << endl;
        foreach(QString s, Kross::classFunctionNames( pluginobj2 ))
            kDebug() << "pluginobj2 function name=" << s << endl;

/*
        {
            QList<QVariant> list;
            list.append(12345);
            list.append( qVariantFromValue(testobj1) );
            list.append("SomeString");
            QVariant result = pluginobj1->call("uintobjstringfunc",list);
            kDebug() << "pluginobj1.uintobjstringfunc() call-result value=" << result.toString() << " typename=" << result.typeName() << endl;
        }
*/

        {
            QList<QVariant> list;

Kross::Object* ooo = dynamic_cast<Kross::Object*>(pluginobj2);
Kross::Object::Ptr oooptr = Kross::Object::Ptr(ooo);
list.append( qVariantFromValue( oooptr ) );
////list.append( qVariantFromValue( pluginobj2 ) );


            QVariant vv = qVariantFromValue(oooptr);
            kDebug() << "0==> pluginobj1.objectfunc() vv=" << vv.toString() << " vv.typename=" << vv.typeName() << endl;

            QVariant result;
            QMetaObject::invokeMethod(pluginobj1, "objectfunc2", Q_RETURN_ARG(QVariant,result), Q_ARG(QVariant,vv));

            //QVariant result = pluginobj1->call("objectfunc2",list);
            kDebug() << "1==> pluginobj1.objectfunc() call-result value=" << result.toString() << " typename=" << result.typeName() << endl;

            Kross::Object::Ptr o = qVariantValue< Kross::Object::Ptr >(result);
            if(o) {
                kDebug() << "2==> object.objectName=" << o->objectName() << endl;
                kDebug() << "2==> object.metaObject.className=" << o->metaObject()->className() << endl;
                kDebug() << "2==> object.metaObject.classInfoCount=" << o->metaObject()->classInfoCount() << endl;
                kDebug() << "2==> object.metaObject.methodCount=" << o->metaObject()->methodCount() << endl;
kDebug() << "2==> object.metaObject.methodOffset=" << o->metaObject()->methodOffset() << endl;

                kDebug() << "2==> object.metaObject.propertyCount=" << o->metaObject()->propertyCount() << endl;
                kDebug() << "2==> object.metaObject.enumeratorCount=" << o->metaObject()->enumeratorCount() << endl;

                int count = o->metaObject()->methodCount();
                for(int i = 0; i < count || i > 100; ++i) {
                    const QMetaObject* mo = o->metaObject();
                    QMetaMethod mm = mo->method(i);
                    kDebug() << "  QMetaMethod i=" << i << " signature=" << mm.signature() << " tag=" << mm.tag() << " typeName=" << mm.typeName() << endl;
                }

QString s, t="hahaha";
//QMetaObject::invokeMethod(o.data(), "bbb", Q_RETURN_ARG(QString,s), Q_ARG(QString,t) );
QMetaObject::invokeMethod(o.data(), "bbb", Q_ARG(QVariant,t));
kDebug() << "CALLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL => " << s << endl;
            }

        }


        // Return the scriptingmanager instance. The manager is used as main
        // entry point to work with Kross.
        Kross::Manager* manager = Kross::Manager::self();

        // Add modules that should be accessible by scripting. Those
        // modules are wrappers around functionality you want to be
        // able to access from within scripts. You don't need to take
        // care of freeing them cause that will be done by Kross.
        // Modules are shared between the Action instances.
        Kross::Manager::self().addModule( Kross::Module::Ptr(new TestPluginModule("krosstestpluginmodule")) );

        // To represent a script that should be executed Kross uses
        // the Script container class. You are able to fill them with
        // what is needed and just execute them.
        Kross::Action::Ptr Action = Kross::Manager::self().getAction("MyScriptName");

        //Action->enableModule("KexiDB");

        Action->setInterpreterName(interpretername);
        Action->setCode(scriptcode);

        //TESTCASE
        TestObject* testobject = new TestObject(app, Action);
        Kross::Manager::self().addQObject( testobject );

        //TestAction* testaction = new TestAction(Action);
        //Kross::Manager::self().addQObject( testaction );

        /*Kross::Object* o =*/ Action->execute();

        // Call a function.
        //Action->callFunction("testobjectCallback" /*, Kross::List* functionarguments */);

        // Call a class.
        /*
        Kross::Object* testclassinstance = Action->classInstance("testClass");
        if(testclassinstance) {
            QValueList<Kross::Object*> ll;
            Kross::Object* instancecallresult = testclassinstance->call("testClassFunction1", Kross::List::create(ll));
            //krossdebug( QString("testClass.testClassFunction1 returnvalue => '%1'").arg( instancecallresult.toString() ) );
        }
        */

        /*
        // Connect QObject signal with scriptfunction.
        Action->connect(testobject, SIGNAL(testSignal()), "testobjectCallback");
        Action->connect(testobject, SIGNAL(testSignalString(const QString&)), "testobjectCallbackWithParams");
        // Call the testSlot to emit the testSignal.
        testobject->testSlot();
        */
    }
    catch(Kross::Exception::Ptr e) {
        std::cout << QString("EXCEPTION %1").arg(e->toString()).toLatin1().data() << std::endl;
    }

/*TESTCASE
    Kross::Action* sc2 = Kross::Manager::self().getAction("MyScriptName222");
    sc2->setInterpreterName(interpretername);
    sc2->setCode(scriptcode);
    try {
        sc2->execute();
    }
    catch(Kross::Exception& e) {
        krossdebug( QString("EXCEPTION type='%1' description='%2'").arg(e.type()).arg(e.description()) );
    }
    //delete sc2;
*/

    delete testobj1; delete testobj2;
    //std::string s; std::cin >> s; // just wait.
}
#endif

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
            int result = readFile( scriptfiles[0], scriptcode );
            if(result != ERROR_OK)
                return result;
            interpretername = getInterpreterName( scriptfiles[0] );
        }

        TestWindow *mainWin = new TestWindow(interpretername, scriptcode);
        app->setMainWidget(mainWin);
        mainWin->show();
        args->clear();
        result = app->exec();
    }
    else {
        app = new KApplication(true);
        foreach(QString file, scriptfiles) {
            result = runScriptFile(file);
            if(result != ERROR_OK)
                break;
        }
    }

    delete app;

    kDebug() << "DONE!!!" << endl;
    return result;
}
