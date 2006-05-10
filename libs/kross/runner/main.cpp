/***************************************************************************
 * main.cpp
 * This file is part of the KDE project
 * copyright (C)2006 by Sebastian Sauer (mail@dipe.org)
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

// for std namespace
#include <string>
#include <iostream>

// Qt
#include <QString>
#include <QFile>

// KDE
#include <kinstance.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <ksharedptr.h>

// Kross
#include "../main/manager.h"
#include "../main/scriptcontainer.h"
#include "../api/interpreter.h"

#define ERROR_OK 0
#define ERROR_HELP -1
#define ERROR_NOSUCHFILE -2
#define ERROR_OPENFAILED -3
#define ERROR_NOINTERPRETER -4
#define ERROR_UNHALDEDEXCEPTION -5
#define ERROR_EXCEPTION -6

KApplication* app = 0;

int runScriptFile(const QString& scriptfile)
{
    // Read the scriptfile
    QFile f(QFile::encodeName(scriptfile));
    if(! f.exists()) {
        std::cerr << "No such scriptfile: " << scriptfile.toLatin1().data() << std::endl;
        return ERROR_NOSUCHFILE;
    }
    if(! f.open(QIODevice::ReadOnly)) {
        std::cerr << "Failed to open scriptfile: " << scriptfile.toLatin1().data() << std::endl;
        return ERROR_OPENFAILED;
    }
    QString scriptcode = f.readAll();
    f.close();

    // Determinate the matching interpreter
    Kross::Api::Manager* manager = Kross::Api::Manager::scriptManager();
    Kross::Api::InterpreterInfo* interpreterinfo = manager->getInterpreterInfo( manager->getInterpreternameForFile(scriptfile) );
    if(! interpreterinfo) {
        std::cerr << "No interpreter for file: " << scriptfile.toLatin1().data() << std::endl;
        return ERROR_NOINTERPRETER;
    }

    // Run the script.
    try {
        // First we need a scriptcontainer and fill it.
        Kross::Api::ScriptContainer::Ptr scriptcontainer = manager->getScriptContainer(scriptfile);
        scriptcontainer->setInterpreterName( interpreterinfo->getInterpretername() );
        scriptcontainer->setCode(scriptcode);
        // Now execute the scriptcontainer.
        scriptcontainer->execute();
        if(scriptcontainer->hadException()) {
            // We had an exception.
            QString errormessage = scriptcontainer->getException()->getError();
            QString tracedetails = scriptcontainer->getException()->getTrace();
            std::cerr << QString("%2\n%1").arg(tracedetails).arg(errormessage).toLatin1().data() << std::endl;
            return ERROR_EXCEPTION;
        }
    }
    catch(Kross::Api::Exception::Ptr e) {
        // Normaly that shouldn't be the case...
        std::cerr << QString("EXCEPTION %1").arg(e->toString()).toLatin1().data() << std::endl;
        return ERROR_UNHALDEDEXCEPTION;
    }
    return ERROR_OK;
}

int main(int argc, char **argv)
{
    int result = ERROR_OK;

    KAboutData about("krossrunner",
                     "krossrunner",
                     "0.1",
                     "KDE application to run Kross scripts.",
                     KAboutData::License_LGPL,
                     "(C) 2006 Sebastian Sauer",
                     "Run Kross scripts.",
                     "http://www.dipe.org/kross",
                     "kross@dipe.org");
    about.addAuthor("Sebastian Sauer", "Author", "mail@dipe.org");

    // Initialize command line args
    KCmdLineArgs::init(argc, argv, &about);
    // Tell which options are supported and parse them.
    static KCmdLineOptions options[] = {
        { "+file", "Scriptfile", 0 },
        KCmdLineLastOption
    };
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    // If no options are defined.
    if(args->count() < 1) {
        std::cout << "Syntax: " << KCmdLineArgs::appName() << " scriptfile1 [scriptfile2] [scriptfile3] ..." << std::endl;
        return ERROR_HELP;
    }

    // Create KApplication instance.
    app = new KApplication( /* GUIenabled */ true );

    //QString interpretername = args->getOption("interpreter");
    //QString scriptfilename = args->getOption("scriptfile");

    // Each argument is a scriptfile to open
    for(int i = 0; i < args->count(); i++) {
        result = runScriptFile(QFile::decodeName(args->arg(i)));
        if(result != ERROR_OK)
            break;
    }

    // Free the KApplication instance and exit the program.
    delete app;
    return result;
}
