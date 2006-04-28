/*
 *  Copyright (c) 2005 Adrian Page
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

#ifndef KIS_TOOL_SHAPE_H_
#define KIS_TOOL_SHAPE_H_

#include <koffice_export.h>

#include "kis_tool_paint.h"
#include "kis_painter.h"

class QGridLayout;
class WdgGeometryOptions;

class KRITACORE_EXPORT KisToolShape : public KisToolPaint {

    Q_OBJECT
    typedef KisToolPaint super;

public:
    KisToolShape(const QString& UIName);
    virtual ~KisToolShape();

    virtual enumToolType toolType() { return TOOL_SHAPE; }

protected:
    virtual QWidget* createOptionWidget(QWidget* parent);

    KisPainter::FillStyle fillStyle();

private:
    QGridLayout *m_optionLayout;
    WdgGeometryOptions *m_shapeOptionsWidget;
};

#endif // KIS_TOOL_SHAPE_H_

