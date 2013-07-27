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

#include <iostream>
#include "Parameter.h"

#include <QString>
#include <QRegExp>

class Component;
class Category;
class Command;
class Parameter;

const QString GIMP_COMMENT = "#@gimp";

// category match example : #@gimp _<b>Lights &amp; Shadows</b>
const QRegExp CATEGORY_NAME_RX("#@gimp\\s+[^:]+$");
// command match example: #@gimp Poster edges : gimp_poster_edges, gimp_poster_edges_preview(0)
const QRegExp COMMAND_NAME_RX("#@gimp\\s+\\w+[^:]+:\\s*\\w+,\\s*\\w+\\(?[0-2]?\\)?");
// parameter match example:  #@gimp : Fast approximation = bool(0)
//                           #@gimp : X-size = float(0.9,0,2)
const QRegExp PARAMETER_RX("#@gimp\\s+:\\s*[^=]*=\\s*[\\w]*");

class KisGmicParser
{

public:
    /**
     * @param filePath path where is your gmic_def.gmic
     * */
    KisGmicParser(const QString& filePath);
    ~KisGmicParser();
    Component * createFilterTree();

    enum ParsingStatus {PARSE_START, PARSE_FOLDER, PARSE_PARAM, PARSE_COMMAND};

private:
    bool isCategory(const QString &line);
    bool isCommand(const QString &line);
    bool isParameter(const QString &line);
    bool matchesRegExp(const QRegExp &regExp, const QString& line);
    QString parseCategoryName(const QString &line);


private:
    QVector<int> cosik;
    QString m_fileName;
    ParsingStatus m_status;
};




#endif
