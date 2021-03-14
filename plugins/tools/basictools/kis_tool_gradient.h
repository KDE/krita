/*
 *  kis_tool_line.h - part of Krayon
 *
 *  SPDX-FileCopyrightText: 2000 John Califf <jcaliff@comuzone.net>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_GRADIENT_H_
#define KIS_TOOL_GRADIENT_H_

#include <QKeySequence>

#include <KisToolPaintFactoryBase.h>

#include <kis_tool_paint.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_gradient_painter.h>
#include <flake/kis_node_shape.h>
#include <kis_icon.h>
#include <kconfig.h>
#include <kconfiggroup.h>


class QLabel;
class QPoint;
class QWidget;
class QCheckBox;
class KComboBox;
class KisDoubleSliderSpinBox;

class KisToolGradient : public KisToolPaint
{

    Q_OBJECT

public:
    KisToolGradient(KoCanvasBase * canvas);
    ~KisToolGradient() override;

    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;

    void paint(QPainter &painter, const KoViewConverter &converter) override;

    QWidget* createOptionWidget() override;

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
private Q_SLOTS:
    void slotSetShape(int);
    void slotSetRepeat(int);
    void slotSetReverse(bool);
    void slotSetDither(bool);
    void slotSetAntiAliasThreshold(qreal);
    void setOpacity(qreal opacity);
protected Q_SLOTS:
    void resetCursorStyle() override;

private Q_SLOTS:

    void areaDone(const QRect & rc) {
        currentNode()->setDirty(rc); // Starts computing the projection for the area we've done.

    }

private:

    void paintLine(QPainter& gc);

    QPointF straightLine(QPointF point);

    QPointF m_startPos;
    QPointF m_endPos;

    KisGradientPainter::enumGradientShape m_shape;
    KisGradientPainter::enumGradientRepeat m_repeat;

    bool m_dither;
    bool m_reverse;
    double m_antiAliasThreshold;

    QLabel *m_lbShape;
    QLabel *m_lbRepeat;
    QCheckBox *m_ckDither;
    QCheckBox *m_ckReverse;
    KComboBox *m_cmbShape;
    KComboBox *m_cmbRepeat;
    QLabel *m_lbAntiAliasThreshold;
    KisDoubleSliderSpinBox *m_slAntiAliasThreshold;
    KConfigGroup m_configGroup;
};

class KisToolGradientFactory : public KisToolPaintFactoryBase
{

public:
    KisToolGradientFactory()
            : KisToolPaintFactoryBase("KritaFill/KisToolGradient") {
        setToolTip(i18n("Gradient Tool"));
        setSection(TOOL_TYPE_FILL);
        setIconName(koIconNameCStr("krita_tool_gradient"));
        setShortcut(QKeySequence(Qt::Key_G));
        setPriority(1);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    ~KisToolGradientFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return  new KisToolGradient(canvas);
    }

};

#endif //KIS_TOOL_GRADIENT_H_

