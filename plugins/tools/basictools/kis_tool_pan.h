/*
 *  Copyright (c) 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
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
