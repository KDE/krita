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

#include <Command.h>
#include <QString>
#include <QStringList>
#include <QChar>
#include <QList>

#include <kis_debug.h>
#include <kis_gmic_parser.h>

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <qslider.h>
#include <QSpinBox>
#include <QComboBox>
#include <QStringListModel>

#include <Parameter.h>

Command::Command(Component* parent): m_parent(parent)
{
}

Command::~Command()
{

}


void Command::processCommandName(const QString& line)
{
    QStringList splittedLine = line.split(":");
    Q_ASSERT(splittedLine.size() == 2);

    QString commandName = splittedLine.at(0);
    setName(commandName.remove(0, GIMP_COMMENT.size()).trimmed());

    QStringList commands = splittedLine[1].split(",");
    Q_ASSERT(commands.size() == 2);

    m_command = commands.at(0).trimmed();
    m_commandPreview = commands.at(1).trimmed();

}


// sep = separator(),
// Preview type = choice("Full","Forward horizontal","Forward vertical","Backward horizontal","Backward vertical","Duplicate horizontal","Duplicate vertical")
QStringList Command::breakIntoTokens(const QString &line, bool &lastTokenEnclosed)
{
    QStringList result;

    lastTokenEnclosed = true;
    int index = 0;
    int lastIndex = 0;
    while (index < line.size())
    {
        // find index of =
        index = line.indexOf("=", index);

        if (index == -1)
        {
            break;
        }

        // point to next character
        index++;

        // eat all spaces
        while (line.at(index).isSpace() && index < line.size())
        {
            index++;
        }

        // skip typedef
        const QChar underscore('_');
        int helperIndex = index;
        while (helperIndex < line.size())
        {
            const QChar c = line.at(helperIndex);
            if (c.isLetter() || c == underscore)
            {
                helperIndex++;
            }
            else
            {
                break;
            }

        }

        QString typeName = line.mid(index, helperIndex - index);
        if (typeName.startsWith(underscore))
        {
            typeName = typeName.mid(1);
        }

        const QList<QString> &typeDefs = PARAMETER_NAMES_STRINGS;
        if (!typeDefs.contains(typeName))
        {
            dbgPlugins << "Unknown type" << typeName << line;
        }

        index = helperIndex;

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
                //Q_ASSERT_X(false,"Unhandled separator", delimiter);
                break;
            }
        }

        while (line.at(index) != closingdelimiter && index < line.size())
        {
            index++;
        }

        if (line.at(index) != closingdelimiter)
        {
            lastTokenEnclosed = false;
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


bool Command::processParameter(const QStringList& block)
{
    QString parameterLine = mergeBlockToLine(block);
    // remove gimp prefix and " :"
    parameterLine = parameterLine.remove(0, GIMP_COMMENT.size()+2).trimmed();
    // break into parameter tokens
    bool lastTokenEnclosed = true;
    QStringList tokens = breakIntoTokens(parameterLine, lastTokenEnclosed);
    if (!lastTokenEnclosed)
    {
        // we need more lines of command parameters
        return false;
    }

    static int unhandledParameters = 0;

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
        }
        else if (typeDefinition.startsWith(PARAMETER_NAMES[Parameter::INT_P]))
        {
            parameter = new IntParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(PARAMETER_NAMES[Parameter::SEPARATOR_P]))
        {
            parameter = new SeparatorParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(PARAMETER_NAMES[Parameter::CHOICE_P]))
        {
            parameter = new ChoiceParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(PARAMETER_NAMES[Parameter::TEXT_P]))
        {
            parameter = new TextParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(PARAMETER_NAMES[Parameter::NOTE_P]))
        {
            parameter = new NoteParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(PARAMETER_NAMES[Parameter::LINK_P]))
        {
            parameter = new LinkParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(PARAMETER_NAMES[Parameter::BOOL_P]))
        {
            parameter = new BoolParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(PARAMETER_NAMES[Parameter::COLOR_P]))
        {
            parameter = new ColorParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(PARAMETER_NAMES[Parameter::FOLDER_P]))
        {
            parameter = new FolderParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(PARAMETER_NAMES[Parameter::FILE_P]))
        {
            parameter = new FileParameter(paramName, showPreviewOnChange);
        }
        else
        {
            unhandledParameters++;
            dbgPlugins << "Unhandled parameter " << unhandledParameters << parent()->name() << "\\" << name() << paramName << typeDefinition;
        }

        if (parameter)
        {
            parameter->parseValues(typeDefinition);
            m_parameters.append(parameter);
            parameter = 0;
        }


    }
    // let's ignore tokens
    return true;
}


void Command::add(Component* c)
{
    Q_UNUSED(c);
}


void Command::print(int level)
{
    for(int j=0; j < level; ++j)
    {
        dbgPlugins << "\t";
    }
    dbgPlugins << "Command : " << qPrintable(name());

    foreach(Parameter * p, m_parameters)
    {
        for(int j=0; j < level+1; ++j)
        {
            dbgPlugins << "\t";
        }
        QString str = p->toString();
        str.truncate(30);
        dbgPlugins << str;
    }
}

Component* Command::child(int index)
{
    Q_UNUSED(index);
    return 0;
}

Component* Command::parent()
{
    return m_parent;
}

int Command::row() const
{
    if (m_parent)
    {
        m_parent->indexOf(const_cast<Command *>(this));
    }
    return 0;
}

QVariant Command::data(int column)
{
    Q_ASSERT(column == 0);
    return name();
}

int Command::childCount() const
{
    return 0;
}

int Command::columnCount() const
{
    return 1;
}


void Command::writeConfiguration(KisGmicFilterSetting* setting)
{
    // example: -gimp_poster_edges 20,60,5,0,10,0,0
    QString command = "-" + m_command + " ";
    foreach(Parameter * p, m_parameters)
    {
        if (!p->value().isNull())
        {
            command.append(p->value() +",");
        }
        else
        {
            if (!p->isPresentationalOnly())
            {
                // implement for given parameter value()!
                dbgPlugins << "UNHANDLED command parameter: " << p->m_name << p->toString();
            }

        }
    }

    if (command.endsWith(","))
    {
        command.chop(1);
    }

    setting->setGmicCommand(command);
}

QString Command::mergeBlockToLine(const QStringList& block)
{
    Q_ASSERT(block.size() > 0);
    if (block.size() == 1)
    {
        return block.at(0);
    }

    QString result = block.at(0);
    for (int i = 1; i < block.size(); i++)
    {
        QString nextLine = block.at(i);
        nextLine = nextLine.remove(0, GIMP_COMMENT.size()+2).trimmed();
        result = result + "<br />" + nextLine;
    }
    return result;
}

void Command::reset()
{
    foreach(Parameter * p, m_parameters)
    {
        p->reset();
    }

}
