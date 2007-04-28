/*
 *  kis_tool_select_eraser.h - part of Krita
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

#ifndef KIS_TOOL_SELECT_ERASER_H_
#define KIS_TOOL_SELECT_ERASER_H_

#include "KoToolFactory.h"

#include "kis_layer_shape.h"
#include "kis_tool_freehand.h"

class KisSelectionOptions;

/**
 * The selection eraser makes a selection smaller by painting with the
 * current eraser shape. Not sure what kind of an icon could represent
 * this... Depends a bit on how we're going to visualize selections.
 */
class KisToolSelectEraser : public KisToolFreehand {
    Q_OBJECT
    typedef KisToolFreehand super;

public:
    KisToolSelectEraser(KoCanvasBase *canvas);
    virtual ~KisToolSelectEraser();

    virtual QWidget* createOptionWidget();
    virtual QWidget* optionWidget();

public slots:
    virtual void activate();

protected:
    virtual void initPaint(KoPointerEvent *e);
    virtual void endPaint();
private:
    KisSelectionOptions * m_optWidget;
};

class KisToolSelectEraserFactory : public KoToolFactory {
    typedef KoToolFactory super;
public:
    KisToolSelectEraserFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent,  "KisToolSelectEraser", i18n( "Selection Eraser" ))
        {
            setToolTip( i18n( "Erase parts of a selection with a brush" ) );
            setToolType( TOOL_TYPE_SELECTED );
            setIcon( "tool_eraser_selection" );
            setShortcut(KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_E));
            setPriority( 0 );
            setActivationShapeId( KIS_LAYER_SHAPE_ID );
        }

    virtual ~KisToolSelectEraserFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return  new KisToolSelectEraser(canvas);
    }

};



#endif // KIS_TOOL_SELECT_ERASER_H_

