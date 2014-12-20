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
#include <QVector>
#include <QColor>

class Parameter
{
public:
    enum ParameterType {
        INVALID_P = -1,
        BOOL_P,
        BUTTON_P,
        CHOICE_P,
        COLOR_P,
        CONST_P,
        FILE_P,
        FLOAT_P,
        FOLDER_P,
        INT_P,
        LINK_P,
        NOTE_P,
        TEXT_P,
        SEPARATOR_P,
    };

    Parameter(const QString &name, bool updatePreview = true);
    virtual ~Parameter(){}

    QString m_name;
    ParameterType m_type;
    bool m_updatePreview;

    virtual QString toString();
    // if the parameter is only GUI option, return null string
    virtual QString value() const;
    virtual void setValue(const QString &value);

    virtual void parseValues(const QString& typeDefinition);

    QString name() const { return m_name; }
    bool isPresentationalOnly() const;

    virtual void reset() { };

protected:
    // strips parameter type (int, note, etc.) and enclosing brackets
    QString extractValues(const QString& typeDefinition);
    QStringList getValues(const QString& typeDefinition);
    QString stripQuotes(const QString& str);
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
    map.insert(Parameter::CONST_P,"const");
    map.insert(Parameter::BUTTON_P,"button");
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
    virtual void setValue(const QString& value);

    virtual void parseValues(const QString& typeDefinition);
    virtual QString toString();
    virtual void reset();
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
    virtual void setValue(const QString& value);

    virtual void parseValues(const QString& typeDefinition);
    virtual QString toString();
    // reset parameter to default value from gmic definition
    // some parameters do not need reset, e.g. const is not mutable
    virtual void reset();
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
    // you can use int or name, if it is int, it will be set as index,
    // if you use name of choice, index will be determined
    virtual void setValue(const QString& value);
    void setIndex(int i);
    virtual QString toString();
    virtual void reset();
};

class NoteParameter : public Parameter
{
public:
    NoteParameter(const QString& name, bool updatePreview = true);
    virtual void parseValues(const QString& typeDefinition);
    virtual QString toString();

    QString m_label;

};

class LinkParameter : public Parameter
{
public:
    LinkParameter(const QString& name, bool updatePreview = true);
    virtual void parseValues(const QString& typeDefinition);
    virtual QString toString();


    QString m_link;
};

class BoolParameter : public Parameter
{
public:
    BoolParameter(const QString& name, bool updatePreview = true);
    virtual void parseValues(const QString& typeDefinition);
    virtual QString toString();
    virtual QString value() const;
    virtual void reset();

    void initValue(bool value);

    bool m_value;
    bool m_defaultValue;

};

class ColorParameter : public Parameter
{
public:
    ColorParameter(const QString& name, bool updatePreview = true);
    virtual void parseValues(const QString& typeDefinition);
    virtual QString toString();
    virtual QString value() const;
    virtual void reset();

    QColor m_value;
    QColor m_defaultValue;
    bool m_hasAlpha;
};

class TextParameter : public Parameter
{
public:
    TextParameter(const QString& name, bool updatePreview = true);
    virtual void parseValues(const QString& typeDefinition);
    virtual QString toString();
    virtual QString value() const;
    virtual void reset();

    QString m_value;
    QString m_defaultValue;
    bool m_multiline;
};

class FolderParameter : public Parameter
{
public:
    FolderParameter(const QString& name, bool updatePreview = true);
    virtual void parseValues(const QString& typeDefinition);
    virtual QString toString();
    virtual QString value() const;
    virtual void reset();

    QString m_folderPath;
    QString m_defaultFolderPath;
};

class FileParameter : public Parameter
{
public:
    FileParameter(const QString& name, bool updatePreview = true);
    virtual void parseValues(const QString& typeDefinition);
    virtual QString toString();
    virtual QString value() const;
    virtual void reset();

    QString m_filePath;
    QString m_defaultFilePath;
};

class ConstParameter : public Parameter
{
public:
    ConstParameter(const QString& name, bool updatePreview = false);
    virtual void parseValues(const QString& typeDefinition);
    virtual QString toString();
    virtual QString value() const;

    QStringList m_values;
};

class ButtonParameter : public Parameter
{
public:
    ButtonParameter(const QString& name, bool updatePreview = false);
    virtual void parseValues(const QString& typeDefinition);
    virtual QString toString();
    virtual QString value() const;
    virtual void setValue(const QString& value);
    virtual void reset();
    void initValue(bool value);
    
    bool m_value;
    bool m_defaultValue;
};


#endif
