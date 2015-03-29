/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com
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

#ifndef KIS_GMIC_PARSER_
#define KIS_GMIC_PARSER_

#include <QVector>

#include "Parameter.h"

#include <QString>
#include <QRegExp>

class Component;
class Category;
class Command;
class Parameter;

class QTextStream;

const static QString GIMP_COMMENT = "#@gimp";

class KisGmicParser
{

public:
    /**
     * @param filePath path where your *.gmic files are
     * */
    KisGmicParser(const QStringList& filePaths);
    ~KisGmicParser();
    /**
     * Creates tree of filters provided by filePaths
     */
    Component * createFilterTree();

    /**
     * Extracts gmic command definitions from .gmic file to be passed to gmic interpreter
     *
     * Gmic definition file mixes gmic commands and UI definitions for filters
     * For executing commands we have to pass to gmic gmic command definition and we don't need
     * to pass UI definitions which are ignored (gmic comments).
     *
     * This function strips all gmic comments: Each line starting with '#' is a comment line.
     */
    static QByteArray extractGmicCommandsOnly(const QString& filePath);


private:
    bool isCategory(const QString &line);
    /* Parses gmic command names for preview and for the filter itself */
    bool isCommand(const QString &line);
    bool isParameter(const QString &line);
    bool matchesRegExp(const QRegExp &regExp, const QString& line);
    QString parseCategoryName(const QString &line);
    QString fetchLine(QTextStream &input, int &lineCounter);

private:
    QStringList m_filePaths;
};

#endif
