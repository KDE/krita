/*
 *  kis_tool_duplicate.h - part of Krita
 *
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef __KIS_TOOL_DUPLICATE_H__
#define __KIS_TOOL_DUPLICATE_H__

#include "kis_tool_freehand.h"
#include "kis_tool_factory.h"

class KisEvent;
class KisButtonPressEvent;



class KisToolDuplicate : public KisToolFreehand {

    typedef KisToolFreehand super;
    Q_OBJECT

public:
    KisToolDuplicate();
    virtual ~KisToolDuplicate();
  
    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_FREEHAND; }
    virtual void buttonPress(KisButtonPressEvent *e);
    virtual void move(KisMoveEvent *e);
    
    virtual void paintAt(const KisPoint &pos,
                 const double pressure,
                 const double xTilt,
                 const double yTilt);

    virtual QString quickHelp() const;

protected slots:
    virtual void activate();

protected:
    virtual void initPaint(KisEvent *e);

    // Tool starting duplicate
    KisPoint m_offset; // This member give the offset from the click position to the point where we take the duplication
    bool m_isOffsetNotUptodate; // Tells if the offset is update
    KisPoint m_position; // Give the position of the last alt-click
};


class KisToolDuplicateFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolDuplicateFactory() : super() {};
    virtual ~KisToolDuplicateFactory(){};
    
    virtual KisTool * createTool(KActionCollection * ac) { 
        KisTool * t =  new KisToolDuplicate();
        Q_CHECK_PTR(t);
        t->setup(ac); 
        return t; 
    }
    virtual KisID id() { return KisID("duplicate", i18n("Duplicate Tool")); }
};



#endif //__KIS_TOOL_DUPLICATE_H__

