/*
 *  kis_tool_select_brush.h - part of Krita
 *
 *  Copyright (c) 2003-2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_SELECT_BRUSH_H_
#define KIS_TOOL_SELECT_BRUSH_H_

#include "KoToolFactory.h"

#include "kis_layer_shape.h"
#include "kis_tool_freehand.h"

class KisSelectedTransaction;
class KisSelectionOptions;

/**
 * The selection brush creates a selection by painting with the current
 * brush shape. Not sure what kind of an icon could represent this...
 * Depends a bit on how we're going to visualize selections.
 */
class KisToolSelectBrush : public KisToolFreehand {
    Q_OBJECT
    typedef KisToolFreehand super;

public:
    KisToolSelectBrush(KoCanvasBase *canvas);
    virtual ~KisToolSelectBrush();

    virtual QWidget* createOptionWidget();
    virtual QWidget* optionWidget();

public slots:
    virtual void activate();

protected:

    virtual void initPaint(KoPointerEvent *e);
    virtual void endPaint();

private:
    KisSelectionOptions * m_optWidget;
    KisSelectedTransaction *m_transaction;
};

class KisToolSelectBrushFactory : public KoToolFactory {

public:
    KisToolSelectBrushFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KisToolSelectBrush", i18n( "Selection Brush" ))
        {
            setToolTip( i18n( "Paint a selection with a brush" ) );
            setToolType( TOOL_TYPE_SELECTED );
            setIcon( "tool_brush_selection" );
            setShortcut(KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_B));
            setPriority( 0 );
            setActivationShapeId( KIS_LAYER_SHAPE_ID );
        }

    virtual ~KisToolSelectBrushFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return  new KisToolSelectBrush(canvas);
    }

};


#endif // KIS_TOOL_SELECT_BRUSH_H_

