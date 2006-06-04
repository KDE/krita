/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                2003 Patrick Julien  <freak@codepimps.org>
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

#ifndef KIS_TOOL_MOVE_H_
#define KIS_TOOL_MOVE_H_

#include "kis_strategy_move.h"
#include "kis_tool_non_paint.h"
#include "kis_tool_factory.h"

// XXX: Moving is not nearly smooth enough!
class KisToolMove : public KisToolNonPaint {

    typedef KisToolNonPaint super;
    Q_OBJECT

public:
    KisToolMove();
    virtual ~KisToolMove();

public:
    virtual void update(KisCanvasSubject *subject);

public:
    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_TRANSFORM; }
    virtual quint32 priority() { return 2; }

    virtual void buttonPress(KisButtonPressEvent *e);
    virtual void move(KisMoveEvent *e);
    virtual void buttonRelease(KisButtonReleaseEvent *e);

private:
    KisCanvasSubject *m_subject;
    KisStrategyMove m_strategy;
};


class KisToolMoveFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolMoveFactory() : super() {};
    virtual ~KisToolMoveFactory(){};
    
    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolMove(); 
        Q_CHECK_PTR(t);
        t->setup(ac); 
        return t; 
    }
    virtual KoID id() { return KoID("move", i18n("Move Tool")); }
};



#endif // KIS_TOOL_MOVE_H_

