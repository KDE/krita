/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_PAINTOP_OPTION_H
#define KIS_PAINTOP_OPTION_H

#include <kis_types.h>
#include <krita_export.h>
#include <kis_properties_configuration.h>

class QWidget;
class QString;

/**
 * Base interface for paintop options. A paintop option
 * can be enabled/disabled, has a configuration page
 * (for example, a curve), a user-visible name and can
 * be serialized and deserialized into KisPaintOpPresets
 *
 * Options are disabled by default.
 */
class PAINTOP_EXPORT KisPaintOpOption : public QObject
{
    Q_OBJECT

public:

    KisPaintOpOption(const QString & label, bool checked = true);
    virtual ~KisPaintOpOption();

    QString & label() const;
    bool isCheckable() {
        return m_checkable;
    }
    bool isChecked() const;

public slots:

    void setChecked(bool checked);

    void setConfigurationPage(QWidget * page);
    QWidget * configurationPage() const;

    /**
     * Re-implement this to save the configuration to the paint configuration.
     */
    virtual void writeOptionSetting(KisPropertiesConfiguration* setting) const {
        Q_UNUSED(setting);
    }

    /**
     * Re-implement this to set te widgets with the values in @param setting.
     */
    virtual void readOptionSetting(const KisPropertiesConfiguration* setting) {
        Q_UNUSED(setting);
    }

    /**
     * re-implement this to handle actuator events.
     */
    //virtual void actuate(KRITA_PAINTOP_OPTION_ACTUATOR actuator);

signals:

    /**
     * emit this whenever a setting has changed. It will update the preview
     */
    void sigSettingChanged();

protected:

    bool m_checkable;

private:

    class Private;
    Private * const m_d;
};

#endif
