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

#include <QDebug>
#include <QFile>
#include <qregexp.h>
#include <QStringList>

#include "kis_gmic_parser.h"

#include <Parameter.h>

const QString GIMP_COMMENT = "#@gimp";

// regexp
const QRegExp FOLDER_NAME_RX("#@gimp\\s+\\w+[^:]+:\\s*\\w+,\\s*\\w+\\(?[0-2]?\\)?");
const QRegExp PARAMETER_RX("#@gimp\\s+:\\s*[\\w\\s]*=\\s*[\\w]*");

KisGmicParser::KisGmicParser(const QString& filePath):m_fileName(filePath)
{

}

KisGmicParser::~KisGmicParser()
{

}


bool KisGmicParser::isFolderName(const QString& line)
{
    int indexOfMatch = FOLDER_NAME_RX.indexIn(line);
    int matchedLength = FOLDER_NAME_RX.matchedLength();
    return (indexOfMatch == 0) && (matchedLength > 0);
}

bool KisGmicParser::isParameter(const QString& line)
{
    int indexOfMatch = PARAMETER_RX.indexIn(line);
    int matchedLength = PARAMETER_RX.matchedLength();
    return (indexOfMatch == 0) && (matchedLength > 0);
}



void KisGmicParser::start()
{
    QFile file(m_fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Can't open file " << m_fileName << file.errorString();
        return;
    }

    QTextStream in(&file);

    int folders = 0;
    m_status = PARSE_START;
    Folder * folder;
    while(!in.atEnd()) {
        QString line = in.readLine();

        if (m_status == PARSE_START)
        {
            // detect folder name
            if (isFolderName(line))
            {
                qDebug() << "Folder " << folders++ << line;
                folder = new Folder();
                folder->processFolderName(line);
                m_status = PARSE_PARAM;
                continue;
            }
        }

        if (m_status == PARSE_PARAM)
        {
            if (line.startsWith(GIMP_COMMENT))
            {
                if (isParameter(line))
                {
                    qDebug() << "Parameters" << line;
                    folder->processParameter(line);
                }


            } else if (line.startsWith("#"))
            {
                continue;
            } else {
                m_status = PARSE_COMMAND;
            }
        }

        if (m_status == PARSE_COMMAND)
        {
            
        }


    }
    folder->debug();

    file.close();
}


void Folder::debug()
{
    qDebug() <<"name"<< m_commandName <<"cmd"<< m_command <<"preview"<< m_commandPreview;
    foreach(Parameter * p, m_parameters)
    {
        qDebug() << p->toString();
    }

}


void Folder::processFolderName(const QString& line)
{
    QStringList splittedLine = line.split(":");
    Q_ASSERT(splittedLine.size() == 2);

    QString commandName = splittedLine.at(0);
    m_commandName = commandName.remove(0, GIMP_COMMENT.size()).trimmed();

    QStringList commands = splittedLine[1].split(",");
    Q_ASSERT(commands.size() == 2);

    m_command = commands.at(0).trimmed();
    m_commandPreview = commands.at(1).trimmed();

}


// sep = separator(),
// Preview type = choice("Full","Forward horizontal","Forward vertical","Backward horizontal","Backward vertical","Duplicate horizontal","Duplicate vertical")
QStringList Folder::breakIntoTokens(const QString& line)
{
    QStringList result;


    int index = 0;
    int lastIndex = 0;
    while (index < line.size()){
        // find index of =
        index = line.indexOf("=", index);
        qDebug() << "=" << " at " << index;

        if (index == -1)
        {
            qDebug() << "done, no more = at " << index;
            break;
        }

        // point to next character
        index++;

        // eat all spaces
        while (line.at(index).isSpace() && index < line.size())
        {
            index++;
        }

        qDebug() << "spaces eatin til " << index ;

        // skip typedef
        int helperIndex = index;
        while (line.at(helperIndex).isLetter() && helperIndex < line.size()){
            helperIndex++;
        }


        const QList<QString> &typeDefs = PARAMETER_NAMES_STRINGS;
        QString typeName = line.mid(index, helperIndex - index);
        if (typeDefs.contains(typeName)){
            // point to next character
            index = helperIndex;
            qDebug() << "found" << typeName;
        }else {
            qDebug() << "Unknown type" << typeName;
            qFatal("Kein chance");
        }


        // Type separators '()' can be replaced by '[]' or '{}' if necessary ...
        QChar delimiter = line.at(index);
        QChar closingdelimiter;
        switch (delimiter.toAscii())
        {
            case '(':
            {
                closingdelimiter = ')';
                break;
            }
            case '[':
            {
                closingdelimiter = ']';
                break;
            }
            case '{':
            {
                closingdelimiter = '}';
                break;
            }
            default:
            {
                Q_ASSERT_X(false,"Unhandled separator", delimiter);
                break;
            }
        }

        qDebug() << "delimiter" << delimiter << "closing" << closingdelimiter;

        while (line.at(index) != closingdelimiter && index < line.size())
        {
            index++;
        }

        // clean ","
        if (lastIndex != 0 )
        {
            if (line.at(lastIndex) == ',')
            {
                lastIndex++;
            }
        }
        QString token = line.mid(lastIndex, index + 1 - lastIndex).trimmed();
        result.append(token);
        lastIndex = index + 1;
    }

    return result;
}


void Folder::processParameter(const QString& line)
{
    QString parameterLine = line;
    // remove gimp prefix and " :"
    parameterLine = parameterLine.remove(0, GIMP_COMMENT.size()+2).trimmed();
    qDebug() << parameterLine;
    // break into parameter tokens
    QStringList tokens = breakIntoTokens(parameterLine);
    qDebug() << "tokens" << tokens.size() << tokens;

    Parameter * parameter = 0;
    foreach (QString token, tokens)
    {
        token = token.trimmed();
        QStringList tokenSplit = token.split("=");
        Q_ASSERT(tokenSplit.size() == 2);

        QString paramName = tokenSplit.at(0).trimmed();
        QString typeDefinition = tokenSplit.at(1).trimmed();
        bool showPreviewOnChange = true;

        if (typeDefinition.startsWith("_"))
        {
            showPreviewOnChange = false;
            typeDefinition.remove(0,1);
        }

        if (typeDefinition.startsWith(PARAMETER_NAMES[Parameter::FLOAT_P]))
        {
            parameter = new FloatParameter(paramName, showPreviewOnChange);
        } else if (typeDefinition.startsWith(PARAMETER_NAMES[Parameter::INT_P]))
        {
            parameter = new IntParameter(paramName, showPreviewOnChange);
        } else if (typeDefinition.startsWith(PARAMETER_NAMES[Parameter::SEPARATOR_P]))
        {
            parameter = new SeparatorParameter(paramName, showPreviewOnChange);
        } else if (typeDefinition.startsWith(PARAMETER_NAMES[Parameter::CHOICE_P]))
        {
            parameter = new ChoiceParameter(paramName, showPreviewOnChange);
        } else if (typeDefinition.startsWith(PARAMETER_NAMES[Parameter::NOTE_P]))
        {
            parameter = new NoteParameter(paramName, showPreviewOnChange);
        } else {
            qFatal("Unhandled parameter");
        }

        if (parameter)
        {
            parameter->parseValues(typeDefinition);
            m_parameters.append(parameter);
            parameter = 0;
        }


    }
    // let's ignore tokens

}

int main()
{
    KisGmicParser parser("gmic_def_poster.gmic");
    parser.start();
    return 0;
}

