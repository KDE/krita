/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_DLG_LAYER_PROPERTIES_H_
#define KIS_DLG_LAYER_PROPERTIES_H_

#include <kdialog.h>

#include "ui_wdglayerproperties.h"

class QWidget;
class KoCompositeOp;
class KoColorSpace;

class WdgLayerProperties : public QWidget, public Ui::WdgLayerProperties
{
    Q_OBJECT

    public:
        WdgLayerProperties(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

class KisDlgLayerProperties : public KDialog {
    typedef KDialog super;
    Q_OBJECT

public:
    KisDlgLayerProperties(const QString& deviceName,
                        qint32 opacity,
                        const KoCompositeOp* compositeOp,
                        const KoColorSpace * colorSpace,
                        QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);

    virtual ~KisDlgLayerProperties();

    QString getName() const;
    qint32 getOpacity() const;
    KoCompositeOp * getCompositeOp() const;

protected slots:
    void slotNameChanged( const QString & );

private:
    WdgLayerProperties * m_page;
};

#endif // KIS_DLG_LAYER_PROPERTIES_H_

