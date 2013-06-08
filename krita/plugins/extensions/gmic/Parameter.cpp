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

#include <Parameter.h>
#include <QString>
#include <QStringList>

#include <QDebug>

Parameter::Parameter(const QString& name, bool updatePreview)
    :m_name(name),
     m_type(INVALID_P),
     m_updatePreview(updatePreview)
{

}

QString Parameter::toString()
{
    return "INVALID";
}

void Parameter::parseValues(const QString& typeDefinition)
{
    Q_UNUSED(typeDefinition);
}

QStringList Parameter::getValues(const QString& typeDefinition)
{
    QString currentType = PARAMETER_NAMES[m_type];
    Q_ASSERT(typeDefinition.startsWith(currentType));

    //qDebug() << currentType << currentType.size();

    // get rid of '(', '{' and '['
    QString onlyValues = typeDefinition;
    onlyValues = onlyValues.remove(0, currentType.size() + 1);
    onlyValues.chop(1);
    QStringList result = onlyValues.split(",");
    return result;
}



FloatParameter::FloatParameter(const QString& name, bool updatePreview): Parameter(name,updatePreview)
{
    m_type = FLOAT_P;
}

// e.g. float(0,0,5) or float[0,0,5] or float{0,0,5}
void FloatParameter::parseValues(const QString& typeDefinition)
{
    QStringList values = getValues(typeDefinition);
    bool isOk = true;
    //qDebug() << values;
    m_defaultValue = values.at(0).toFloat(&isOk);
    Q_ASSERT(isOk);
    m_minValue = values.at(1).toFloat(&isOk);
    Q_ASSERT(isOk);
    m_maxValue = values.at(2).toFloat(&isOk);
    Q_ASSERT(isOk);
}

QString FloatParameter::toString()
{
    QString result;
    result.append(m_name+";");
    result.append(PARAMETER_NAMES[m_type]+";");
    result.append(QString::number(m_defaultValue)+";");
    result.append(QString::number(m_minValue)+";");
    result.append(QString::number(m_maxValue)+";");
    return result;
}

IntParameter::IntParameter(const QString& name, bool updatePreview): Parameter(name, updatePreview)
{
    m_type = INT_P;
}

void IntParameter::parseValues(const QString& typeDefinition)
{
    QStringList values = getValues(typeDefinition);
    bool isOk = true;
    //qDebug() << values;
    m_defaultValue = values.at(0).toInt(&isOk);
    Q_ASSERT(isOk);
    m_minValue = values.at(1).toInt(&isOk);
    Q_ASSERT(isOk);
    m_maxValue = values.at(2).toInt(&isOk);
    Q_ASSERT(isOk);
}

QString IntParameter::toString()
{
    QString result;
    result.append(m_name+";");
    result.append(PARAMETER_NAMES[m_type]+";");
    result.append(QString::number(m_defaultValue)+";");
    result.append(QString::number(m_minValue)+";");
    result.append(QString::number(m_maxValue)+";");
    return result;
}


SeparatorParameter::SeparatorParameter(const QString& name, bool updatePreview): Parameter(name, updatePreview)
{
    m_type = SEPARATOR_P;
}

void SeparatorParameter::parseValues(const QString& typeDefinition)
{
    Q_UNUSED(typeDefinition);
}

QString SeparatorParameter::toString()
{
    QString result;
    result.append(m_name+";");
    return result;
}


ChoiceParameter::ChoiceParameter(const QString& name, bool updatePreview): Parameter(name, updatePreview)
{
    m_type = CHOICE_P;
}

void ChoiceParameter::parseValues(const QString& typeDefinition)
{
    m_choices = getValues(typeDefinition);
}

QString ChoiceParameter::toString()
{
    QString result;
    result.append(m_name+";");
    foreach (QString choice, m_choices)
    {
        result.append(choice+";");
    }
    return result;
}

QString ChoiceParameter::defaultChoice() const
{
    return m_choices.at(0);
}


NoteParameter::NoteParameter(const QString& name, bool updatePreview): Parameter(name, updatePreview)
{
    m_type = NOTE_P;
}

void NoteParameter::parseValues(const QString& typeDefinition)
{
    QStringList values = getValues(typeDefinition);
    m_label = values.at(0);
}

QString NoteParameter::toString()
{
    QString result;
    result.append(m_name+";");
    result.append(m_label+";");
    return result;
}

