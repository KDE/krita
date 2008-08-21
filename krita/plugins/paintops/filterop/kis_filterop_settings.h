/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_FILTEROP_SETTINGS_H
#define KIS_FILTEROP_SETTINGS_H

#include "kis_types.h"
#include "kis_paintop_settings.h"

class QWidget;
class KisFilterConfiguration;
class QDomElement;
class QDomDocument;
class QGridLayout;
class Ui_FilterOpOptions;
class KoID;
class KisFilterConfigWidget;

class KisFilterOpSettings : public QObject, public KisPaintOpSettings {

Q_OBJECT

public:

    KisFilterOpSettings(QWidget* parent, KisImageSP image);

    virtual ~KisFilterOpSettings();

    virtual KisPaintOpSettingsSP clone() const;

    virtual QWidget *widget() const { return m_optionsWidget; }

    const KisFilterSP filter() const;

    KisFilterConfiguration* filterConfig() const;

    bool ignoreAlpha() const;

    virtual void setNode( KisNodeSP node );

    using KisPaintOpSettings::fromXML;
    virtual void fromXML(const QDomElement&);

    using KisPaintOpSettings::toXML;
    virtual void toXML(QDomDocument&, QDomElement&) const;

protected slots:

    void setCurrentFilter(const KoID &);

private:

    void updateFilterConfigWidget();

private:

    QWidget* m_optionsWidget;
    Ui_FilterOpOptions* m_uiOptions;
    QGridLayout* m_layout;
    const KisFilterSP m_currentFilter;
    KisPaintDeviceSP m_paintDevice;
    KisFilterConfigWidget* m_currentFilterConfigWidget;
    KisImageSP m_image;

};



#endif
