/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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


#include <kritaui_export.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include "kis_tool_paint.h"
#include "kis_painter.h"
#include "ui_wdggeometryoptions.h"



class KoCanvasBase;
class KoPathShape;


class WdgGeometryOptions : public QWidget, public Ui::WdgGeometryOptions
{
    Q_OBJECT

public:
    WdgGeometryOptions(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

/**
 * Base for tools specialized in drawing shapes
 */
class KRITAUI_EXPORT KisToolShape : public KisToolPaint
{

    Q_OBJECT

public:
    KisToolShape(KoCanvasBase * canvas, const QCursor & cursor);
    ~KisToolShape() override;
    int flags() const override;
    WdgGeometryOptions *m_shapeOptionsWidget;

public Q_SLOTS:
    void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes) override;
    virtual void outlineSettingChanged(int value);
    virtual void fillSettingChanged(int value);

protected:
    QWidget* createOptionWidget() override;

    virtual KisPainter::FillStyle fillStyle();
    KisPainter::StrokeStyle strokeStyle();

    qreal currentStrokeWidth() const;

    struct KRITAUI_EXPORT ShapeAddInfo {
        bool shouldAddShape = false;
        bool shouldAddSelectionShape = false;

        void markAsSelectionShapeIfNeeded(KoShape *shape) const;
    };

    ShapeAddInfo shouldAddShape(KisNodeSP currentNode) const;

    void addShape(KoShape* shape);

    void addPathShape(KoPathShape* pathShape, const KUndo2MagicString& name);

    KConfigGroup m_configGroup;


};

#endif // KIS_TOOL_SHAPE_H_

