/****************************************************************************
** Copyright (C) 2001-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Chart library.
**
** Licensees holding valid commercial KD Chart licenses may use this file in
** accordance with the KD Chart Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#ifndef KDCHARTABSTRACTAREAWIDGET_H
#define KDCHARTABSTRACTAREAWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QRect>

#include "KDChartAbstractAreaBase.h"

namespace KDChart {


/**
  * @class AbstractAreaWidget KDChartAbstractArea.h
  * @brief An area in the chart with a background, a frame, etc.
  *
  * AbstractAreaWidget is the base for all widget classes that have
  * a set of background attributes and frame attributes, such as
  * KDChart::Chart and KDChart::Legend.
  */
class KDCHART_EXPORT AbstractAreaWidget : public QWidget, public AbstractAreaBase
{
    Q_OBJECT

    Q_DISABLE_COPY( AbstractAreaWidget )
    KDCHART_DECLARE_PRIVATE_DERIVED_QWIDGET( AbstractAreaWidget )

public:
    explicit AbstractAreaWidget( QWidget* parent = 0 );

    /**
      * @brief Draws the background and frame, then calls paint().
      *
      * In most cases there is no need to overwrite this method in a derived
      * class, but you would overwrite paint() instead.
      * @sa paint
      */
    virtual void paintEvent( QPaintEvent* event );

    /**
      * @brief Draws the background and frame, then calls paint().
      *
      * In most cases there is no need to overwrite this method in a derived
      * class, but you would overwrite paint() instead.
      */
    virtual void paintIntoRect( QPainter& painter, const QRect& rect );

    /**
      * Overwrite this to paint the inner contents of your widget.
      *
      * @note When overriding this method, please let your widget draw
      * itself at the top/left corner of the painter.  You should call rect()
      * (or width(), height(), resp.) to find the drawable area's size:
      * While the paint() method is being executed the frame of the widget
      * is outside of its rect(), so you can use all of rect() for
      * your custom drawing!
      * @sa paint, paintIntoRect
      */
    virtual void paint( QPainter* painter ) = 0;

    /**
      * Call paintAll, if you want the background and the frame to be drawn
      * before the normal paint() is invoked automatically.
      */
    void paintAll( QPainter& painter );

    /**
      * Call this to trigger an unconditional re-building of the widget's internals.
      */ 
    virtual void forceRebuild();

    /**
      * Call this to trigger an conditional re-building of the widget's internals.
      *
      * e.g. AbstractAreaWidget call this, before calling layout()->setGeometry()
      */ 
    virtual void needSizeHint();
    //virtual void setGeometry( const QRect & rect );
    virtual void resizeLayout( const QSize& );

protected:
    virtual ~AbstractAreaWidget() ;
    virtual QRect areaGeometry() const;
    virtual void positionHasChanged();


public:
//    virtual AbstractAreaWidget * clone() const = 0;

Q_SIGNALS:
    void positionChanged( AbstractAreaWidget * );

}; // End of class AbstractAreaWidget

}
#endif // KDCHARTABSTRACTAREAWIDGET_H
