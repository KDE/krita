/*
 *  kis_tool_curve_select.h -- part of Krita
 *
 *  Copyright (c) 2006 Emanuele Tamponi <emanuele@valinor.it>
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

#ifndef KIS_TOOL_CURVE_SELECT_H_
#define KIS_TOOL_CURVE_SELECT_H_

#include <qpen.h>
#include <qcursor.h>

#include "kis_selection.h"
#include "kis_curve_framework.h"
#include "kis_tool_curve.h"
#include "kis_point.h"

class KisSelectionOptions;

class KisToolCurveSelect : public KisToolCurve {

    typedef KisToolCurve super;
    Q_OBJECT

public:
    KisToolCurveSelect(const QString& UIName);
    virtual ~KisToolCurveSelect();

    virtual QWidget* createOptionWidget(QWidget* parent);
    virtual QWidget* optionWidget();

public slots:
    void slotSetAction(int action);

protected:

    virtual QValueVector<KisPoint> convertCurve();
    //
    // KisToolCurve interface
    //
    virtual void paintCurve();

private:
    QString m_UIName;

    KisSelectionOptions * m_optWidget;
    enumSelectionMode m_selectAction;
};

#endif //__KIS_TOOL_CURVE_SELECT_H_
