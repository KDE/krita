/*
 *  kis_tool_line.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@comuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
    void activate(ToolActivation activation, const QSet<KoShape*> &shapes) override;
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
        setSection(TOOL_TYPE_SHAPE);
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
