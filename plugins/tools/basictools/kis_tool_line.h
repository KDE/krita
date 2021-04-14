/*
 *  kis_tool_line.h - part of Krayon
 *
 *  SPDX-FileCopyrightText: 2000 John Califf <jcaliff@comuzone.net>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_LINE_H_
#define KIS_TOOL_LINE_H_

#include "kis_tool_shape.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <QScopedPointer>
#include <kis_global.h>
#include <kis_types.h>
#include <KisToolPaintFactoryBase.h>
#include <flake/kis_node_shape.h>
#include <kis_signal_compressor.h>
#include <kis_icon.h>
#include <KoIcon.h>

class QPoint;
class KoCanvasBase;
class QCheckBox;
class KisPaintingInformationBuilder;
class KisToolLineHelper;


class KisToolLine : public KisToolShape
{
    Q_OBJECT

public:
    KisToolLine(KoCanvasBase * canvas);
    ~KisToolLine() override;

    void requestStrokeCancellation() override;
    void requestStrokeEnd() override;

    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;

    void paint(QPainter& gc, const KoViewConverter &converter) override;

    QString quickHelp() const override;

protected Q_SLOTS:
    void resetCursorStyle() override;

private Q_SLOTS:
    void updateStroke();
    void setUseSensors(bool value);
    void setShowPreview(bool value);
    void setShowGuideline(bool value);

private:
    void paintLine(QPainter& gc, const QRect& rc);
    QPointF straightLine(QPointF point);
    void updateGuideline();
    void showSize();
    void updatePreviewTimer(bool showGuide);
    QWidget* createOptionWidget() override;

    void endStroke();
    void cancelStroke();

private:
    bool m_showGuideline;

    QPointF m_startPoint;
    QPointF m_endPoint;
    QPointF m_lastUpdatedPoint;

    bool m_strokeIsRunning;


    QCheckBox *m_chkUseSensors;
    QCheckBox *m_chkShowPreview;
    QCheckBox *m_chkShowGuideline;

    QScopedPointer<KisPaintingInformationBuilder> m_infoBuilder;
    QScopedPointer<KisToolLineHelper> m_helper;
    KisSignalCompressor m_strokeUpdateCompressor;
    KisSignalCompressor m_longStrokeUpdateCompressor;

    KConfigGroup configGroup;
};


class KisToolLineFactory : public KisToolPaintFactoryBase
{

public:

    KisToolLineFactory()
            : KisToolPaintFactoryBase("KritaShape/KisToolLine") {
        setToolTip(i18n("Line Tool"));
        // Temporarily
        setSection(ToolBoxSection::Shape);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setPriority(1);
        setIconName(koIconNameCStr("krita_tool_line"));
    }

    ~KisToolLineFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolLine(canvas);
    }

};




#endif //KIS_TOOL_LINE_H_
