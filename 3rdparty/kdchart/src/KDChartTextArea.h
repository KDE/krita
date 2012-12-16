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

#ifndef KDCHART_TEXT_AREA_H
#define KDCHART_TEXT_AREA_H

#include <QObject>

#include "KDChartGlobal.h"
#include "KDChartAbstractAreaBase.h"
#include "KDChartLayoutItems.h"

namespace KDChart {


/**
  * @class TextArea KDChartTextArea.h
  * @brief A text area in the chart with a background, a frame, etc.
  *
  * TextArea is the base class for all text containing non-widget chart elements
  * that have a set of background attributes and frame attributes, such as
  * headers or footers.
  *
  * @note This class inherits from AbstractAreaBase, TextLayoutItem, QObject.
  * The reason for this tripple inheritance is that neither AbstractAreaBase nor
  * TextLayoutItem are QObject.
  */
class KDCHART_EXPORT TextArea : public QObject, public AbstractAreaBase, public TextLayoutItem
{
    Q_OBJECT

    Q_DISABLE_COPY( TextArea )
    KDCHART_DECLARE_PRIVATE_DERIVED( TextArea )


public:
    virtual ~TextArea() ;

//    virtual TextArea * clone() const = 0;
    /**
      * @brief Draws the background and frame, then calls paint().
      *
      * In most cases there is no need to overwrite this method in a derived
      * class, but you would overwrite TextLayoutItem::paint() instead.
      */
    virtual void paintIntoRect( QPainter& painter, const QRect& rect );

    /**
      * Call paintAll, if you want the background and the frame to be drawn
      * before the normal paint() is invoked automatically.
      */
    void paintAll( QPainter& painter );

protected:
    TextArea();
    virtual QRect areaGeometry() const;
    virtual void positionHasChanged();

Q_SIGNALS:
    void positionChanged( TextArea * );

    //KDCHART_DECLARE_PRIVATE_DERIVED(TextArea)
}; // End of class TextArea

}
#endif // KDCHART_TEXT_AREA_H
