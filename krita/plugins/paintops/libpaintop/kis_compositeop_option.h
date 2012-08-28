/*
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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
#ifndef KIS_COMPOSITEOP_OPTION_H
#define KIS_COMPOSITEOP_OPTION_H

#include <kis_paintop_option.h>
#include <krita_export.h>
#include <QString>

// const QString AIRBRUSH_ENABLED = "AirbrushOption/isAirbrushing";
// const QString AIRBRUSH_RATE = "AirbrushOption/rate";

class QLabel;
class QModelIndex;
class QPushButton;
class KisCompositeOpListWidget;
class KoID;

class PAINTOP_EXPORT KisCompositeOpOption: public KisPaintOpOption
{
    Q_OBJECT
    
public:
     KisCompositeOpOption(bool createConfigWidget=false);
    ~KisCompositeOpOption();
    
    virtual void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    virtual void readOptionSetting(const KisPropertiesConfiguration* setting);

private slots:
    void slotCompositeOpChanged(const QModelIndex& index);
    void slotEraserToggled(bool toggled);
    
private:
    void changeCompositeOp(const KoID& compositeOp);
    
private:
    QLabel*                   m_label;
    QPushButton*              m_bnEraser;
    KisCompositeOpListWidget* m_list;
    QString                   m_prevCompositeOpID;
    QString                   m_currCompositeOpID;
    bool                      m_createConfigWidget;

};

#endif
