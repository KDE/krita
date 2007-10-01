/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QFile>
#include <QProcess>
#include <QTemporaryFile>

#include <KAboutData>
#include <KCmdLineArgs>
#include <KApplication>

#include "KoColorSpaceRegistry.h"
#include "KoColorConversionSystem.h"

#include <iostream>

int main(char argc, char** argv)
{
    KAboutData aboutData("CSSGraph",
                         0,
                         ki18n("CSSGraph"),
                         "1.0",
                         ki18n("Output the graph of color conversion of pigment's Color Conversion"),
                         KAboutData::License_LGPL,
                         ki18n("(c) 2007 Cyrille Berger"),
                         ki18n(""),
                         "www.koffice.org",
                         "submit@bugs.kde.org");
    KCmdLineArgs::init( argc, argv, &aboutData );
    // Initialize the list of options
    KCmdLineOptions options;
    options.add("graphs", ki18n("return the list of available graphs"));
    options.add("graph <type>", ki18n("specify the type of graph (see --graphs to get the full list, the default is full)"), "full");
    options.add("output <type>", ki18n("specify the output (can be ps or dot, the default is ps)"), "ps");
    options.add("+outputfile", ki18n("name of the output file"));
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if( args->isSet("graphs"))
    {
        std::cout << "full : show all the connection on the graph" << std::endl;
        std::cout << "bestpath : show the best path for a given transformation" << std::endl;
        exit(0);
    }
    QString graphType = args->getOption("graph");
    QString outputType = args->getOption("output");
    if( args->count() != 1 )
    {
        kError() << "No output file name specified";
        args->usage();
        exit(-1);
    }
    QString outputFileName = args->arg(0);
    // Generate the graph
    KApplication app;
    QString dot;
    if(graphType == "full")
    {
        dot = KoColorSpaceRegistry::instance()->colorConversionSystem()->toDot();
    } else if(graphType == "bestpath") {
        dot = KoColorSpaceRegistry::instance()->colorConversionSystem()->bestPathToDot("RGBA", "F32", "XYZA", "F32");
    } else {
        std::cout << "Unknow graph type : " << graphType.latin1() << std::endl;
        exit(-1);
    }
    if(outputType == "dot")
    {
        QFile file(outputFileName);
        if (not file.open(QIODevice::WriteOnly | QIODevice::Text))
            exit(-1);
        QTextStream out(&file);
        out << dot;
    } else if(outputType == "ps")
    {
        QTemporaryFile file;
        if (not file.open())
            exit(-1);
        QTextStream out(&file);
        out << dot;
        QString cmd = QString("dot -Tps %1 -o %2").arg(file.fileName()).arg(outputFileName);
        file.close();
        kDebug() << cmd;
        if(QProcess::execute(cmd) != 0)
        {
            kError() << "An error has occured when executing : '" << cmd << "' it's most likely that the command 'dot' is missing, and that you should install graphviz (from http://www.graphiz.org)";
        }
    } else {
        std::cout << "Unknow output type : " << outputType.latin1() << std::endl;
        exit(-1);
    }
}
