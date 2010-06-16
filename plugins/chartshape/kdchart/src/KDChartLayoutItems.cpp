/****************************************************************************
 ** Copyright (C) 2007 Klar�vdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/

#include "KDChartLayoutItems.h"
#include "KDTextDocument.h"
#include "KDChartAbstractArea.h"
#include "KDChartAbstractDiagram.h"
#include "KDChartBackgroundAttributes.h"
#include "KDChartFrameAttributes.h"
#include "KDChartPaintContext.h"
#include "KDChartPainterSaver_p.h"
#include "KDChartPrintingParameters.h"
#include <QTextCursor>
#include <QTextBlockFormat>
#include <QTextDocumentFragment>
#include <QAbstractTextDocumentLayout>
#include <QLayout>
#include <QPainter>
#include <QDebug>
#include <QCoreApplication>
#include <QApplication>
#include <QStringList>
#include <QStyle>

#include <KDABLibFakes>

#include <math.h>

#define PI 3.141592653589793



//#define DEBUG_ITEMS_PAINT

/**
    Inform the item about its widget: This enables the item,
    to trigger that widget's update, whenever the size of the item's
    contents has changed.

    Thus, you need to call setParentWidget on every item, that
    has a non-fixed size.
  */
void KDChart::AbstractLayoutItem::setParentWidget( QWidget* widget )
{
    mParent = widget;
}

void KDChart::AbstractLayoutItem::paintAll( QPainter& painter )
{
    paint( &painter );
}

/**
  * Default impl: Paint the complete item using its layouted position and size.
  */
void KDChart::AbstractLayoutItem::paintCtx( PaintContext* context )
{
    if( context )
        paint( context->painter() );
}

/**
    Report changed size hint: ask the parent widget to recalculate the layout.
  */
void KDChart::AbstractLayoutItem::sizeHintChanged()const
{
    // This is exactly like what QWidget::updateGeometry does.
//  qDebug("KDChart::AbstractLayoutItem::sizeHintChanged() called");
    if( mParent ) {
        if ( mParent->layout() )
            mParent->layout()->invalidate();
        else
            QApplication::postEvent( mParent, new QEvent( QEvent::LayoutRequest ) );
    }
}

KDChart::TextBubbleLayoutItem::TextBubbleLayoutItem( const QString& text,
                                         const KDChart::TextAttributes& attributes,
                                         const QObject* area,
                                         KDChartEnums::MeasureOrientation orientation,
                                         Qt::Alignment alignment )
    : AbstractLayoutItem( alignment ),
      m_text( new TextLayoutItem( text, attributes, area, orientation, alignment ) )
{
}

KDChart::TextBubbleLayoutItem::TextBubbleLayoutItem()
    : AbstractLayoutItem( Qt::AlignLeft ),
      m_text( new TextLayoutItem() )
{
}

KDChart::TextBubbleLayoutItem::~TextBubbleLayoutItem()
{
    delete m_text;
}

void KDChart::TextBubbleLayoutItem::setAutoReferenceArea( const QObject* area )
{
    m_text->setAutoReferenceArea( area );
}

const QObject* KDChart::TextBubbleLayoutItem::autoReferenceArea() const
{
    return m_text->autoReferenceArea();
}

void KDChart::TextBubbleLayoutItem::setText( const QString& text )
{
    m_text->setText( text );
}

QString KDChart::TextBubbleLayoutItem::text() const
{
    return m_text->text();
}

void KDChart::TextBubbleLayoutItem::setTextAttributes( const TextAttributes& a )
{
    m_text->setTextAttributes( a );
}

KDChart::TextAttributes KDChart::TextBubbleLayoutItem::textAttributes() const
{
    return m_text->textAttributes();
}

bool KDChart::TextBubbleLayoutItem::isEmpty() const
{
    return m_text->isEmpty();
}

Qt::Orientations KDChart::TextBubbleLayoutItem::expandingDirections() const
{
    return m_text->expandingDirections();
}

QSize KDChart::TextBubbleLayoutItem::maximumSize() const
{
    const int border = borderWidth();
    return m_text->maximumSize() + QSize( 2 * border, 2 * border );
}

QSize KDChart::TextBubbleLayoutItem::minimumSize() const
{
    const int border = borderWidth();
    return m_text->minimumSize() + QSize( 2 * border, 2 * border );
}

QSize KDChart::TextBubbleLayoutItem::sizeHint() const
{
    const int border = borderWidth();
    return m_text->sizeHint() + QSize( 2 * border, 2 * border );
}

void KDChart::TextBubbleLayoutItem::setGeometry( const QRect& r )
{
    const int border = borderWidth();
    m_text->setGeometry( r.adjusted( border, border, -border, -border ) );
}

QRect KDChart::TextBubbleLayoutItem::geometry() const
{
    const int border = borderWidth();
    return m_text->geometry().adjusted( -border, -border, border, border );
}

void KDChart::TextBubbleLayoutItem::paint( QPainter* painter )
{
    const QPen oldPen = painter->pen();
    const QBrush oldBrush = painter->brush();
    painter->setPen( Qt::black );
    painter->setBrush( QColor( 255, 255, 220 ) );
    painter->drawRoundRect( geometry(), 10 );
    painter->setPen( oldPen );
    painter->setBrush( oldBrush );
    m_text->paint( painter );
}

int KDChart::TextBubbleLayoutItem::borderWidth() const
{
    return 1;
}

KDChart::TextLayoutItem::TextLayoutItem( const QString& text,
                                         const KDChart::TextAttributes& attributes,
                                         const QObject* area,
                                         KDChartEnums::MeasureOrientation orientation,
                                         Qt::Alignment alignment )
    : AbstractLayoutItem( alignment )
    , mText( text )
    , mTextAlignment( alignment )
    , mAttributes( attributes )
    , mAutoReferenceArea( area )
    , mAutoReferenceOrientation( orientation )
    , cachedSizeHint() // default this to invalid to force just-in-time calculation before first use of sizeHint()
    , cachedFontSize( 0.0 )
    , cachedFont( mAttributes.font() )
{
}

KDChart::TextLayoutItem::TextLayoutItem()
    : AbstractLayoutItem( Qt::AlignLeft )
    , mText()
    , mTextAlignment( Qt::AlignLeft )
    , mAttributes()
    , mAutoReferenceArea( 0 )
    , mAutoReferenceOrientation( KDChartEnums::MeasureOrientationHorizontal )
    , cachedSizeHint() // default this to invalid to force just-in-time calculation before first use of sizeHint()
    , cachedFontSize( 0.0 )
    , cachedFont( mAttributes.font() )
{

}

void KDChart::TextLayoutItem::setAutoReferenceArea( const QObject* area )
{
    mAutoReferenceArea = area;
    cachedSizeHint = QSize();
    sizeHint();
}

const QObject* KDChart::TextLayoutItem::autoReferenceArea() const
{
    return mAutoReferenceArea;
}

void KDChart::TextLayoutItem::setText(const QString & text)
{
    mText = text;
    cachedSizeHint = QSize();
    sizeHint();
    if( mParent )
        mParent->update();
}

QString KDChart::TextLayoutItem::text() const
{
    return mText;
}

void KDChart::TextLayoutItem::setTextAlignment( Qt::Alignment alignment)
{
    if( mTextAlignment == alignment )
        return;
    mTextAlignment = alignment;
    if( mParent )
        mParent->update();
}

Qt::Alignment KDChart::TextLayoutItem::textAlignment() const
{
    return mTextAlignment;
}

/**
  \brief Use this to specify the text attributes to be used for this item.

  \sa textAttributes
*/
void KDChart::TextLayoutItem::setTextAttributes( const TextAttributes &a )
{
    mAttributes = a;
    cachedFont = a.font();
    cachedSizeHint = QSize(); // invalidate size hint
    sizeHint();
    if( mParent )
        mParent->update();
}

/**
  Returns the text attributes to be used for this item.

  \sa setTextAttributes
*/
KDChart::TextAttributes KDChart::TextLayoutItem::textAttributes() const
{
    return mAttributes;
}


Qt::Orientations KDChart::TextLayoutItem::expandingDirections() const
{
    return 0; // Grow neither vertically nor horizontally
}

QRect KDChart::TextLayoutItem::geometry() const
{
    return mRect;
}

bool KDChart::TextLayoutItem::isEmpty() const
{
    return false; // never empty, otherwise the layout item would not exist
}

QSize KDChart::TextLayoutItem::maximumSize() const
{
    return sizeHint(); // PENDING(kalle) Review, quite inflexible
}

QSize KDChart::TextLayoutItem::minimumSize() const
{
    return sizeHint(); // PENDING(kalle) Review, quite inflexible
}

void KDChart::TextLayoutItem::setGeometry( const QRect& r )
{
    mRect = r;
}

QPointF rotatedPoint( const QPointF& pt, qreal rotation, const QPointF& center )
{
    const qreal angle = PI * rotation / 180.0;
    const qreal cosAngle = cos( angle );
    const qreal sinAngle = -sin( angle );
    return QPointF(
            (cosAngle * ( pt.x() - center.x() ) + sinAngle * ( pt.y() - center.y() ) ),
            (cosAngle * ( pt.y() - center.y() ) - sinAngle * ( pt.x() - center.x() ) ) ) + center;
}


QRectF rotatedRect( const QRectF& oldRect, qreal angleInt, const QPointF& center )
{
    const QRect rect( oldRect.translated( center ).toRect() );
    const qreal angle = PI * angleInt / 180.0;
    const qreal cosAngle = cos( angle );
    const qreal sinAngle = sin( angle );
    QMatrix rotationMatrix(cosAngle, sinAngle, -sinAngle, cosAngle, 0, 0);
    QPolygon rotPts;
    rotPts <<  rotationMatrix.map(rect.topLeft()) //QPoint(0,0)
            << rotationMatrix.map(rect.topRight())
            << rotationMatrix.map(rect.bottomRight())
            << rotationMatrix.map(rect.bottomLeft());
            //<< rotatedPoint(rect.topRight(), angleInt, center).toPoint()
            //<< rotatedPoint(rect.bottomRight(), angleInt, center).toPoint()
            //<< rotatedPoint(rect.bottomLeft(), angleInt, center).toPoint();
    return rotPts.boundingRect();
/*
    const QPointF topLeft( rotatedPoint( oldRect.topLeft(), angle, center ) );
    const QPointF topRight( rotatedPoint( oldRect.topRight(), angle, center ) );
    const QPointF bottomLeft( rotatedPoint( oldRect.bottomLeft(), angle, center ) );
    const QPointF bottomRight( rotatedPoint( oldRect.bottomRight(), angle, center ) );

    const qreal x = qMin( qMin( topLeft.x(), topRight.x() ), qMin( bottomLeft.x(), topLeft.x() ) );
    const qreal y = qMin( qMin( topLeft.y(), topRight.y() ), qMin( bottomLeft.y(), topLeft.y() ) );
    const qreal width = qMax( qMax( topLeft.x(), topRight.x() ), qMax( bottomLeft.x(), topLeft.x() ) ) - x;
    const qreal height = qMax( qMax( topLeft.y(), topRight.y() ), qMax( bottomLeft.y(), topLeft.y() ) ) - y;

    return QRectF( x, y, width, height );
*/
}

/*
QRectF rotatedRect( const QRectF& rect, qreal angle )
{
    const QPointF topLeft(  rotatedPoint( rect.topLeft(),  angle ) );
    //const QPointF topRight( rotatedPoint( rect.topRight(), angle ) );
    //const QPointF bottomLeft(  rotatedPoint( rect.bottomLeft(),  angle ) );
#if 1
    const QPointF bottomRight( rotatedPoint( rect.bottomRight(), angle ) );
    const QRectF result( topLeft, QSizeF( bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y() ) );
#else
    const QPointF siz( rotatedPoint( QPointF( rect.size().width(), rect.size().height() ), angle ) );
    const QRectF result(
            topLeft,
            QSizeF( siz.x(), //bottomRight.x() - topLeft.x(),
                    siz.y() ) ); //bottomRight.y() - topLeft.y() ) );
    //qDebug() << "angle" << angle << "\nbefore:" << rect << "\n after:" << result;
#endif
    return result;
}*/

qreal KDChart::TextLayoutItem::fitFontSizeToGeometry() const
{
    QFont f = realFont();
    const qreal origResult = f.pointSizeF();
    qreal result = origResult;
    const QSize mySize = geometry().size();
    if( mySize.isNull() )
        return result;

    const QString t = text();
    QFontMetrics fm( f );
    while( true )
    {
        const QSizeF textSize = rotatedRect( fm.boundingRect( t ), mAttributes.rotation() ).normalized().size();

        if( textSize.height() <= mySize.height() && textSize.width() <= mySize.width() )
            return result;

        result -= 0.5;
        if( result <= 0.0 )
            return origResult;
        f.setPointSizeF( result );
        fm = QFontMetrics( f );
    }
}

qreal KDChart::TextLayoutItem::realFontSize() const
{
    return mAttributes.calculatedFontSize( mAutoReferenceArea, mAutoReferenceOrientation );
}


bool KDChart::TextLayoutItem::realFontWasRecalculated() const
{
    const qreal fntSiz = realFontSize();
    const bool bRecalcDone =
        ( ( ! cachedSizeHint.isValid() ) || ( cachedFontSize != fntSiz   ) );

    if( bRecalcDone && fntSiz > 0.0 ){
        cachedFontSize = fntSiz;
        cachedFont.setPointSizeF( fntSiz );
    }
    return bRecalcDone;
}


QFont KDChart::TextLayoutItem::realFont() const
{
    realFontWasRecalculated(); // we can safely ignore the boolean return value
    return cachedFont;
}

QPolygon KDChart::TextLayoutItem::rotatedCorners() const
{
    // the angle in rad
    const qreal angle = mAttributes.rotation() * PI / 180.0;
    QSize size = unrotatedSizeHint();

    // my P1 - P4 (the four points of the rotated area)
    QPointF P1( size.height() * sin( angle ), 0 );
    QPointF P2( size.height() * sin( angle ) + size.width() * cos( angle ), size.width() * sin( angle ) );
    QPointF P3( size.width() * cos( angle ), size.width() * sin( angle ) + size.height() * cos( angle ) );
    QPointF P4( 0, size.height() * cos( angle ) );

    QPolygon result;
    result << P1.toPoint() << P2.toPoint() << P3.toPoint() << P4.toPoint();
    return result;
}

bool KDChart::TextLayoutItem::intersects( const TextLayoutItem& other, const QPointF& myPos, const QPointF& otherPos ) const
{
    return intersects( other, myPos.toPoint(), otherPos.toPoint() );
}

bool KDChart::TextLayoutItem::intersects( const TextLayoutItem& other, const QPoint& myPos, const QPoint& otherPos ) const
{
    if ( mAttributes.rotation() != other.mAttributes.rotation() )
    {
        // that's the code for the common case: the rotation angles don't need to match here
        QPolygon myPolygon(          rotatedCorners() );
        QPolygon otherPolygon( other.rotatedCorners() );

        // move the polygons to their positions
        myPolygon.translate( myPos );
        otherPolygon.translate( otherPos );

        // create regions out of it
        QRegion myRegion( myPolygon );
        QRegion otherRegion( otherPolygon );

        // now the question - do they intersect or not?
        return ! myRegion.intersect( otherRegion ).isEmpty();

    } else {
        // and that's the code for the special case: the rotation angles match, which is less time consuming in calculation
        const qreal angle = mAttributes.rotation() * PI / 180.0;
        // both sizes
        const QSizeF mySize(          unrotatedSizeHint() );
        const QSizeF otherSize( other.unrotatedSizeHint() );

        // that's myP1 relative to myPos
        QPointF myP1( mySize.height() * sin( angle ), 0.0 );
        // that's otherP1 to myPos
        QPointF otherP1 = QPointF( otherSize.height() * sin( angle ), 0.0 ) + otherPos - myPos;

        // now rotate both points the negative angle around myPos
        myP1 = QPointF( myP1.x() * cos( -angle ), myP1.x() * sin( -angle ) );
        qreal r = sqrt( otherP1.x() * otherP1.x() + otherP1.y() * otherP1.y() );
        otherP1 = QPointF( r * cos( -angle ), r * sin( -angle ) );

        // finally we look, whether both rectangles intersect or even not
        const bool res = QRectF( myP1, mySize ).intersects( QRectF( otherP1, otherSize ) );
        //qDebug() << res << QRectF( myP1, mySize ) << QRectF( otherP1, otherSize );
        return res;
    }
}

QSize KDChart::TextLayoutItem::sizeHint() const
{
    QPoint dummy;
    return sizeHintAndRotatedCorners(dummy,dummy,dummy,dummy);
}

QSize KDChart::TextLayoutItem::sizeHintAndRotatedCorners(
        QPoint& topLeftPt, QPoint& topRightPt, QPoint& bottomRightPt, QPoint& bottomLeftPt) const
{
    if( realFontWasRecalculated() || mAttributes.rotation() )
    {
        const QSize newSizeHint( calcSizeHint( cachedFont,
                                               topLeftPt, topRightPt, bottomRightPt, bottomLeftPt ) );
        if( newSizeHint != cachedSizeHint ){
            cachedSizeHint = newSizeHint;
            sizeHintChanged();
        }
        cachedTopLeft     = topLeftPt;
        cachedTopRight    = topRightPt;
        cachedBottomRight = bottomRightPt;
        cachedBottomLeft  = bottomLeftPt;
    }else{
        topLeftPt     = cachedTopLeft;
        topRightPt    = cachedTopRight;
        bottomRightPt = cachedBottomRight;
        bottomLeftPt  = cachedBottomLeft;
    }
    //qDebug() << "-------- KDChart::TextLayoutItem::sizeHint() returns:"<<cachedSizeHint<<" ----------";
    return cachedSizeHint;
}

QSize KDChart::TextLayoutItem::sizeHintUnrotated() const
{
    realFontWasRecalculated(); // make sure the cached font is updated if needed
    return unrotatedSizeHint( cachedFont );
}


// PENDING(kalle) Support auto shrink


QSize KDChart::TextLayoutItem::unrotatedSizeHint( QFont fnt ) const
{
    if ( fnt == QFont() )
        fnt = realFont(); // this *is* the chached font in most of the time

    const QFontMetricsF met( fnt, GlobalMeasureScaling::paintDevice() );
    QSize ret(0, 0);
    // note: boundingRect() does NOT take any newlines into account
    //       so we need to calculate the size by combining several
    //       rectangles: one per line.  This fixes bugz issue #3720.
    //       (khz, 2007 04 14)
    QStringList lines = mText.split(QString::fromAscii("\n"));
    for (int i = 0; i < lines.size(); ++i) {
        const QSize lSize = met.boundingRect(lines.at(i) ).toRect().size();
        ret.setWidth(qMax( ret.width(), lSize.width() ));
        ret.rheight() += lSize.height();
    }

    int frame = QApplication::style()->pixelMetric( QStyle::PM_ButtonMargin, 0, 0 );
    // fine-tuning for small font sizes: the frame must not be so big, if the font is tiny
    frame = qMin( frame, ret.height() * 2 / 3 );
    //qDebug() << "frame:"<< frame;
    ret += QSize( frame, frame );
    return ret;
    //const QFontMetricsF met( fnt, GlobalMeasureScaling::paintDevice() );
    //const int frame = QApplication::style()->pixelMetric( QStyle::PM_ButtonMargin, 0, 0 );
    //return
    //    met.boundingRect( mText ).size().toSize() + QSize( frame, frame );
}


QSize KDChart::TextLayoutItem::calcSizeHint(
        QFont fnt, QPoint& topLeftPt, QPoint& topRightPt, QPoint& bottomRightPt, QPoint& bottomLeftPt ) const
{
    const QSize siz( unrotatedSizeHint( fnt ));
    //qDebug() << "-------- siz: "<<siz;
    if( ! mAttributes.rotation() ){
        topLeftPt     = QPoint(0,0);
        topRightPt    = QPoint(siz.width(),0);
        bottomRightPt = QPoint(siz.width(),siz.height());
        bottomLeftPt  = QPoint(0,siz.height());
        return siz;
    }

    const QRect rect(QPoint(0, 0), siz + QSize(4,4));
    const qreal angle = PI * mAttributes.rotation() / 180.0;
    const qreal cosAngle = cos( angle );
    const qreal sinAngle = sin( angle );
    QMatrix rotationMatrix(cosAngle, sinAngle, -sinAngle, cosAngle, 0, 0);
    QPolygon rotPts;
    rotPts << rotationMatrix.map(rect.topLeft())
           << rotationMatrix.map(rect.topRight())
           << rotationMatrix.map(rect.bottomRight())
           << rotationMatrix.map(rect.bottomLeft());
    QSize rotSiz( rotPts.boundingRect().size() );
    //qDebug() << "-------- KDChart::TextLayoutItem::calcSizeHint() returns:"<<rotSiz<<rotPts;
    topLeftPt     = rotPts[0];
    topRightPt    = rotPts[1];
    bottomRightPt = rotPts[2];
    bottomLeftPt  = rotPts[3];
    return rotSiz;
}


void KDChart::TextLayoutItem::paint( QPainter* painter )
{
    // make sure, cached font is updated, if needed:
    // sizeHint();

    if( !mRect.isValid() )
        return;

    const PainterSaver painterSaver( painter );
    QFont f = realFont();
    if ( mAttributes.autoShrink() )
        f.setPointSizeF( fitFontSizeToGeometry() );
    painter->setFont( f );
    QRectF rect( geometry() );

// #ifdef DEBUG_ITEMS_PAINT
//     painter->setPen( Qt::black );
//     painter->drawRect( rect );
// #endif
    painter->translate( rect.center() );
    rect.moveTopLeft( QPointF( - rect.width() / 2, - rect.height() / 2 ) );
#ifdef DEBUG_ITEMS_PAINT
    painter->setPen( Qt::blue );
    painter->drawRect( rect );
    painter->drawRect( QRect(QPoint((rect.topLeft().toPoint()  + rect.bottomLeft().toPoint())  / 2 - QPoint(2,2)), QSize(3,3)) );
    //painter->drawRect( QRect(QPoint((rect.topRight().toPoint() + rect.bottomRight().toPoint()) / 2 - QPoint(2,2)), QSize(3,3)) );
#endif
    painter->rotate( mAttributes.rotation() );
    rect = rotatedRect( rect, mAttributes.rotation() );
#ifdef DEBUG_ITEMS_PAINT
    painter->setPen( Qt::red );
    painter->drawRect( rect );
    painter->drawRect( QRect(QPoint((rect.topLeft().toPoint()  + rect.bottomLeft().toPoint())  / 2 - QPoint(2,2)), QSize(3,3)) );
    //painter->drawRect( QRect(QPoint((rect.topRight().toPoint() + rect.bottomRight().toPoint()) / 2 - QPoint(2,2)), QSize(3,3)) );
#endif
    painter->setPen( PrintingParameters::scalePen( mAttributes.pen() ) );
    QFontMetrics fontMetrics( f );
    const int AHight = fontMetrics.boundingRect( QChar::fromAscii( 'A' ) ).height();
    const qreal AVCenter = fontMetrics.ascent() - AHight / 2.0;
    // Make sure that capital letters are vertically centered. This looks much
    // better than just centering the text's bounding rect.
    rect.translate( 0.0, rect.height() / 2.0 - AVCenter );
    //painter->drawText( rect, Qt::AlignHCenter | Qt::AlignTop, mText );
    painter->drawText( rect, mTextAlignment, mText );

//    if (  calcSizeHint( realFont() ).width() > rect.width() )
//        qDebug() << "rect.width()" << rect.width() << "text.width()" << calcSizeHint( realFont() ).width();
//
//    //painter->drawText( rect, Qt::AlignHCenter | Qt::AlignVCenter, mText );
}

KDChart::HorizontalLineLayoutItem::HorizontalLineLayoutItem()
    : AbstractLayoutItem( Qt::AlignCenter )
{
}

Qt::Orientations KDChart::HorizontalLineLayoutItem::expandingDirections() const
{
    return Qt::Vertical|Qt::Horizontal; // Grow both vertically, and horizontally
}

QRect KDChart::HorizontalLineLayoutItem::geometry() const
{
    return mRect;
}

bool KDChart::HorizontalLineLayoutItem::isEmpty() const
{
    return false; // never empty, otherwise the layout item would not exist
}

QSize KDChart::HorizontalLineLayoutItem::maximumSize() const
{
    return QSize( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
}

QSize KDChart::HorizontalLineLayoutItem::minimumSize() const
{
    return QSize( 0, 0 );
}

void KDChart::HorizontalLineLayoutItem::setGeometry( const QRect& r )
{
    mRect = r;
}

QSize KDChart::HorizontalLineLayoutItem::sizeHint() const
{
    return QSize( -1, 3 ); // see qframe.cpp
}


void KDChart::HorizontalLineLayoutItem::paint( QPainter* painter )
{
    if( !mRect.isValid() )
        return;

    painter->drawLine( QPointF( mRect.left(), mRect.center().y() ),
                       QPointF( mRect.right(), mRect.center().y() ) );
}


KDChart::VerticalLineLayoutItem::VerticalLineLayoutItem()
    : AbstractLayoutItem( Qt::AlignCenter )
{
}

Qt::Orientations KDChart::VerticalLineLayoutItem::expandingDirections() const
{
    return Qt::Vertical|Qt::Vertical; // Grow both vertically, and horizontally
}

QRect KDChart::VerticalLineLayoutItem::geometry() const
{
    return mRect;
}

bool KDChart::VerticalLineLayoutItem::isEmpty() const
{
    return false; // never empty, otherwise the layout item would not exist
}

QSize KDChart::VerticalLineLayoutItem::maximumSize() const
{
    return QSize( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
}

QSize KDChart::VerticalLineLayoutItem::minimumSize() const
{
    return QSize( 0, 0 );
}

void KDChart::VerticalLineLayoutItem::setGeometry( const QRect& r )
{
    mRect = r;
}

QSize KDChart::VerticalLineLayoutItem::sizeHint() const
{
    return QSize( 3, -1 ); // see qframe.cpp
}


void KDChart::VerticalLineLayoutItem::paint( QPainter* painter )
{
    if( !mRect.isValid() )
        return;

    painter->drawLine( QPointF( mRect.center().x(), mRect.top() ),
                       QPointF( mRect.center().x(), mRect.bottom() ) );
}



KDChart::MarkerLayoutItem::MarkerLayoutItem( KDChart::AbstractDiagram* diagram,
                                             const MarkerAttributes& marker,
                                             const QBrush& brush, const QPen& pen,
                                             Qt::Alignment alignment )
    : AbstractLayoutItem( alignment )
    , mDiagram( diagram )
    , mMarker( marker )
    , mBrush( brush )
    , mPen( pen )
{
}

Qt::Orientations KDChart::MarkerLayoutItem::expandingDirections() const
{
    return 0; // Grow neither vertically nor horizontally
}

QRect KDChart::MarkerLayoutItem::geometry() const
{
    return mRect;
}

bool KDChart::MarkerLayoutItem::isEmpty() const
{
    return false; // never empty, otherwise the layout item would not exist
}

QSize KDChart::MarkerLayoutItem::maximumSize() const
{
    return sizeHint(); // PENDING(kalle) Review, quite inflexible
}

QSize KDChart::MarkerLayoutItem::minimumSize() const
{
    return sizeHint(); // PENDING(kalle) Review, quite inflexible
}

void KDChart::MarkerLayoutItem::setGeometry( const QRect& r )
{
    mRect = r;
}

QSize KDChart::MarkerLayoutItem::sizeHint() const
{
    //qDebug() << "KDChart::MarkerLayoutItem::sizeHint() returns:"<<mMarker.markerSize().toSize();
    return mMarker.markerSize().toSize();
}

void KDChart::MarkerLayoutItem::paint( QPainter* painter )
{
    paintIntoRect( painter, mRect, mDiagram, mMarker, mBrush, mPen );
}

void KDChart::MarkerLayoutItem::paintIntoRect(
        QPainter* painter,
        const QRect& rect,
        AbstractDiagram* diagram,
        const MarkerAttributes& marker,
        const QBrush& brush,
        const QPen& pen )
{
    if( ! rect.isValid() )
        return;

    // The layout management may assign a larger rect than what we
    // wanted. We need to adjust the position.
    const QSize siz = marker.markerSize().toSize();
    QPointF pos = rect.topLeft();
    pos += QPointF( static_cast<qreal>(( rect.width()  - siz.width()) / 2.0 ),
                    static_cast<qreal>(( rect.height() - siz.height()) / 2.0 ) );

#ifdef DEBUG_ITEMS_PAINT
    QPointF oldPos = pos;
#endif

// And finally, drawMarker() assumes the position to be the center
    // of the marker, adjust again.
    pos += QPointF( static_cast<qreal>( siz.width() ) / 2.0,
                    static_cast<qreal>( siz.height() )/ 2.0 );

    diagram->paintMarker( painter, marker, brush, pen, pos.toPoint(), siz );

#ifdef DEBUG_ITEMS_PAINT
    const QPen oldPen( painter->pen() );
    painter->setPen( Qt::red );
    painter->drawRect( QRect(oldPos.toPoint(), siz) );
    painter->setPen( oldPen );
#endif
}


KDChart::LineLayoutItem::LineLayoutItem( KDChart::AbstractDiagram* diagram,
                                         int length,
                                         const QPen& pen,
                                         Qt::Alignment alignment )
    : AbstractLayoutItem( alignment )
    , mDiagram( diagram )
    , mLength( length )
    , mPen( pen )
{
    //have a mini pen width
    if ( pen.width() < 2 )
        mPen.setWidth( 2 );
}

Qt::Orientations KDChart::LineLayoutItem::expandingDirections() const
{
    return 0; // Grow neither vertically nor horizontally
}

QRect KDChart::LineLayoutItem::geometry() const
{
    return mRect;
}

bool KDChart::LineLayoutItem::isEmpty() const
{
    return false; // never empty, otherwise the layout item would not exist
}

QSize KDChart::LineLayoutItem::maximumSize() const
{
    return sizeHint(); // PENDING(kalle) Review, quite inflexible
}

QSize KDChart::LineLayoutItem::minimumSize() const
{
    return sizeHint(); // PENDING(kalle) Review, quite inflexible
}

void KDChart::LineLayoutItem::setGeometry( const QRect& r )
{
    mRect = r;
}

QSize KDChart::LineLayoutItem::sizeHint() const
{
    return QSize( mLength, mPen.width()+2 );
}

void KDChart::LineLayoutItem::paint( QPainter* painter )
{
    paintIntoRect( painter, mRect, mPen );
}

void KDChart::LineLayoutItem::paintIntoRect(
        QPainter* painter,
        const QRect& rect,
        const QPen& pen )
{
    if( ! rect.isValid() )
        return;

    const QPen oldPen = painter->pen();
    painter->setPen( PrintingParameters::scalePen( pen ) );
    const qreal y = rect.center().y();
    painter->drawLine( QPointF( rect.left(), y ),
                       QPointF( rect.right(), y ) );
    painter->setPen( oldPen );
}


KDChart::LineWithMarkerLayoutItem::LineWithMarkerLayoutItem(
        KDChart::AbstractDiagram* diagram,
        int lineLength,
        const QPen& linePen,
        int markerOffs,
        const MarkerAttributes& marker,
        const QBrush& markerBrush,
        const QPen& markerPen,
        Qt::Alignment alignment )
    : AbstractLayoutItem( alignment )
    , mDiagram(     diagram )
    , mLineLength(  lineLength )
    , mLinePen(     linePen )
    , mMarkerOffs(  markerOffs )
    , mMarker(      marker )
    , mMarkerBrush( markerBrush )
    , mMarkerPen(   markerPen )
{
}

Qt::Orientations KDChart::LineWithMarkerLayoutItem::expandingDirections() const
{
    return 0; // Grow neither vertically nor horizontally
}

QRect KDChart::LineWithMarkerLayoutItem::geometry() const
{
    return mRect;
}

bool KDChart::LineWithMarkerLayoutItem::isEmpty() const
{
    return false; // never empty, otherwise the layout item would not exist
}

QSize KDChart::LineWithMarkerLayoutItem::maximumSize() const
{
    return sizeHint(); // PENDING(kalle) Review, quite inflexible
}

QSize KDChart::LineWithMarkerLayoutItem::minimumSize() const
{
    return sizeHint(); // PENDING(kalle) Review, quite inflexible
}

void KDChart::LineWithMarkerLayoutItem::setGeometry( const QRect& r )
{
    mRect = r;
}

QSize KDChart::LineWithMarkerLayoutItem::sizeHint() const
{
    const QSize sizeM = mMarker.markerSize().toSize();
    const QSize sizeL = QSize( mLineLength, mLinePen.width()+2 );
    return QSize( qMax(sizeM.width(),  sizeL.width()),
                  qMax(sizeM.height(), sizeL.height()) );
}

void KDChart::LineWithMarkerLayoutItem::paint( QPainter* painter )
{
    // paint the line over the full width, into the vertical middle of the rect
    LineLayoutItem::paintIntoRect( painter, mRect, mLinePen );

    // paint the marker with the given offset from the left side of the line
    const QRect r(
            QPoint( mRect.x()+mMarkerOffs, mRect.y() ),
            QSize( mMarker.markerSize().toSize().width(), mRect.height() ) );
    MarkerLayoutItem::paintIntoRect(
            painter, r, mDiagram, mMarker, mMarkerBrush, mMarkerPen );
}



KDChart::AutoSpacerLayoutItem::AutoSpacerLayoutItem(
        bool layoutIsAtTopPosition, QHBoxLayout *rightLeftLayout,
        bool layoutIsAtLeftPosition, QVBoxLayout *topBottomLayout )
    : AbstractLayoutItem( Qt::AlignCenter )
    , mLayoutIsAtTopPosition(  layoutIsAtTopPosition )
    , mRightLeftLayout( rightLeftLayout )
    , mLayoutIsAtLeftPosition( layoutIsAtLeftPosition )
    , mTopBottomLayout( topBottomLayout )
{
}

Qt::Orientations KDChart::AutoSpacerLayoutItem::expandingDirections() const
{
    return 0; // Grow neither vertically nor horizontally
}

QRect KDChart::AutoSpacerLayoutItem::geometry() const
{
    return mRect;
}

bool KDChart::AutoSpacerLayoutItem::isEmpty() const
{
    return true; // never empty, otherwise the layout item would not exist
}

QSize KDChart::AutoSpacerLayoutItem::maximumSize() const
{
    return sizeHint();
}

QSize KDChart::AutoSpacerLayoutItem::minimumSize() const
{
    return sizeHint();
}

void KDChart::AutoSpacerLayoutItem::setGeometry( const QRect& r )
{
    mRect = r;
}


static void updateCommonBrush( QBrush& commonBrush, bool& bStart, const KDChart::AbstractArea& area )
{
    const KDChart::BackgroundAttributes ba( area.backgroundAttributes() );
    const bool hasSimpleBrush = (
            ! area.frameAttributes().isVisible() &&
            ba.isVisible() &&
            ba.pixmapMode() == KDChart::BackgroundAttributes::BackgroundPixmapModeNone &&
            ba.brush().gradient() == 0 );
    if( bStart ){
        bStart = false;
        commonBrush = hasSimpleBrush ? ba.brush() : QBrush();
    }else{
        if( ! hasSimpleBrush || ba.brush() != commonBrush )
        {
            commonBrush = QBrush();
        }
    }
}

QSize KDChart::AutoSpacerLayoutItem::sizeHint() const
{
    QBrush commonBrush;
    bool bStart=true;
    // calculate the maximal overlap of the top/bottom axes:
    int topBottomOverlap = 0;
    if( mTopBottomLayout ){
        for (int i = 0; i < mTopBottomLayout->count(); ++i){
            AbstractArea* area = dynamic_cast<AbstractArea*>(mTopBottomLayout->itemAt(i));
            if( area ){
                //qDebug() << "AutoSpacerLayoutItem testing" << area;
                topBottomOverlap =
                    mLayoutIsAtLeftPosition
                    ? qMax( topBottomOverlap, area->rightOverlap() )
                    : qMax( topBottomOverlap, area->leftOverlap() );
                updateCommonBrush( commonBrush, bStart, *area );
            }
        }
    }
    // calculate the maximal overlap of the left/right axes:
    int leftRightOverlap = 0;
    if( mRightLeftLayout ){
        for (int i = 0; i < mRightLeftLayout->count(); ++i){
            AbstractArea* area = dynamic_cast<AbstractArea*>(mRightLeftLayout->itemAt(i));
            if( area ){
                //qDebug() << "AutoSpacerLayoutItem testing" << area;
                leftRightOverlap =
                        mLayoutIsAtTopPosition
                        ? qMax( leftRightOverlap, area->bottomOverlap() )
                        : qMax( leftRightOverlap, area->topOverlap() );
                updateCommonBrush( commonBrush, bStart, *area );
            }
        }
    }
    if( topBottomOverlap > 0 && leftRightOverlap > 0 )
        mCommonBrush = commonBrush;
    else
        mCommonBrush = QBrush();
    mCachedSize = QSize( topBottomOverlap, leftRightOverlap );
    //qDebug() << mCachedSize;
    return mCachedSize;
}


void KDChart::AutoSpacerLayoutItem::paint( QPainter* painter )
{
    if( mParentLayout && mRect.isValid() && mCachedSize.isValid() &&
        mCommonBrush.style() != Qt::NoBrush )
    {
        QPoint p1( mRect.topLeft() );
        QPoint p2( mRect.bottomRight() );
        if( mLayoutIsAtLeftPosition )
            p1.rx() += mCachedSize.width() - mParentLayout->spacing();
        else
            p2.rx() -= mCachedSize.width() - mParentLayout->spacing();
        if( mLayoutIsAtTopPosition ){
            p1.ry() += mCachedSize.height() - mParentLayout->spacing() - 1;
            p2.ry() -= 1;
        }else
            p2.ry() -= mCachedSize.height() - mParentLayout->spacing() - 1;
        //qDebug() << mLayoutIsAtTopPosition << mLayoutIsAtLeftPosition;
        //qDebug() << mRect;
        //qDebug() << mParentLayout->margin();
        //qDebug() << QRect( p1, p2 );
        const QPoint oldBrushOrigin( painter->brushOrigin() );
        const QBrush oldBrush( painter->brush() );
        const QPen   oldPen(   painter->pen() );
        const QPointF newTopLeft( painter->deviceMatrix().map( p1 ) );
        painter->setBrushOrigin( newTopLeft );
        painter->setBrush( mCommonBrush );
        painter->setPen( Qt::NoPen );
        painter->drawRect( QRect( p1, p2 ) );
        painter->setBrushOrigin( oldBrushOrigin );
        painter->setBrush( oldBrush );
        painter->setPen( oldPen );
    }
    // debug code:
#if 0
    //qDebug() << "KDChart::AutoSpacerLayoutItem::paint()";
    if( !mRect.isValid() )
        return;

    painter->drawRect( mRect );
    painter->drawLine( QPointF( mRect.x(), mRect.top() ),
                       QPointF( mRect.right(), mRect.bottom() ) );
    painter->drawLine( QPointF( mRect.right(), mRect.top() ),
                       QPointF( mRect.x(), mRect.bottom() ) );
#endif
}
