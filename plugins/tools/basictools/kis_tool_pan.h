/*
 *  SPDX-FileCopyrightText: 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_PAN_H_
#define KIS_TOOL_PAN_H_

#include <kis_tool.h>
#include <KoToolFactoryBase.h>

class KisToolPan : public KisTool
{
    Q_OBJECT
public:
    KisToolPan(KoCanvasBase *canvas);
    ~KisToolPan() override;

    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void paint(QPainter &painter, const KoViewConverter &converter) override;

    bool wantsAutoScroll() const override;

private:
    QPoint m_lastPosition;
};


class KisToolPanFactory : public KoToolFactoryBase
{
public:
    KisToolPanFactory();
    ~KisToolPanFactory() override;

    KoToolBase *createTool(KoCanvasBase *canvas) override;
};

#endif // KIS_TOOL_PAN_H_
