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

class Parameter;
class QString;
class Folder;

class KisGmicParser
{

public:
    /**
     * @param filePath path where is your gmic_def.gmic
     * */
    KisGmicParser(const QString& filePath);
    ~KisGmicParser();
    void start();

    enum ParsingStatus {PARSE_START, PARSE_FOLDER, PARSE_PARAM, PARSE_COMMAND};

private:
    bool isFolderName(const QString &line);
    bool isParameter(const QString &line);



private:
    QVector<int> cosik;
    QString m_fileName;
    ParsingStatus m_status;
};



class Folder
{
public:
    QString m_commandName;
    QString m_command;
    QString m_commandPreview;
    QList<Parameter*> m_parameters;

    void debug();
    bool isValid();

    void processFolderName(const QString &line);
    void processParameter(const QString &line);
private:
    QStringList breakIntoTokens(const QString &line);
};

#endif
