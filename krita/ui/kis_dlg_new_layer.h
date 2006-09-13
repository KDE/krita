/*
 *  Copyright (c) 2000 Michael Koch <koch@kde.org>
 *  Copyright (c) 2000 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2006 Casper Boemann <cbr@boemann.dk>
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
#ifndef KIS_DLG_NEW_LAYER_H_
#define KIS_DLG_NEW_LAYER_H_

#include <kdialog.h>

#include "KoCompositeOp.h"
#include <kis_global.h>

class QWidget;
class KisPaintDevice;
class WdgLayerProperties;

class NewLayerDialog : public KDialog {
    typedef KDialog super;
    Q_OBJECT

public:
    NewLayerDialog(const KoID colorSpace,
               const QString & profilename,
               const QString & deviceName,
               QWidget *parent = 0,
               const char *name = 0);

    QString layerName() const;
    KoCompositeOp * compositeOp() const;
    qint32 opacity() const;
    KoID colorSpaceID() const;
    QString profileName() const;

    void setColorSpaceEnabled(bool enabled);

private slots:
    void fillCmbProfiles(const KoID & s);
    void fillCmbComposite(const KoID & s);

private:
    WdgLayerProperties * m_page;
};

#endif // KIS_DLG_NEW_LAYER_H_

