/*
 *  Copyright (c) 2000 Michael Koch <koch@kde.org>
 *  Copyright (c) 2000 Patrick Julien <freak@codepimps.org>
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
#ifndef  KIS_DLG_NEW_LAYER_H_
#define KIS_DLG_NEW_LAYER_H_

#include "kis_composite_op.h"

class QSpinBox;
class QWidget;
class KIntSpinBox;
class KLineEdit;
class KisPaintDeviceImpl;
class KIntNumInput;
class KisCmbComposite;
class KisCmbImageType;
class KisCmbIDList;

#include <kdialogbase.h>

#include <kis_global.h>

class NewLayerDialog : public KDialogBase {
    typedef KDialogBase super;
    Q_OBJECT

public:
    NewLayerDialog(const KisID colorSpace,
               const QString & deviceName,
               QWidget *parent = 0,
               const char *name = 0);

    QString layerName() const;
    KisCompositeOp compositeOp() const;
    Q_INT32 opacity() const;
    QPoint position() const;
    KisID colorStrategyID() const;

private slots:
    void slotSetColorStrategy(const KisID &colorStrategyId);

protected slots:
    void slotNameChanged( const QString & );

private:
    KLineEdit *m_name;
    KIntNumInput *m_opacity;
    KisCmbComposite *m_cmbComposite;
    KisCmbIDList *m_cmbImageType;
};

#endif // KIS_DLG_NEW_LAYER_H_

