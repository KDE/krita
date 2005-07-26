/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include "kis_global.h"
#include "kis_types.h"
#include "kis_composite_op.h"

class QWidget;
class KIntSpinBox;
class KLineEdit;
class KIntNumInput;
class KisCmbComposite;

class KisPaintPropertyDlg : public KDialogBase {
	typedef KDialogBase super;
    Q_OBJECT

public:
	KisPaintPropertyDlg(const QString& deviceName,
			    const QPoint& pos,
			    Q_INT32 opacity,
			    const KisCompositeOp& compositeOp,
			    const KisStrategyColorSpaceSP colorSpace,
			    QWidget *parent = 0, const char *name = 0, WFlags f = 0);
	virtual ~KisPaintPropertyDlg();

	QString getName() const;
	Q_INT32 getOpacity() const;
	KisCompositeOp getCompositeOp() const;
	QPoint getPosition() const;
protected slots:
    void slotNameChanged( const QString & );


private:
	KLineEdit *m_name;
	KIntNumInput *m_opacity;
	KIntSpinBox *m_x;
	KIntSpinBox *m_y;
	KisCmbComposite *m_cmbComposite;
};

#endif // KIS_DLG_LAYER_PROPERTIES_H_

