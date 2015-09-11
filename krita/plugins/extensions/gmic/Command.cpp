/*
 * Copyright (c) 2013-2015 Lukáš Tvrdý <lukast.dev@gmail.com>
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
    setName(commandName.trimmed());

    QStringList commands = splittedLine[1].split(",");
    Q_ASSERT(commands.size() == 2);

    m_command = commands.at(0).trimmed();
    m_commandPreview = commands.at(1).trimmed();

    QStringList splitted = m_commandPreview.split("(");
    if (splitted.size() == 2)
    {
        m_commandPreview = splitted.at(0);
        m_commandPreviewZoom = splitted.at(1);
        m_commandPreviewZoom.chop(1);
    }

}


// sep = separator(),
// Preview type = choice("Full","Forward horizontal","Forward vertical","Backward horizontal","Backward vertical","Duplicate horizontal","Duplicate vertical")
QStringList Command::breakIntoTokens(const QString &line, bool &lastTokenEnclosed)
{
    QStringList result;
    lastTokenEnclosed = false;
    int index = 0;
    int lastIndex = 0;
    while (index < line.size())
    {
        // skip parameter name, find index of =
        index = line.indexOf("=", index);

        if (index == -1)
        {
            break;
        }

        // point to next character
        index++;

        // eat all spaces
        index = skipWhitespace(line, index);

        // skip type-definition name e.g. "int", "_note", etc.
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
            typeName.remove(0, 1);
        }

        if (!Parameter::isTypeDefined(typeName))
        {
            dbgPlugins << "Unknown type" << typeName << line;
        }

        index = helperIndex;

        // skip all whitespaces, e.g. note = note ("Example")
        index = skipWhitespace(line, index);

        // determine separator
        // Type separators '()' can be replaced by '[]' or '{}' if necessary ...
        QChar delimiter = line.at(index);
        QChar closingdelimiter;
        switch (delimiter.toLatin1())
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
                dbgPlugins << "Delimiter: " << delimiter << line;
                Q_ASSERT(false);
                break;
            }
        }

        // search for enclosing separator
        while (index < line.size() - 1)
        {
            if (line.at(index) != closingdelimiter)
            {
                index++;
            }
            else
            {
                // we found enclosing separator
                break;
            }
        }

        if (line.at(index) != closingdelimiter)
        {
            lastTokenEnclosed = false;
            //dbgPlugins << "Enclosing delimiter not found, trying again" << line.at(index);
            break;
        }
        else
        {
            lastTokenEnclosed = true;
            // "," removal only if there are n tokens in one line
            if (lastIndex != 0 )
            {
                // if there are more tokens on one line, they are separated by ",", so remove it here
                if (line.at(lastIndex) == ',')
                {
                    lastIndex++;
                }
            }

            QString token = line.mid(lastIndex, index + 1 - lastIndex).trimmed();
            result.append(token);
            lastIndex = index + 1;


        }
    } // while

    return result;
}


bool Command::processParameter(const QStringList& block)
{
    QString parameterLine = mergeBlockToLine(block);
    // remove " :"
    parameterLine = parameterLine.remove(0, 2).trimmed();

    /* State: one parameter one line
     * one parameter n lines
     * n parameters one line
    */

    // break into parameter tokens
    bool lastTokenEnclosed = true;
    QStringList tokens = breakIntoTokens(parameterLine, lastTokenEnclosed);



    if (!lastTokenEnclosed)
    {
        // we need more lines of command parameters
        //dbgPlugins << "ParameterLine not enclosed";
        return false;
    }

    static int unhandledParameters = 0;

    Parameter * parameter = 0;
    foreach (QString token, tokens)
    {

        QString paramName;
        QString typeDefinition;

        // determine parameter name and parameter definition
        processToken(token, paramName, typeDefinition);

        bool showPreviewOnChange = true;
        if (typeDefinition.startsWith("_"))
        {
            showPreviewOnChange = false;
            typeDefinition.remove(0,1);
        }

        if (typeDefinition.startsWith(Parameter::PARAMETER_NAMES[Parameter::FLOAT_P]))
        {
            parameter = new FloatParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(Parameter::PARAMETER_NAMES[Parameter::INT_P]))
        {
            parameter = new IntParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(Parameter::PARAMETER_NAMES[Parameter::SEPARATOR_P]))
        {
            parameter = new SeparatorParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(Parameter::PARAMETER_NAMES[Parameter::CHOICE_P]))
        {
            parameter = new ChoiceParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(Parameter::PARAMETER_NAMES[Parameter::TEXT_P]))
        {
            parameter = new TextParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(Parameter::PARAMETER_NAMES[Parameter::NOTE_P]))
        {
            parameter = new NoteParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(Parameter::PARAMETER_NAMES[Parameter::LINK_P]))
        {
            parameter = new LinkParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(Parameter::PARAMETER_NAMES[Parameter::BOOL_P]))
        {
            parameter = new BoolParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(Parameter::PARAMETER_NAMES[Parameter::COLOR_P]))
        {
            parameter = new ColorParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(Parameter::PARAMETER_NAMES[Parameter::FOLDER_P]))
        {
            parameter = new FolderParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(Parameter::PARAMETER_NAMES[Parameter::FILE_P]))
        {
            parameter = new FileParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(Parameter::PARAMETER_NAMES[Parameter::CONST_P]))
        {
            parameter = new ConstParameter(paramName, showPreviewOnChange);
        }
        else if (typeDefinition.startsWith(Parameter::PARAMETER_NAMES[Parameter::BUTTON_P]))
        {
            parameter = new ButtonParameter(paramName, showPreviewOnChange);
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

Component* Command::child(int index) const
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
    Q_UNUSED(column);
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


QString Command::buildCommand(const QString &baseCommand)
{
    // build list of parameters
    QString parameterList;
    foreach(Parameter * p, m_parameters)
    {
        if (!p->value().isNull())
        {
            parameterList.append(p->value() +",");
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

    if (parameterList.endsWith(","))
    {
        parameterList.chop(1);
    }

    QString command = "-" + baseCommand;
    // some commands might contain presentational parameters only
    if (!parameterList.isEmpty())
    {
        command.append(" ");
        command.append(parameterList);
    }

    return command;
}

void Command::writeConfiguration(KisGmicFilterSetting* setting)
{
    QString command = buildCommand(m_command);
    setting->setGmicCommand(command);

    QString commandPreview = buildCommand(m_commandPreview);
    setting->setPreviewGmicCommand(commandPreview);
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
        nextLine = nextLine.remove(0, 2).trimmed();
        result = result + nextLine;
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


bool Command::processToken(const QString& token, QString& parameterName, QString& parameterDefinition)
{
        // determine parameter name and parameter definition
        // dbgPlugins << "Processing token " << token;

        QString trimedToken = token.trimmed();
        // '=' separates parameter name and parameter definition
        // parameter definition can contain '=' so avoid split(") here
        int firstSeparatorIndex = trimedToken.indexOf("=");
        Q_ASSERT(firstSeparatorIndex != -1);
        parameterName = trimedToken.left(firstSeparatorIndex).trimmed();
        parameterDefinition = trimedToken.mid(firstSeparatorIndex + 1).trimmed();
        //dbgPlugins << ppVar(parameterName);
        //dbgPlugins << ppVar(parameterDefinition);
        return true;
}

int Command::skipWhitespace(const QString& line, int index)
{
    while (index < line.size())
    {
        if (line.at(index).isSpace())
        {
            index++;
        }
        else
        {
            break;
        }
    }
    return index;
}

void Command::setParameter(const QString& name, const QString& value)
{
    for (int i = 0; i < m_parameters.size(); i++)
    {
        if (m_parameters.at(i)->name() == name)
        {
            m_parameters[i]->setValue(value);
        }
    }
}

QString Command::parameter(const QString &name) const
{
    for (int i = 0; i < m_parameters.size(); i++)
    {
        if (m_parameters.at(i)->name() == name)
        {
            return m_parameters.at(i)->value();
        }
    }

    return QString();
}

bool Command::hasParameterName(const QString& paramName, const QString& paramType)
{
    Parameter::ParameterType type = Parameter::INVALID_P;
    if (!paramType.isEmpty())
    {
        type = Parameter::nameToType(paramType);
    }

    for (int i = 0; i < m_parameters.size(); i++)
    {
        Parameter * currentParameter = m_parameters.at(i);
        if (currentParameter->name() == paramName)
        {
            // if not empty, we check type also
            if (!paramType.isEmpty())
            {
                if (currentParameter->m_type == type)
                {
                    return true;
                }
                else
                {
                    dbgKrita << "Ignoring type " << currentParameter->m_type;
                }
            }
            else
            {
                return true;
            }
        }
    }
    return false;
}
