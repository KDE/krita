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

#include "KDChartTernaryAxis.h"

#include <QPainter>

#include <KDChartChart>
#include <KDChartPaintContext>

#include "TernaryConstants.h"
#include "KDChartTernaryCoordinatePlane.h"
#include "KDChartAbstractTernaryDiagram.h"


#include "../src/KDChartLayoutItems.h"
#include "PrerenderedElements/KDChartTextLabelCache.h"

using namespace KDChart;

// m_label and m_fifty do not have to be pointers, once the class is
// pimpled (PrerenderedLabel is not published API)

TernaryAxis::TernaryAxis ( AbstractTernaryDiagram* diagram)
    : AbstractAxis( diagram )
    , m_position( KDChartEnums::PositionUnknown )
    , m_label( new PrerenderedLabel )
    , m_fifty( new PrerenderedLabel )
{
    resetTitleTextAttributes();
    setPosition( KDChartEnums::PositionSouth ); // arbitrary
    m_fifty->setText( QObject::tr( "50%" ) ); // const
    // FIXME is this consistent with other diagram/axis/plane implementations?
    diagram->addAxis( this );
}

TernaryAxis::~TernaryAxis()
{
    delete m_label; m_label = 0;
    delete m_label; m_fifty = 0;
}

void  TernaryAxis::paintAll (QPainter &)
{
    // not used
}

void  TernaryAxis::paint (QPainter *)
{
    // not used
}

void  TernaryAxis::paintCtx (PaintContext * paintContext)
{
    QPainter* p = paintContext->painter();
    TernaryCoordinatePlane* plane =
        (TernaryCoordinatePlane*) paintContext->coordinatePlane();
    // QObject* refArea = plane->parent();

    // paint the axis label (across the triangle, that one):
    QList<PrerenderedLabel*> labels;
    labels << m_label << m_fifty;
    Q_FOREACH( PrerenderedLabel* label, labels ) {
        const QPixmap& pixmap = label->pixmap();
        QPointF point = plane->translate( label->position() )
                        - label->referencePointLocation();
        p->drawPixmap( point, pixmap );
    }
}

bool TernaryAxis::isEmpty() const
{
    // todo: what's this method for?
    return false;
}

QRect TernaryAxis::geometry () const
{
    return m_geometry;
}

void TernaryAxis::setGeometry (const QRect &rect)
{
    m_geometry = rect;
}

QSize  TernaryAxis::minimumSize () const
{
    // todo: return realistic sizes
    return QSize( 100, 100 );
}

QSize  TernaryAxis::maximumSize () const
{
    return QSize( 300, 200 );
}

QSize  TernaryAxis::sizeHint () const
{
    return QSize( 150, 100 );
}

Qt::Orientations TernaryAxis::expandingDirections () const
{
    return Qt::Vertical | Qt::Horizontal;
}

const Position TernaryAxis::position () const
{
    return m_position;
}

void  TernaryAxis::setPosition (Position p)
{
    if ( p == position() ) return;

    if ( p != KDChartEnums::PositionWest
         && p != KDChartEnums::PositionEast
         && p != KDChartEnums::PositionSouth )
    {
        qDebug() << "TernaryAxis::setPosition: only south, east and west are supported "
            "positions for ternary axes.";
        return;
    }

    if ( m_title.isEmpty() )
        switch( p.value() ) {
        case KDChartEnums::PositionSouth:
            m_label->setText( tr( "A" ) );
            break;
        case KDChartEnums::PositionWest:
            m_label->setText( tr( "C" ) );
            break;
        case KDChartEnums::PositionEast:
            m_label->setText( tr( "B" ) );
            break;
        default:
            break;
        }

    m_position = p;
    updatePrerenderedLabels(); // position has changed
}

void TernaryAxis::setTitleText( const QString& text )
{
    m_title = text; // do not remove
    m_label->setText( text );
}

QString TernaryAxis::titleText() const
{
    return m_label->text();
}

void TernaryAxis::setTitleTextAttributes( const TextAttributes &a )
{
    m_titleAttributes = a;
    updatePrerenderedLabels();
}

TextAttributes TernaryAxis::titleTextAttributes() const
{
    return m_titleAttributes;
}

void TernaryAxis::resetTitleTextAttributes()
{
    TextAttributes a;
    m_titleAttributes = a;
    updatePrerenderedLabels();
}

bool TernaryAxis::hasDefaultTitleTextAttributes() const
{
    TextAttributes a;
    return m_titleAttributes == a;
}

void TernaryAxis::updatePrerenderedLabels()
{
    TextAttributes attributes = titleTextAttributes();
    double axisLabelAngle = 0.0;
    double fiftyMarkAngle = 0.0;
    QPointF axisLabelPosition;
    QPointF fiftyMarkPosition;
    KDChartEnums::PositionValue fiftyMarkReferencePoint = KDChartEnums::PositionUnknown;

    switch( position().value() ) {
    case KDChartEnums::PositionSouth:
        // this is the axis on the other side of A
        axisLabelAngle = 0.0;
        fiftyMarkAngle = 0.0;
        axisLabelPosition = TriangleTop;
        fiftyMarkPosition = 0.5 * AxisVector_B_C - RelMarkerLength * Norm_B_C;
        fiftyMarkReferencePoint = KDChartEnums::PositionNorth;
        break;
    case KDChartEnums::PositionEast:
        // this is the axis on the other side of B
        axisLabelAngle = 240.0;
        fiftyMarkAngle = 60;
        axisLabelPosition = TriangleBottomLeft;
        fiftyMarkPosition = AxisVector_B_C + 0.5 * AxisVector_C_A - RelMarkerLength * Norm_C_A;
        fiftyMarkReferencePoint = KDChartEnums::PositionSouth;
        break;
    case KDChartEnums::PositionWest:
        // this is the axis on the other side of C
        axisLabelAngle = 120.0;
        fiftyMarkAngle = 300.0;
        axisLabelPosition = TriangleBottomRight;
        fiftyMarkPosition = 0.5 * AxisVector_B_A + RelMarkerLength * Norm_B_A;
        fiftyMarkReferencePoint = KDChartEnums::PositionSouth;
        break;
    case KDChartEnums::PositionUnknown:
        break; // initial value
    default:
        qDebug() << "TernaryAxis::updatePrerenderedLabel: unknown location";
    };

    m_label->setFont( attributes.font() );
    // m_label->setText( titleText() ); // done by setTitleText()
    m_label->setAngle( axisLabelAngle );
    m_label->setPosition( axisLabelPosition );
    m_label->setReferencePoint( KDChartEnums::PositionSouth );
    QFont font = attributes.font();
    font.setPointSizeF( 0.85 * font.pointSizeF() );
    m_fifty->setFont( font );
    m_fifty->setAngle( fiftyMarkAngle );
    m_fifty->setPosition( fiftyMarkPosition );
    m_fifty->setReferencePoint( fiftyMarkReferencePoint );
}

QPair<QSizeF, QSizeF> TernaryAxis::requiredMargins() const
{
    QSizeF topleft( 0.0, 0.0 );
    QSizeF bottomRight( 0.0, 0.0 );

    switch( position().value() ) {
    case KDChartEnums::PositionSouth:
        // the label of the south axis is, in fact, up north.
        topleft.setHeight( m_label->pixmap().height() );
        bottomRight.setHeight( m_fifty->pixmap().height() );
        break;
    case KDChartEnums::PositionWest:
        bottomRight.setWidth( m_label->pixmap().width()
                              - m_label->referencePointLocation().x() );
        bottomRight.setHeight( m_label->pixmap().height()
                               - m_label->referencePointLocation().y() );
        break;
    case KDChartEnums::PositionEast:
        topleft.setWidth( m_label->pixmap().width()
                          - ( m_label->pixmap().width()
                              - m_label->referencePointLocation().x() ) );
        bottomRight.setHeight( m_label->pixmap().height()
                               - ( m_label->pixmap().height()
                                   - m_label->referencePointLocation().y() ) );
        break;
    default:
        qDebug() << "TernaryAxis::requiredMargins: unknown location";
    }
//     qDebug() << "TernaryAxis::requiredMargins:" << topleft << bottomRight;
    return QPair<QSizeF, QSizeF>( topleft, bottomRight );
}
