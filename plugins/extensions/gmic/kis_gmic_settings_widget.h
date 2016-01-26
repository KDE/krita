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

#ifndef KIS_GMIC_SETTINGS_WIDGET
#define KIS_GMIC_SETTINGS_WIDGET

#include <QWidget>
#include <QHash>

#include <QUrl>

#include <Command.h>
#include <kis_config_widget.h>


/**
 * creates GUI for GMIC filter definition (Command)
 */

enum ROLE {
    CreateRole = 0,
    LoadRole = 1
};

class KisGmicSettingsWidget : public KisConfigWidget {
    Q_OBJECT
public:
    KisGmicSettingsWidget(Command * command = 0);
    ~KisGmicSettingsWidget();

    virtual KisPropertiesConfiguration* configuration() const { return 0; }
    virtual void setConfiguration(const KisPropertiesConfiguration* config) { Q_UNUSED(config) }

    Command * currentCommandSettings();

    void reload();

private:
    void createSettingsWidget(ROLE role);

private:
    Command * m_commandDefinition;
    QHash<QWidget *, int> m_widgetToParameterIndexMapper;
    QHash<Parameter *, QWidget *> m_parameterToWidgetMapper;

    Parameter * parameter(QObject * widget);
    QWidget * widget(Parameter * parameter);

    // maps the parameter to widget in both directions, two hash tables are used for it
    void mapParameterWidget(Parameter * parameter, QWidget * widget);


private Q_SLOTS:
    void setIntValue(int value);
    void setFloatValue(qreal value);
    void setBoolValue(bool value);
    void setChoiceIndex(int index);
    void setColorValue(const QColor &color);
    void setTextValue();
    void setFolderPathValue(const QUrl &kurl);
    void setFilePathValue(const QUrl &kurl);

};

Q_DECLARE_METATYPE(KisGmicSettingsWidget*)

#endif

