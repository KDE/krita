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

#ifndef PARAMETER_H__
#define PARAMETER_H__

#include <QMap>
#include <QString>
#include <QStringList>

class Parameter
{
public:
    enum ParameterType {
        INVALID_P,
        FLOAT_P,
        INT_P,
        BOOL_P,
        CHOICE_P,
        TEXT_P,
        FILE_P,
        FOLDER_P,
        COLOR_P,
        NOTE_P,
        LINK_P,
        SEPARATOR_P
    };

    Parameter(const QString &name, bool updatePreview = true);
    virtual ~Parameter(){}

    QString m_name;
    ParameterType m_type;
    bool m_updatePreview;

    virtual QString toString();
    // if the parameter is only GUI option, return null string
    virtual QString value() const
    {
        return QString();
    }
    virtual void parseValues(const QString& typeDefinition);

    QString name() const { return m_name; }

protected:
    QStringList getValues(const QString& typeDefinition);

};

static QMap<Parameter::ParameterType, QString> initMap()
{
    QMap<Parameter::ParameterType, QString> map;
    map.insert(Parameter::FLOAT_P,"float");
    map.insert(Parameter::INT_P, "int");
    map.insert(Parameter::BOOL_P, "bool");
    map.insert(Parameter::CHOICE_P, "choice");
    map.insert(Parameter::TEXT_P, "text");
    map.insert(Parameter::FILE_P, "file");
    map.insert(Parameter::FOLDER_P, "folder");
    map.insert(Parameter::COLOR_P, "color");
    map.insert(Parameter::NOTE_P, "note");
    map.insert(Parameter::LINK_P, "link");
    map.insert(Parameter::SEPARATOR_P, "separator");
    return map;
}

static const QMap<Parameter::ParameterType, QString> PARAMETER_NAMES = initMap();

static const QList<QString> PARAMETER_NAMES_STRINGS = PARAMETER_NAMES.values();

class FloatParameter : public Parameter
{
public:
    FloatParameter(const QString& name, bool updatePreview = true);

    float m_defaultValue;
    float m_value;
    float m_minValue;
    float m_maxValue;

    virtual QString value() const;
    virtual void parseValues(const QString& typeDefinition);
    virtual QString toString();
};

class IntParameter : public Parameter
{
public:
    IntParameter(const QString& name, bool updatePreview = true);
    virtual ~IntParameter(){}

    int m_defaultValue;
    int m_value;
    int m_minValue;
    int m_maxValue;

    virtual QString value() const;
    virtual void parseValues(const QString& typeDefinition);
    virtual QString toString();
};

class SeparatorParameter : public Parameter
{
public:
    SeparatorParameter(const QString& name, bool updatePreview = true);
    virtual void parseValues(const QString& typeDefinition);
    virtual QString toString();
};

class ChoiceParameter : public Parameter
{
public:
    ChoiceParameter(const QString& name, bool updatePreview = true);
    virtual void parseValues(const QString& typeDefinition);


    // default index
    int m_defaultValue;
    // current index
    int m_value;

    QStringList m_choices;

    virtual QString value() const;
    virtual QString toString();
};

class NoteParameter : public Parameter
{
public:
    NoteParameter(const QString& name, bool updatePreview = true);
    virtual void parseValues(const QString& typeDefinition);
    virtual QString toString();

    QString m_label;

};

#endif
