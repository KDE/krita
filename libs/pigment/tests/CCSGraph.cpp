/*
 *  Copyright (c) 2007-2008 Cyrille Berger <cberger@cberger.net>
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
#include <kdebug.h>


#include "KoColorSpaceRegistry.h"
#include "KoColorConversionSystem.h"

#include <iostream>

int main(int argc, char** argv)
{
    KAboutData aboutData("CCSGraph",
                         0,
                         ki18n("CCSGraph"),
                         "1.0",
                         ki18n("Output the graph of color conversion of pigment's Color Conversion"),
                         KAboutData::License_LGPL,
                         ki18n("(c) 2007 Cyrille Berger"),
                         KLocalizedString(),
                         "www.koffice.org",
                         "submit@bugs.kde.org");
    KCmdLineArgs::init(argc, argv, &aboutData);
    // Initialize the list of options
    KCmdLineOptions options;
    options.add("graphs", ki18n("return the list of available graphs"));
    options.add("graph <type>", ki18n("specify the type of graph (see --graphs to get the full list, the default is full)"), "full");
    options.add("src-key <key>", ki18n("specify the key of the source color space"), "");
    options.add("dst-key <key>", ki18n("specify the key of the destination color space"), "");
    options.add("output <type>", ki18n("specify the output (can be ps or dot, the default is ps)"), "ps");
    options.add("+outputfile", ki18n("name of the output file"));
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("graphs")) {
        // Don't change those lines to use qDebug derivatives, they need to be outputed
        // to stdout not stderr.
        std::cout << "full : show all the connection on the graph" << std::endl;
        std::cout << "bestpath : show the best path for a given transformation" << std::endl;
        exit(EXIT_SUCCESS);
    }
    QString graphType = args->getOption("graph");
    QString outputType = args->getOption("output");
    if (args->count() != 1) {
        kError() << "No output file name specified";
        args->usage();
        exit(EXIT_FAILURE);
    }
    QString outputFileName = args->arg(0);
    // Generate the graph
    KApplication app;
    QString dot;
    if (graphType == "full") {
        dot = KoColorSpaceRegistry::instance()->colorConversionSystem()->toDot();
    } else if (graphType == "bestpath") {
        QString srcKey = args->getOption("src-key");
        QString dstKey = args->getOption("dst-key");
        if (srcKey == "" || dstKey == "") {
            kError() << "src-key and dst-key must be specified for the graph bestpath";
            exit(EXIT_FAILURE);
        } else {
            dot = KoColorSpaceRegistry::instance()->colorConversionSystem()->bestPathToDot(srcKey, dstKey);
        }
    } else {
        kError() << "Unknow graph type : " << graphType.toLatin1();
        exit(EXIT_FAILURE);
    }

    if (outputType == "dot") {
        QFile file(outputFileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            exit(EXIT_FAILURE);
        QTextStream out(&file);
        out << dot;
    } else if (outputType == "ps" || outputType == "svg") {
        QTemporaryFile file;
        if (!file.open()) {
            exit(EXIT_FAILURE);
        }
        QTextStream out(&file);
        out << dot;
        QString cmd = QString("dot -T%1 %2 -o %3").arg(outputType).arg(file.fileName()).arg(outputFileName);
        file.close();

        if (QProcess::execute(cmd) != 0) {
            kError() << "An error has occurred when executing : '" << cmd << "' the most likely cause is that 'dot' command is missing, and that you should install graphviz (from http://www.graphiz.org)";
        }
    } else {
        kError() << "Unknow output type : " << outputType;
        exit(EXIT_FAILURE);
    }
}
