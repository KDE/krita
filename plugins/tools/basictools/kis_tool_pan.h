/*
 *  SPDX-FileCopyrightText: 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_PAN_H_
#define KIS_TOOL_PAN_H_

#include <kis_tool.h>
#include <KoToolFactoryBase.h>

#include <kis_signal_auto_connection.h>


class KisToolPan : public KisTool
{
    Q_OBJECT
public:
    KisToolPan(KoCanvasBase *canvas);
    ~KisToolPan() override;

    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;

    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;

    void paint(QPainter &painter, const KoViewConverter &converter) override;

    bool wantsAutoScroll() const override;

private:
    QPoint m_lastPosition;
    KisSignalAutoConnectionsStore m_actionConnections;
};


class KisToolPanFactory : public KoToolFactoryBase
{
public:
    KisToolPanFactory();
    ~KisToolPanFactory() override;

    QList<QAction *> createActionsImpl() override;
    KoToolBase *createTool(KoCanvasBase *canvas) override;
};

#endif // KIS_TOOL_PAN_H_
