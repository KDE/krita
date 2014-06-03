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

#ifndef KDCHARTTEXTLABELCACHE_H
#define KDCHARTTEXTLABELCACHE_H

#include <QPixmap>
#include <QFont>
#include <QBrush>
#include <QPen>

#include "KDChartEnums.h"

/**
  * @brief  base class for prerendered elements like labels, pixmaps, markers, etc.
  */
class PrerenderedElement {
public:
    PrerenderedElement();
    virtual ~PrerenderedElement() {}

    /** Returns the rendered element.
        If any of the properties have change, the element will be
        regenerated. */
    virtual const QPixmap& pixmap() const = 0;

    /** Return the location of the reference point relatively to the
        pixmap's origin. */
    virtual QPointF referencePointLocation( KDChartEnums::PositionValue ) const = 0;

    /** Set the position of the element. */
    void setPosition( const QPointF& position );
    /** Get the position of the element. */
    const QPointF& position() const;

    /** Set the reference point of the element.
        Every element has nine possible reference points (all compass
        directions, plus the center.
     */
    void setReferencePoint( KDChartEnums::PositionValue );
    /** Get the reference point of the element. */
    KDChartEnums::PositionValue referencePoint() const;

protected:
    /** invalidate() needs to be called if any of the properties that
        determine the visual appearance of the prerendered element
        change.
        It can be called for a const object, as objects may need to
        force recalculation of the pixmap.
    */
    virtual void invalidate() const = 0;

private:
    QPointF m_position;
    KDChartEnums::PositionValue m_referencePoint;
};

/** 
    @brief PrerenderedLabel is an internal KDChart class that simplifies creation
    and caching of cached text labels.
    
    It provides referenze points to anchor the text to other
    elements. Reference points use the positions defined in
    KDChartEnums.

    Usage:
    <pre>
    double angle = 90.0;
    CachedLabel label;
    label.paint( font, tr("Label"), angle );
    </pre>
*/

// FIXME this is merely a prototype
// FIXME caching could be done by a second layer that can be used to,
// e.g., query for a prerendered element by id or name, or by changing
// the pixmap() method to do lazy evaluation.
class PrerenderedLabel : public PrerenderedElement
{
public:
    PrerenderedLabel();
    ~PrerenderedLabel();

    void setFont( const QFont& font );
    const QFont& font() const;

    void setText( const QString& text );
    const QString& text() const;

    void setBrush( const QBrush& brush );
    const QBrush& brush() const;

    void setPen( const QPen& );
    const QPen& pen() const;

    void setAngle( double angle );
    double angle() const;

    // reimpl PrerenderedElement:
    const QPixmap& pixmap() const;
    QPointF referencePointLocation( KDChartEnums::PositionValue position ) const;
    // overload: return location of referencePoint():
    QPointF referencePointLocation() const;

protected:
    void invalidate() const;

private:
    /** Create a label with the given text and the given rotation
        angle. Needs to be const, otherwise the pixmap() method cannot
        update when needed. */
    void paint() const;

    // store the settings (these are used for the painting):
    mutable bool m_dirty;
    QFont m_font;
    QString m_text;
    QBrush m_brush;
    QPen m_pen;
    double m_angle;

    // these are valid once the label has been rendered:
    mutable QPixmap m_pixmap;
    mutable QPointF m_referenceBottomLeft;
    mutable QPointF m_textBaseLineVector;
    mutable QPointF m_textAscendVector;
};

#endif
