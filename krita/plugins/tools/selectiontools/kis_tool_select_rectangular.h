/*
 *  kis_tool_select_rectangular.h - part of Krita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
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

#ifndef KIS_TOOL_SELECT_RECTANGULAR_H_
#define KIS_TOOL_SELECT_RECTANGULAR_H_

#include "KoPoint.h"
#include "kis_tool_non_paint.h"
#include "kis_selection.h"
#include "KoToolFactory.h"

class KisSelectionOptions;

class KisToolSelectRectangular : public KisToolNonPaint {

    typedef KisToolNonPaint super;
    Q_OBJECT

public:
    KisToolSelectRectangular();
    virtual ~KisToolSelectRectangular();

    virtual void setup(KActionCollection *collection);
    virtual quint32 priority() { return 3; }
    virtual enumToolType toolType() { return TOOL_SELECT; }
    virtual QWidget * createOptionWidget(QWidget* parent);
        virtual QWidget* optionWidget();

    virtual void paint(QPainter& gc);
    virtual void paint(QPainter& gc, const QRect& rc);
    virtual void buttonPress(KoPointerEvent *e);
    virtual void move(KoPointerEvent *e);
    virtual void buttonRelease(KoPointerEvent *e);

public slots:
    virtual void slotSetAction(int);
    virtual void activate();


private:
    void clearSelection();
    void paintOutline();
    void paintOutline(QPainter& gc, const QRect& rc);

private:
    KisCanvasSubject *m_subject;
    KoPoint m_centerPos;
    KoPoint m_startPos;
    KoPoint m_endPos;
    bool m_selecting;
    KisSelectionOptions * m_optWidget;
    enumSelectionMode m_selectAction;

};

class KisToolSelectRectangularFactory : public KoToolFactory {

public:
    KisToolSelectRectangularFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KisToolSelectRectangular", i18n( "Rectangular Selection" ))
        {
            setToolTip( i18n( "Select a rectangular area" ) );
            setToolType( TOOL_TYPE_SELECTED );
            setIcon( "tool_rect_selection" );
            setShortcut( Qt::Key_R );
            setPriority( 0 );
        }

    virtual ~KisToolSelectRectangularFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return  new KisToolSelectRectangular(canvas);
    }
};



#endif // KIS_TOOL_SELECT_RECTANGULAR_H_

