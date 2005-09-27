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

#include <kdialogbase.h>

class QWidget;
class WdgLayerProperties;
class KisCompositeOp;
class KisColorSpace;

class KisDlgLayerProperties : public KDialogBase {
    typedef KDialogBase super;
    Q_OBJECT

public:
    KisDlgLayerProperties(const QString& deviceName,
                        Q_INT32 opacity,
                        const KisCompositeOp& compositeOp,
                        const KisColorSpace * colorSpace,
                        QWidget *parent = 0, const char *name = 0, WFlags f = 0);

    virtual ~KisDlgLayerProperties();

    QString getName() const;
    Q_INT32 getOpacity() const;
    KisCompositeOp getCompositeOp() const;

protected slots:
    void slotNameChanged( const QString & );

private:
    WdgLayerProperties * m_page;
};

#endif // KIS_DLG_LAYER_PROPERTIES_H_

