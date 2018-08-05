/*
 *  Copyright (c) 2007-2008 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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
#include <QCoreApplication>

#include <DebugPigment.h>

#include "KoColorSpaceRegistry.h"
#include "KoColorConversionSystem.h"

#include <iostream>
#include <QCommandLineParser>
#include <QCommandLineOption>

struct FriendOfColorSpaceRegistry {
static QString toDot() {
    return KoColorSpaceRegistry::instance()->colorConversionSystem()->toDot();
}

static QString bestPathToDot(const QString &srcKey, const QString &dstKey) {
    return KoColorSpaceRegistry::instance()->colorConversionSystem()->bestPathToDot(srcKey, dstKey);
}
};

int main(int argc, char** argv)
{

    QCoreApplication app(argc, argv);

    QCommandLineParser parser;

    parser.addVersionOption();
    parser.addHelpOption();
    // Initialize the list of options
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("graphs"), i18n("return the list of available graphs")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("graph"), i18n("specify the type of graph (see --graphs to get the full list, the default is full)"), QLatin1String("type"), QLatin1String("full")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("key"), i18n("specify the key of the source color space"), QLatin1String("key"), QString()));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("key"), i18n("specify the key of the destination color space"), QLatin1String("key"), QString()));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("output"), i18n("specify the output (can be ps or dot, the default is ps)"), QLatin1String("type"), QLatin1String("ps")));
    parser.addPositionalArgument(QLatin1String("outputfile"), i18n("name of the output file"));
    parser.process(app); // PORTING SCRIPT: move this to after any parser.addOption

    if (parser.isSet("graphs")) {
        // Don't change those lines to use dbgPigment derivatives, they need to be outputted
        // to stdout not stderr.
        std::cout << "full : show all the connection on the graph" << std::endl;
        std::cout << "bestpath : show the best path for a given transformation" << std::endl;
        exit(EXIT_SUCCESS);
    }
    QString graphType = parser.value("graph");
    QString outputType = parser.value("output");
    if (parser.positionalArguments().count() != 1) {
        errorPigment << "No output file name specified";
        parser.showHelp();
        exit(EXIT_FAILURE);
    }
    QString outputFileName = parser.positionalArguments()[0];
    // Generate the graph

    QString dot;
    if (graphType == "full") {
        dot = FriendOfColorSpaceRegistry::toDot();
    } else if (graphType == "bestpath") {
        QString srcKey = parser.value("src-key");
        QString dstKey = parser.value("dst-key");
        if (srcKey.isEmpty() || dstKey.isEmpty()) {
            errorPigment << "src-key and dst-key must be specified for the graph bestpath";
            exit(EXIT_FAILURE);
        } else {
            dot = FriendOfColorSpaceRegistry::bestPathToDot(srcKey, dstKey);
        }
    } else {
        errorPigment << "Unknown graph type : " << graphType.toLatin1();
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
            errorPigment << "An error has occurred when executing : '" << cmd << "' the most likely cause is that 'dot' command is missing, and that you should install graphviz (from http://www.graphiz.org)";
        }
    } else {
        errorPigment << "Unknown output type : " << outputType;
        exit(EXIT_FAILURE);
    }
}
