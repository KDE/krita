/*
 *  SPDX-FileCopyrightText: 2005 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_SHAPE_H_
#define KIS_TOOL_SHAPE_H_


#include <kritaui_export.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <kis_painter.h>

#include "kis_tool_paint.h"
#include "KisSelectionToolFactoryBase.h"
#include "KisToolShapeUtils.h"

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
    void activate(const QSet<KoShape*> &shapes) override;
    virtual void outlineSettingChanged(int value);
    virtual void fillSettingChanged(int value);
    virtual void patternRotationSettingChanged(qreal value);
    virtual void patternScaleSettingChanged(qreal value);

protected:
    QWidget* createOptionWidget() override;

    KisToolShapeUtils::FillStyle fillStyle();
    KisToolShapeUtils::StrokeStyle strokeStyle();
    QTransform fillTransform();

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

