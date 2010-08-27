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

#include "KDChartChart.h"
#include "KDChartChart_p.h"

#include <QList>
#include <QtDebug>
#include <QGridLayout>
#include <QLabel>
#include <QHash>
#include <QToolTip>
#include <QPainter>
#include <QPaintEvent>
#include <QLayoutItem>
#include <QPushButton>
#include <QApplication>
#include <QEvent>

#include "KDChartCartesianCoordinatePlane.h"
#include "KDChartAbstractCartesianDiagram.h"
#include "KDChartHeaderFooter.h"
#include "KDChartEnums.h"
#include "KDChartLegend.h"
#include "KDChartLayoutItems.h"
#include <KDChartTextAttributes.h>
#include <KDChartMarkerAttributes>
#include "KDChartPainterSaver_p.h"
#include "KDChartPrintingParameters.h"

#if defined KDAB_EVAL
#include "../evaldialog/evaldialog.h"
#endif

#include <KDABLibFakes>

#define SET_ALL_MARGINS_TO_ZERO

// Layout widgets even if they are not visible
class MyWidgetItem : public QWidgetItem
{
public:
    explicit MyWidgetItem(QWidget *w, Qt::Alignment alignment = 0)
        : QWidgetItem(w) {
        setAlignment( alignment );
    }
    /*reimp*/ bool isEmpty() const {
        QWidget* w = const_cast<MyWidgetItem *>(this)->widget();
        // legend->hide() should indeed hide the legend,
        // but a legend in a chart that hasn't been shown yet isn't hidden
        // (as can happen when using Chart::paint() without showing the chart)
        return w->isHidden() && w->testAttribute(Qt::WA_WState_ExplicitShowHide);
    }
};

using namespace KDChart;

void Chart::Private::slotUnregisterDestroyedLegend( Legend *l )
{
    legends.removeAll( l );
    slotRelayout();
}

void Chart::Private::slotUnregisterDestroyedHeaderFooter( HeaderFooter* hf )
{
    headerFooters.removeAll( hf );
    hf->removeFromParentLayout();
    textLayoutItems.remove( textLayoutItems.indexOf( hf ) );
    slotRelayout();
}

void Chart::Private::slotUnregisterDestroyedPlane( AbstractCoordinatePlane* plane )
{
    coordinatePlanes.removeAll( plane );
    Q_FOREACH ( AbstractCoordinatePlane* p, coordinatePlanes )
    {
        if ( p->referenceCoordinatePlane() == plane) {
            p->setReferenceCoordinatePlane(0);
        }
    }
    plane->layoutPlanes();
}

Chart::Private::Private( Chart* chart_ )
    : chart( chart_ )
    , layout( 0 )
    , vLayout( 0 )
    , planesLayout( 0 )
    , headerLayout( 0 )
    , footerLayout( 0 )
    , dataAndLegendLayout( 0 )
    , globalLeadingLeft( 0 )
    , globalLeadingRight( 0 )
    , globalLeadingTop( 0 )
    , globalLeadingBottom( 0 )
{
    for( int row = 0; row < 3; ++row )
    {
        for( int column = 0; column < 3; ++column )
        {
            dummyHeaders[ row ][ column ] = HorizontalLineLayoutItem();
            dummyFooters[ row ][ column ] = HorizontalLineLayoutItem();
            innerHdFtLayouts[0][row][column] = 0;
            innerHdFtLayouts[1][row][column] = 0;
        }
    }
}

Chart::Private::~Private()
{
    removeDummyHeaderFooters();
}

void Chart::Private::removeDummyHeaderFooters()
{
    for ( int row = 0; row < 3; ++row )
    {
        for ( int column = 0; column < 3; ++ column )
        {
            if( innerHdFtLayouts[0][row][column] ){
                innerHdFtLayouts[0][row][column]->removeItem( &(dummyHeaders[row][column]) );
                innerHdFtLayouts[1][row][column]->removeItem( &(dummyFooters[row][column]) );
            }
        }
    }
}

void Chart::Private::layoutHeadersAndFooters()
{
    removeDummyHeaderFooters();

    bool headersLineFilled[] = { false, false, false };
    bool footersLineFilled[] = { false, false, false };

    Q_FOREACH( HeaderFooter *hf, headerFooters ) {
        // for now, there are only two types of Header/Footer,
        // we use a pointer to the right layout, depending on the type():
        int innerLayoutIdx = 0;
        switch( hf->type() ){
            case HeaderFooter::Header:
                innerLayoutIdx = 0;
                break;
            case HeaderFooter::Footer:
                innerLayoutIdx = 1;
                break;
            default:
                Q_ASSERT( false ); // all types need to be handled
                break;
        };

        if( hf->position() != Position::Unknown ) {
            int row, column;
            Qt::Alignment hAlign, vAlign;
            if( hf->position().isNorthSide() ){
                row = 0;
                vAlign = Qt::AlignTop;
            }
            else if( hf->position().isSouthSide() ){
                row = 2;
                vAlign = Qt::AlignBottom;
            }
            else{
                row = 1;
                vAlign = Qt::AlignVCenter;
            }
            if( hf->position().isWestSide() ){
                column = 0;
                hAlign = Qt::AlignLeft;
            }
            else if( hf->position().isEastSide() ){
                column = 2;
                hAlign = Qt::AlignRight;
            }
            else{
                column = 1;
                hAlign = Qt::AlignHCenter;
            }
            switch( hf->type() ){
                case HeaderFooter::Header:
                    if( !headersLineFilled[ row ] )
                    {
                        for( int col = 0; col < 3; ++col )
                            innerHdFtLayouts[0][row][col]->addItem( &(dummyHeaders[ row ][ col ]) );
                        headersLineFilled[ row ] = true;
                    }
                    break;
                case HeaderFooter::Footer:
                    if( !footersLineFilled[ row ] )
                    {
                        for( int col = 0; col < 3; ++col )
                            innerHdFtLayouts[1][row][col]->addItem( &(dummyFooters[ row ][ col ]) );
                        footersLineFilled[ row ] = true;
                    }
                    break;
            };
            textLayoutItems << hf;
            QVBoxLayout* headerFooterLayout = innerHdFtLayouts[innerLayoutIdx][row][column];
            hf->setParentLayout( headerFooterLayout );
            hf->setAlignment( hAlign | vAlign );
            headerFooterLayout->addItem( hf );
        }
        else{
            qDebug( "Unknown header/footer position" );
        }
    }
}

void Chart::Private::layoutLegends()
{
    //qDebug() << "starting Chart::Private::layoutLegends()";
    // To support more than one Legend, we first collect them all
    // in little lists: one list per grid position.
    // Since the dataAndLegendLayout is a 3x3 grid, we need 9 little lists.
    QList<Legend*> infos[3][3];

    Q_FOREACH( Legend *legend, legends ) {

        legend->needSizeHint(); // we'll lay it out soon

        bool bOK = true;
        int row, column;
        //qDebug() << legend->position().name();
        switch( legend->position().value() ) {
            case KDChartEnums::PositionNorthWest:  row = 0;  column = 0;
                break;
            case KDChartEnums::PositionNorth:      row = 0;  column = 1;
                break;
            case KDChartEnums::PositionNorthEast:  row = 0;  column = 2;
                break;
            case KDChartEnums::PositionEast:       row = 1;  column = 2;
                break;
            case KDChartEnums::PositionSouthEast:  row = 2;  column = 2;
                break;
            case KDChartEnums::PositionSouth:      row = 2;  column = 1;
                break;
            case KDChartEnums::PositionSouthWest:  row = 2;  column = 0;
                break;
            case KDChartEnums::PositionWest:       row = 1;  column = 0;
                break;
            case KDChartEnums::PositionCenter:
                qDebug( "Sorry: Legend not shown, because position Center is not supported." );
                bOK = false;
                break;
            case KDChartEnums::PositionFloating:
                bOK = false;
                break;
            default:
                qDebug( "Sorry: Legend not shown, because of unknown legend position." );
                bOK = false;
                break;
        }
        if( bOK )
            infos[row][column] << legend;
    }
    // We have collected all legend information,
    // so we can design their layout now.
    for (int iR = 0; iR < 3; ++iR) {
        for (int iC = 0; iC < 3; ++iC) {
            QList<Legend*>& list = infos[iR][iC];
            const int count = list.size();
            switch( count ){
            case 0:
                break;
            case 1: {
                    Legend* legend = list.first();
                    dataAndLegendLayout->addItem( new MyWidgetItem(legend),
                        iR, iC, 1, 1, legend->alignment() );
            }
                break;
            default: {
                    // We have more than one legend in the same cell
                    // of the big dataAndLegendLayout grid.
                    //
                    // So we need to find out, if they are aligned the
                    // same way:
                    // Those legends, that are aligned the same way, will be drawn
                    // leftbound, on top of each other, in a little VBoxLayout.
                    //
                    // If not al of the legends are aligned the same way,
                    // there will be a grid with 3 cells: for left/mid/right side
                    // (or top/mid/bottom side, resp.) legends
                    Legend* legend = list.first();
                    Qt::Alignment alignment = legend->alignment();
                    bool haveSameAlign = true;
                    for (int i = 1; i < count; ++i) {
                        legend = list.at(i);
                        if( alignment != legend->alignment() ){
                            haveSameAlign = false;
                            break;
                        }
                    }
                    if( haveSameAlign ){
                        QVBoxLayout* vLayout = new QVBoxLayout();
#if defined SET_ALL_MARGINS_TO_ZERO
                        vLayout->setMargin(0);
#endif
                        for (int i = 0; i < count; ++i) {
                            vLayout->addItem( new MyWidgetItem(list.at(i), Qt::AlignLeft) );
                        }
                        dataAndLegendLayout->addLayout( vLayout, iR, iC, 1, 1, alignment );
                    }else{
                        QGridLayout* gridLayout = new QGridLayout();
#if defined SET_ALL_MARGINS_TO_ZERO
                        gridLayout->setMargin(0);
#endif


#define ADD_VBOX_WITH_LEGENDS(row, column, align) \
{ \
    QVBoxLayout* innerLayout = new QVBoxLayout(); \
    for (int i = 0; i < count; ++i) { \
        legend = list.at(i); \
        if( legend->alignment() == ( align ) ) \
            innerLayout->addItem( new MyWidgetItem(legend, Qt::AlignLeft) ); \
    } \
    gridLayout->addLayout( innerLayout, row, column, ( align  ) ); \
}
                        ADD_VBOX_WITH_LEGENDS( 0, 0, Qt::AlignTop     | Qt::AlignLeft )
                        ADD_VBOX_WITH_LEGENDS( 0, 1, Qt::AlignTop     | Qt::AlignHCenter )
                        ADD_VBOX_WITH_LEGENDS( 0, 2, Qt::AlignTop     | Qt::AlignRight )

                        ADD_VBOX_WITH_LEGENDS( 1, 0, Qt::AlignVCenter | Qt::AlignLeft )
                        ADD_VBOX_WITH_LEGENDS( 1, 1, Qt::AlignCenter )
                        ADD_VBOX_WITH_LEGENDS( 1, 2, Qt::AlignVCenter | Qt::AlignRight )

                        ADD_VBOX_WITH_LEGENDS( 2, 0, Qt::AlignBottom  | Qt::AlignLeft )
                        ADD_VBOX_WITH_LEGENDS( 2, 1, Qt::AlignBottom  | Qt::AlignHCenter )
                        ADD_VBOX_WITH_LEGENDS( 2, 2, Qt::AlignBottom  | Qt::AlignRight )

                        dataAndLegendLayout->addLayout( gridLayout, iR, iC, 1, 1 );
                    }
                }
            }
        }
    }
    //qDebug() << "finished Chart::Private::layoutLegends()";
}


QHash<AbstractCoordinatePlane*, PlaneInfo> Chart::Private::buildPlaneLayoutInfos()
{
    /* There are two ways in which planes can be caused to interact in
     * where they are put layouting wise: The first is the reference plane. If
     * such a reference plane is set, on a plane, it will use the same cell in the
     * layout as that one. In addition to this, planes can share an axis. In that case
     * they will be laid out in relation to each other as suggested by the position
     * of the axis. If, for example Plane1 and Plane2 share an axis at position Left,
     * that will result in the layout: Axis Plane1 Plane 2, vertically. If Plane1
     * also happens to be Plane2's referece plane, both planes are drawn over each
     * other. The reference plane concept allows two planes to share the same space
     * even if neither has any axis, and in case there are shared axis, it is used
     * to decided, whether the planes should be painted on top of each other or
     * laid out vertically or horizontally next to each other. */
    QHash<CartesianAxis*, AxisInfo> axisInfos;
    QHash<AbstractCoordinatePlane*, PlaneInfo> planeInfos;
    Q_FOREACH(AbstractCoordinatePlane* plane, coordinatePlanes )
    {
        PlaneInfo p;
        // first check if we share space with another plane
        p.referencePlane = plane->referenceCoordinatePlane();
        planeInfos.insert( plane, p );

        Q_FOREACH( AbstractDiagram* abstractDiagram, plane->diagrams() ) {
            AbstractCartesianDiagram* diagram =
                    dynamic_cast<AbstractCartesianDiagram*> ( abstractDiagram );
            if( !diagram ) continue;

            Q_FOREACH( CartesianAxis* axis, diagram->axes() ) {
                if ( !axisInfos.contains( axis ) ) {
                    /* If this is the first time we see this axis, add it, with the
                     * current plane. The first plane added to the chart that has
                     * the axis associated with it thus "owns" it, and decides about
                     * layout. */
                    AxisInfo i;
                    i.plane = plane;
                    axisInfos.insert( axis, i );
                } else {
                    AxisInfo i = axisInfos[axis];
                    if ( i.plane == plane ) continue; // we don't want duplicates, only shared

                    /* The user expects diagrams to be added on top, and to the right
                     * so that horizontally we need to move the new diagram, vertically
                     * the reference one. */
                    PlaneInfo pi = planeInfos[plane];
                    // plane-to-plane linking overrides linking via axes
                    if ( !pi.referencePlane ) {
                        // we're not the first plane to see this axis, mark us as a slave
                        pi.referencePlane = i.plane;
                        if ( axis->position() == CartesianAxis::Left
                            ||  axis->position() == CartesianAxis::Right )
                            pi.horizontalOffset += 1;
                        planeInfos[plane] = pi;

                        pi = planeInfos[i.plane];
                        if ( axis->position() == CartesianAxis::Top
                                || axis->position() == CartesianAxis::Bottom  )
                            pi.verticalOffset += 1;

                        planeInfos[i.plane] = pi;
                    }
                }
            }
        }
        // Create a new grid layout for each plane that has no reference.
        p = planeInfos[plane];
        if ( p.referencePlane == 0 ) {
            p.gridLayout = new QGridLayout();
            // TESTING(khz): set the margin of all of the layouts to Zero
#if defined SET_ALL_MARGINS_TO_ZERO
            p.gridLayout->setMargin(0);
#endif
            planeInfos[plane] = p;
        }
    }
    return planeInfos;
}

    template <typename T>
static T* findOrCreateLayoutByObjectName( QLayout * parentLayout, const char* name )
{
    T *box = qFindChild<T*>( parentLayout, QString::fromLatin1( name ) );
    if ( !box ) {
        box = new T();
        // TESTING(khz): set the margin of all of the layouts to Zero
#if defined SET_ALL_MARGINS_TO_ZERO
        box->setMargin(0);
#endif
        box->setObjectName( QString::fromLatin1( name ) );
        box->setSizeConstraint( QLayout::SetFixedSize );
    }
    return box;
}

#if 0
static QVBoxLayout* findOrCreateVBoxLayoutByObjectName( QLayout* parentLayout, const char* name )
{
    return findOrCreateLayoutByObjectName<QVBoxLayout>( parentLayout, name );
}

static QHBoxLayout* findOrCreateHBoxLayoutByObjectName( QLayout* parentLayout, const char* name )
{
    return findOrCreateLayoutByObjectName<QHBoxLayout>( parentLayout, name );
}
#endif

void Chart::Private::slotLayoutPlanes()
{
    //qDebug() << "KDChart::Chart is layouting the planes";
    const QBoxLayout::Direction oldPlanesDirection =
        planesLayout ? planesLayout->direction() : QBoxLayout::TopToBottom;
    if ( planesLayout && dataAndLegendLayout )
        dataAndLegendLayout->removeItem( planesLayout );
    
    const bool hadPlanesLayout = planesLayout != 0;
    int left, top, right, bottom;
    if(hadPlanesLayout)
        planesLayout->getContentsMargins(&left, &top, &right, &bottom);
        
    KDAB_FOREACH( KDChart::AbstractLayoutItem* plane, planeLayoutItems ) {
        plane->removeFromParentLayout();
    }
    planeLayoutItems.clear();
    delete planesLayout;
    //hint: The direction is configurable by the user now, as
    //      we are using a QBoxLayout rather than a QVBoxLayout.  (khz, 2007/04/25)
    planesLayout = new QBoxLayout( oldPlanesDirection );

    if(hadPlanesLayout)
        planesLayout->setContentsMargins(left, top, right, bottom);

    // TESTING(khz): set the margin of all of the layouts to Zero
#if defined SET_ALL_MARGINS_TO_ZERO
    planesLayout->setMargin(0);
    planesLayout->setSpacing(0);
#endif
    planesLayout->setObjectName( QString::fromLatin1( "planesLayout" ) );

    /* First go through all planes and all axes and figure out whether the planes
     * need to coordinate. If they do, they share a grid layout, if not, each
     * get their own. See buildPlaneLayoutInfos() for more details. */
    QHash<AbstractCoordinatePlane*, PlaneInfo> planeInfos = buildPlaneLayoutInfos();
    QHash<AbstractAxis*, AxisInfo> axisInfos;
    KDAB_FOREACH( AbstractCoordinatePlane* plane, coordinatePlanes ) {
        Q_ASSERT( planeInfos.contains(plane) );
        PlaneInfo& pi = planeInfos[ plane ];
        int column = pi.horizontalOffset;
        int row = pi.verticalOffset;
        //qDebug() << "processing plane at column" << column << "and row" << row;
        QGridLayout *planeLayout = pi.gridLayout;
        if ( !planeLayout ) {
            // this plane is sharing an axis with another one, so use
            // the grid of that one as well
            planeLayout = planeInfos[pi.referencePlane].gridLayout;
            Q_ASSERT( planeLayout );
        } else {
            planesLayout->addLayout( planeLayout );
        }

        /* Put the plane in the center of the layout. If this is our own, that's
         * the middle of the layout, if we are sharing, it's a cell in the center
         * column of the shared grid. */
        planeLayoutItems << plane;
        plane->setParentLayout( planeLayout );
        planeLayout->addItem( plane, row, column, 1, 1, 0 );
        //qDebug() << "Chart slotLayoutPlanes() calls planeLayout->addItem("<< row << column << ")";
        planeLayout->setRowStretch(    row,    2 );
        planeLayout->setColumnStretch( column, 2 );

        KDAB_FOREACH( AbstractDiagram* abstractDiagram, plane->diagrams() )
        {
            AbstractCartesianDiagram* diagram =
                dynamic_cast<AbstractCartesianDiagram*> ( abstractDiagram );
            //qDebug() << "--------------- diagram ???????????????????? -----------------";
            if( !diagram ) continue;  // FIXME polar ?
            //qDebug() << "--------------- diagram ! ! ! ! ! ! ! ! ! !  -----------------";

            if( pi.referencePlane != 0 )
            {
                pi.topAxesLayout = planeInfos[ pi.referencePlane ].topAxesLayout;
                pi.bottomAxesLayout = planeInfos[ pi.referencePlane ].bottomAxesLayout;
                pi.leftAxesLayout = planeInfos[ pi.referencePlane ].leftAxesLayout;
                pi.rightAxesLayout = planeInfos[ pi.referencePlane ].rightAxesLayout;
            }

            // collect all axes of a kind into sublayouts
            if( pi.topAxesLayout == 0 )
            {
                pi.topAxesLayout = new QVBoxLayout;
#if defined SET_ALL_MARGINS_TO_ZERO
                pi.topAxesLayout->setMargin(0);
#endif
                pi.topAxesLayout->setObjectName( QString::fromLatin1( "topAxesLayout" ) );
            }
            if( pi.bottomAxesLayout == 0 )
            {
                pi.bottomAxesLayout = new QVBoxLayout;
#if defined SET_ALL_MARGINS_TO_ZERO
                pi.bottomAxesLayout->setMargin(0);
#endif
                pi.bottomAxesLayout->setObjectName( QString::fromLatin1( "bottomAxesLayout" ) );
            }
            if( pi.leftAxesLayout == 0 )
            {
                pi.leftAxesLayout = new QHBoxLayout;
#if defined SET_ALL_MARGINS_TO_ZERO
                pi.leftAxesLayout->setMargin(0);
#endif
                pi.leftAxesLayout->setObjectName( QString::fromLatin1( "leftAxesLayout" ) );
            }
            if( pi.rightAxesLayout == 0 )
            {
                pi.rightAxesLayout = new QHBoxLayout;
#if defined SET_ALL_MARGINS_TO_ZERO
                pi.rightAxesLayout->setMargin(0);
#endif
                pi.rightAxesLayout->setObjectName( QString::fromLatin1( "rightAxesLayout" ) );
            }

            if( pi.referencePlane != 0 )
            {
                planeInfos[ pi.referencePlane ].topAxesLayout = pi.topAxesLayout;
                planeInfos[ pi.referencePlane ].bottomAxesLayout = pi.bottomAxesLayout;
                planeInfos[ pi.referencePlane ].leftAxesLayout = pi.leftAxesLayout;
                planeInfos[ pi.referencePlane ].rightAxesLayout = pi.rightAxesLayout;
            }
 
            //pi.leftAxesLayout->setSizeConstraint( QLayout::SetFixedSize );

            KDAB_FOREACH( CartesianAxis* axis, diagram->axes() ) {
                if ( axisInfos.contains( axis ) ) continue; // already laid this one out
                Q_ASSERT ( axis );
                axis->setCachedSizeDirty();
                //qDebug() << "--------------- axis added to planeLayoutItems  -----------------";
                planeLayoutItems << axis;
                /*
                // Unused code trying to use a push-model: This did not work
                // since we can not re-layout the planes each time when
                // Qt layouting is calling sizeHint()
                connect( axis, SIGNAL( needAdjustLeftRightColumnsForOverlappingLabels(
                CartesianAxis*, int, int ) ),
                this, SLOT( slotAdjustLeftRightColumnsForOverlappingLabels(
                CartesianAxis*, int, int ) ) );
                connect( axis, SIGNAL( needAdjustTopBottomRowsForOverlappingLabels(
                CartesianAxis*, int, int ) ),
                this, SLOT( slotAdjustTopBottomRowsForOverlappingLabels(
                CartesianAxis*, int, int ) ) );
                */
                switch ( axis->position() )
                {
                    case CartesianAxis::Top:
                        axis->setParentLayout( pi.topAxesLayout );
                        pi.topAxesLayout->addItem( axis );
                        break;
                    case CartesianAxis::Bottom:
                        axis->setParentLayout( pi.bottomAxesLayout );
                        pi.bottomAxesLayout->addItem( axis );
                        break;
                    case CartesianAxis::Left:
                        axis->setParentLayout( pi.leftAxesLayout );
                        pi.leftAxesLayout->addItem( axis );
                        break;
                    case CartesianAxis::Right:
                        axis->setParentLayout( pi.rightAxesLayout );
                        pi.rightAxesLayout->addItem( axis );
                        break;
                    default:
                        Q_ASSERT_X( false, "Chart::paintEvent",
                                "unknown axis position" );
                        break;
                };
                axisInfos.insert( axis, AxisInfo() );
            }
            /* Put each stack of axes-layouts in the cells surrounding the
             * associated plane. We are laying out in the oder the planes
             * were added, and the first one gets to lay out shared axes.
             * Private axes go here as well, of course. */
            if ( !pi.topAxesLayout->parent() )
                planeLayout->addLayout( pi.topAxesLayout,    row - 1, column );
            if ( !pi.bottomAxesLayout->parent() )
                planeLayout->addLayout( pi.bottomAxesLayout, row + 1, column );
            if ( !pi.leftAxesLayout->parent() ){
                planeLayout->addLayout( pi.leftAxesLayout,   row,     column - 1);
                //planeLayout->setRowStretch(    row, 0 );
                //planeLayout->setColumnStretch( 0,   0 );
            }
            if ( !pi.rightAxesLayout->parent() )
                planeLayout->addLayout( pi.rightAxesLayout,  row,     column + 1);
        }

        // use up to four auto-spacer items in the corners around the diagrams:
#define ADD_AUTO_SPACER_IF_NEEDED( \
        spacerRow, spacerColumn, hLayoutIsAtTop, hLayout, vLayoutIsAtLeft, vLayout ) \
        { \
            if( hLayout || vLayout ) { \
                AutoSpacerLayoutItem * spacer \
                = new AutoSpacerLayoutItem( hLayoutIsAtTop, hLayout, vLayoutIsAtLeft, vLayout ); \
                planeLayout->addItem( spacer, spacerRow, spacerColumn, 1, 1 ); \
                spacer->setParentLayout( planeLayout ); \
                planeLayoutItems << spacer; \
            } \
        }
        ADD_AUTO_SPACER_IF_NEEDED( row-1, column-1, false, pi.leftAxesLayout,  false, pi.topAxesLayout )
            ADD_AUTO_SPACER_IF_NEEDED( row+1, column-1, true,  pi.leftAxesLayout,  false,  pi.bottomAxesLayout )
            ADD_AUTO_SPACER_IF_NEEDED( row-1, column+1, false, pi.rightAxesLayout, true, pi.topAxesLayout )
            ADD_AUTO_SPACER_IF_NEEDED( row+1, column+1, true,  pi.rightAxesLayout, true,  pi.bottomAxesLayout )
    }
    // re-add our grid(s) to the chart's layout
    if ( dataAndLegendLayout ){
        dataAndLegendLayout->addLayout( planesLayout, 1, 1 );
        dataAndLegendLayout->setRowStretch(    1, 1000 );
        dataAndLegendLayout->setColumnStretch( 1, 1000 );
    }

    slotRelayout();
    //qDebug() << "KDChart::Chart finished layouting the planes.";
}

void Chart::Private::createLayouts( QWidget* w )
{
    KDAB_FOREACH( KDChart::TextArea* textLayoutItem, textLayoutItems ) {
        textLayoutItem->removeFromParentLayout();
    }
    textLayoutItems.clear();

    KDAB_FOREACH( KDChart::AbstractArea* layoutItem, layoutItems ) {
        layoutItem->removeFromParentLayout();
    }
    layoutItems.clear();

    removeDummyHeaderFooters();

    // layout for the planes is handled separately, so we don't want to delete it here
    if ( dataAndLegendLayout) {
        dataAndLegendLayout->removeItem( planesLayout );
        planesLayout->setParent( 0 );
    }
    // nuke the old bunch
    delete layout;

    // The HBox d->layout provides the left and right global leadings
    layout = new QHBoxLayout( w );
    // TESTING(khz): set the margin of all of the layouts to Zero
#if defined SET_ALL_MARGINS_TO_ZERO
    layout->setMargin(0);
#endif
    layout->setObjectName( QString::fromLatin1( "Chart::Private::layout" ) );
    layout->addSpacing( globalLeadingLeft );

    // The vLayout provides top and bottom global leadings and lays
    // out headers/footers and the data area.
    vLayout = new QVBoxLayout();
    // TESTING(khz): set the margin of all of the layouts to Zero
#if defined SET_ALL_MARGINS_TO_ZERO
    vLayout->setMargin(0);
#endif
    vLayout->setObjectName( QString::fromLatin1( "vLayout" ) );
    layout->addLayout( vLayout, 1000 );
    layout->addSpacing( globalLeadingRight );



    // 1. the gap above the top edge of the headers area
    vLayout->addSpacing( globalLeadingTop );
    // 2. the header(s) area
    headerLayout = new QGridLayout();
    // TESTING(khz): set the margin of all of the layouts to Zero
#if defined SET_ALL_MARGINS_TO_ZERO
    headerLayout->setMargin(0);
#endif
    vLayout->addLayout( headerLayout );
    // 3. the area containing coordinate plane(s), axes, legend(s)
    dataAndLegendLayout = new QGridLayout();
    // TESTING(khz): set the margin of all of the layouts to Zero
#if defined SET_ALL_MARGINS_TO_ZERO
    dataAndLegendLayout->setMargin(0);
#endif
    dataAndLegendLayout->setObjectName( QString::fromLatin1( "dataAndLegendLayout" ) );
    vLayout->addLayout( dataAndLegendLayout, 1000 );
    // 4. the footer(s) area
    footerLayout = new QGridLayout();
    // TESTING(khz): set the margin of all of the layouts to Zero
#if defined SET_ALL_MARGINS_TO_ZERO
    footerLayout->setMargin(0);
#endif
    footerLayout->setObjectName( QString::fromLatin1( "footerLayout" ) );
    vLayout->addLayout( footerLayout );

    // 5. Prepare the header / footer layout cells:
    //    Each of the 9 header cells (the 9 footer cells)
    //    contain their own QVBoxLayout
    //    since there can be more than one header (footer) per cell.
    static const Qt::Alignment hdFtAlignments[3][3] = {
        { Qt::AlignTop     | Qt::AlignLeft,  Qt::AlignTop     | Qt::AlignHCenter,  Qt::AlignTop     | Qt::AlignRight },
        { Qt::AlignVCenter | Qt::AlignLeft,  Qt::AlignVCenter | Qt::AlignHCenter,  Qt::AlignVCenter | Qt::AlignRight },
        { Qt::AlignBottom  | Qt::AlignLeft,  Qt::AlignBottom  | Qt::AlignHCenter,  Qt::AlignBottom  | Qt::AlignRight }
    };
    for ( int row = 0; row < 3; ++row )
    {
        for ( int column = 0; column < 3; ++ column )
        {
            QVBoxLayout* innerHdLayout = new QVBoxLayout();
            QVBoxLayout* innerFtLayout = new QVBoxLayout();
            innerHdFtLayouts[0][row][column] = innerHdLayout;
            innerHdFtLayouts[1][row][column] = innerFtLayout;
#if defined SET_ALL_MARGINS_TO_ZERO
            innerHdLayout->setMargin(0);
            innerFtLayout->setMargin(0);
#endif
            const Qt::Alignment align = hdFtAlignments[row][column];
            innerHdLayout->setAlignment( align );
            innerFtLayout->setAlignment( align );
            headerLayout->addLayout( innerHdLayout, row, column, align );
            footerLayout->addLayout( innerFtLayout, row, column, align );
        }
    }

    // 6. the gap below the bottom edge of the headers area
    vLayout->addSpacing( globalLeadingBottom );

    // the data+axes area
    dataAndLegendLayout->addLayout( planesLayout, 1, 1 );
    dataAndLegendLayout->setRowStretch(    1, 1 );
    dataAndLegendLayout->setColumnStretch( 1, 1 );

    //qDebug() << "w->rect()" << w->rect();
}

void Chart::Private::slotRelayout()
{
    //qDebug() << "Chart relayouting started.";
    createLayouts( chart );

    layoutHeadersAndFooters();
    layoutLegends();

    // This triggers the qlayout, see QBoxLayout::setGeometry
    // The geometry is not necessarily w->rect(), when using paint(), this is why
    // we don't call layout->activate().
    const QRect geo( QRect( 0, 0, currentLayoutSize.width(), currentLayoutSize.height() ) );
    if( geo.isValid() && geo != layout->geometry() ){
        //qDebug() << "Chart slotRelayout() adjusting geometry to" << geo;
        //if( coordinatePlanes.count() )
        //    qDebug() << "           plane geo before" << coordinatePlanes.first()->geometry();
        layout->setGeometry( geo );
        //if( coordinatePlanes.count() ) {
        //    qDebug() << "           plane geo after " << coordinatePlanes.first()->geometry();
        //}
    }

    // Adapt diagram drawing to the new size
    KDAB_FOREACH (AbstractCoordinatePlane* plane, coordinatePlanes ) {
        plane->layoutDiagrams();
    }
    //qDebug() << "Chart relayouting done.";
}

// Called when the size of the chart changes.
// So in theory, we only need to adjust geometries.
// But this also needs to make sure that everything is in place for the first painting.
void Chart::Private::resizeLayout( const QSize& size )
{
    currentLayoutSize = size;
    //qDebug() << "Chart::resizeLayout(" << currentLayoutSize << ")";

    /*
    // We need to make sure that the legend's layouts are populated,
    // so that setGeometry gets proper sizeHints from them and resizes them properly.
    KDAB_FOREACH( Legend *legend, legends ) {
    // This forceRebuild will see a wrong areaGeometry, but I don't care about geometries yet,
    // only about the fact that legends should have their contents populated.
    // -> it would be better to dissociate "building contents" and "resizing" in Legend...

    //        legend->forceRebuild();

    legend->resizeLayout( size );
    }
    */
    slotLayoutPlanes(); // includes slotRelayout

    //qDebug() << "Chart::resizeLayout done";
}


void Chart::Private::paintAll( QPainter* painter )
{
    QRect rect( QPoint(0, 0), currentLayoutSize );

    //qDebug() << this<<"::paintAll() uses layout size" << currentLayoutSize;

    // Paint the background (if any)
    KDChart::AbstractAreaBase::paintBackgroundAttributes(
            *painter, rect, backgroundAttributes );
    // Paint the frame (if any)
    KDChart::AbstractAreaBase::paintFrameAttributes(
            *painter, rect, frameAttributes );

    chart->reLayoutFloatingLegends();

    KDAB_FOREACH( KDChart::AbstractArea* layoutItem, layoutItems ) {
        layoutItem->paintAll( *painter );
    }
    KDAB_FOREACH( KDChart::AbstractLayoutItem* planeLayoutItem, planeLayoutItems ) {
        planeLayoutItem->paintAll( *painter );
    }
    KDAB_FOREACH( KDChart::TextArea* textLayoutItem, textLayoutItems ) {
        textLayoutItem->paintAll( *painter );
    }
}

// ******** Chart interface implementation ***********

Chart::Chart ( QWidget* parent )
    : QWidget ( parent )
    , _d( new Private( this ) )
{
#if defined KDAB_EVAL
    EvalDialog::checkEvalLicense( "KD Chart" );
#endif

    FrameAttributes frameAttrs;
// no frame per default...
//    frameAttrs.setVisible( true );
    frameAttrs.setPen( QPen( Qt::black ) );
    frameAttrs.setPadding( 1 );
    setFrameAttributes( frameAttrs );

    addCoordinatePlane( new CartesianCoordinatePlane ( this ) );
}

Chart::~Chart()
{
    delete _d;
}

#define d d_func()

void Chart::setFrameAttributes( const FrameAttributes &a )
{
    d->frameAttributes = a;
}

FrameAttributes Chart::frameAttributes() const
{
    return d->frameAttributes;
}

void Chart::setBackgroundAttributes( const BackgroundAttributes &a )
{
    d->backgroundAttributes = a;
}

BackgroundAttributes Chart::backgroundAttributes() const
{
    return d->backgroundAttributes;
}

//TODO KDChart 3.0; change QLayout into QBoxLayout::Direction
void Chart::setCoordinatePlaneLayout( QLayout * layout )
{
    delete d->planesLayout;
    d->planesLayout = dynamic_cast<QBoxLayout*>( layout );
    d->slotLayoutPlanes();
}

QLayout* Chart::coordinatePlaneLayout()
{
    return d->planesLayout;
}

AbstractCoordinatePlane* Chart::coordinatePlane()
{
    if ( d->coordinatePlanes.isEmpty() )
    {
        qWarning() << "Chart::coordinatePlane: warning: no coordinate plane defined.";
        return 0;
    } else {
        return d->coordinatePlanes.first();
    }
}

CoordinatePlaneList Chart::coordinatePlanes()
{
    return d->coordinatePlanes;
}

void Chart::addCoordinatePlane( AbstractCoordinatePlane* plane )
{
    connect( plane, SIGNAL( destroyedCoordinatePlane( AbstractCoordinatePlane* ) ),
             d,   SLOT( slotUnregisterDestroyedPlane( AbstractCoordinatePlane* ) ) );
    connect( plane, SIGNAL( needUpdate() ),       this,   SLOT( update() ) );
    connect( plane, SIGNAL( needRelayout() ),     d,      SLOT( slotRelayout() ) ) ;
    connect( plane, SIGNAL( needLayoutPlanes() ), d,      SLOT( slotLayoutPlanes() ) ) ;
    connect( plane, SIGNAL( propertiesChanged() ),this, SIGNAL( propertiesChanged() ) );
    d->coordinatePlanes.append( plane );
    plane->setParent( this );
    d->slotLayoutPlanes();
}

void Chart::replaceCoordinatePlane( AbstractCoordinatePlane* plane,
                                    AbstractCoordinatePlane* oldPlane_ )
{
    if( plane && oldPlane_ != plane ){
        AbstractCoordinatePlane* oldPlane = oldPlane_;
        if( d->coordinatePlanes.count() ){
            if( ! oldPlane ){
                oldPlane = d->coordinatePlanes.first();
                if( oldPlane == plane )
                    return;
            }
            takeCoordinatePlane( oldPlane );
        }
        delete oldPlane;
        addCoordinatePlane( plane );
    }
}

void Chart::takeCoordinatePlane( AbstractCoordinatePlane* plane )
{
    const int idx = d->coordinatePlanes.indexOf( plane );
    if( idx != -1 ){
        d->coordinatePlanes.takeAt( idx );
        disconnect( plane, SIGNAL( destroyedCoordinatePlane( AbstractCoordinatePlane* ) ),
                    d, SLOT( slotUnregisterDestroyedPlane( AbstractCoordinatePlane* ) ) );
        plane->removeFromParentLayout();
        plane->setParent( 0 );
    }
    d->slotLayoutPlanes();
    // Need to emit the signal: In case somebody has connected the signal
    // to her own slot for e.g. calling update() on a widget containing the chart.
    emit propertiesChanged();
}

void Chart::setGlobalLeading( int left, int top, int right, int bottom )
{
    setGlobalLeadingLeft( left );
    setGlobalLeadingTop( top );
    setGlobalLeadingRight( right );
    setGlobalLeadingBottom( bottom );
    d->slotRelayout();
}

void Chart::setGlobalLeadingLeft( int leading )
{
    d->globalLeadingLeft = leading;
    d->slotRelayout();
}

int Chart::globalLeadingLeft() const
{
    return d->globalLeadingLeft;
}

void Chart::setGlobalLeadingTop( int leading )
{
    d->globalLeadingTop = leading;
    d->slotRelayout();
}

int Chart::globalLeadingTop() const
{
    return d->globalLeadingTop;
}

void Chart::setGlobalLeadingRight( int leading )
{
    d->globalLeadingRight = leading;
    d->slotRelayout();
}

int Chart::globalLeadingRight() const
{
    return d->globalLeadingRight;
}

void Chart::setGlobalLeadingBottom( int leading )
{
    d->globalLeadingBottom = leading;
    d->slotRelayout();
}

int Chart::globalLeadingBottom() const
{
    return d->globalLeadingBottom;
}

void Chart::paint( QPainter* painter, const QRect& target )
{
    if( target.isEmpty() || !painter ) return;
    //qDebug() << "Chart::paint( ..," << target << ")";

    QPaintDevice* prevDevice = GlobalMeasureScaling::paintDevice();
    GlobalMeasureScaling::setPaintDevice( painter->device() );

    // Output on a widget
    if( dynamic_cast< QWidget* >( painter->device() ) != 0 )
    {
        GlobalMeasureScaling::setFactors(
                static_cast< qreal >( target.width() ) /
                static_cast< qreal >( geometry().size().width() ),
                static_cast< qreal >( target.height() ) /
                static_cast< qreal >( geometry().size().height() ) );
    }
    // Output onto a QPixmap 
    else
    {
        PrintingParameters::setScaleFactor( static_cast< qreal >( painter->device()->logicalDpiX() ) / static_cast< qreal >( logicalDpiX() ) );

        const qreal resX = static_cast< qreal >( logicalDpiX() ) / static_cast< qreal >( painter->device()->logicalDpiX() );
        const qreal resY = static_cast< qreal >( logicalDpiY() ) / static_cast< qreal >( painter->device()->logicalDpiY() );

        GlobalMeasureScaling::setFactors(
                static_cast< qreal >( target.width() ) /
                static_cast< qreal >( geometry().size().width() ) * resX,
                static_cast< qreal >( target.height() ) /
                static_cast< qreal >( geometry().size().height() ) * resY );
    }


    if( target.size() != d->currentLayoutSize ){
        d->resizeLayout( target.size() );
    }
    const QPoint translation = target.topLeft();
    painter->translate( translation );

    d->paintAll( painter );

    // for debugging:
    //painter->setPen(QPen(Qt::blue, 8));
    //painter->drawRect(target.adjusted(12,12,-12,-12));

    KDAB_FOREACH( Legend *legend, d->legends ) {
        const bool hidden = legend->isHidden() && legend->testAttribute(Qt::WA_WState_ExplicitShowHide);
        if ( !hidden ) {
            //qDebug() << "painting legend at " << legend->geometry();
            legend->paintIntoRect( *painter, legend->geometry() );
            //testing:
            //legend->paintIntoRect( *painter, legend->geometry().adjusted(-100,0,-100,0) );
        }
    }

    painter->translate( -translation.x(), -translation.y() );

    GlobalMeasureScaling::instance()->resetFactors();
    PrintingParameters::resetScaleFactor();
    GlobalMeasureScaling::setPaintDevice( prevDevice );

    //qDebug() << "KDChart::Chart::paint() done.\n";
}

void Chart::resizeEvent ( QResizeEvent * )
{
    d->resizeLayout( size() );
    KDAB_FOREACH( AbstractCoordinatePlane* plane, d->coordinatePlanes ){
        plane->setGridNeedsRecalculate();
    }
    reLayoutFloatingLegends();
}


void Chart::reLayoutFloatingLegends()
{
    KDAB_FOREACH( Legend *legend, d->legends ) {
        const bool hidden = legend->isHidden() && legend->testAttribute(Qt::WA_WState_ExplicitShowHide);
        if ( legend->position().isFloating() && !hidden ){
            // resize the legend
            const QSize legendSize( legend->sizeHint() );
            legend->setGeometry( QRect( legend->geometry().topLeft(), legendSize ) );
            // find the legends corner point (reference point plus any paddings)
            const RelativePosition relPos( legend->floatingPosition() );
            QPointF pt( relPos.calculatedPoint( size() ) );
            //qDebug() << pt;
            // calculate the legend's top left point
            const Qt::Alignment alignTopLeft = Qt::AlignBottom | Qt::AlignLeft;
            if( (relPos.alignment() & alignTopLeft) != alignTopLeft ){
                if( relPos.alignment() & Qt::AlignRight )
                    pt.rx() -= legendSize.width();
                else if( relPos.alignment() & Qt::AlignHCenter )
                    pt.rx() -= 0.5 * legendSize.width();

                if( relPos.alignment() & Qt::AlignBottom )
                    pt.ry() -= legendSize.height();
                else if( relPos.alignment() & Qt::AlignVCenter )
                    pt.ry() -= 0.5 * legendSize.height();
            }
            //qDebug() << pt << endl;
            legend->move( static_cast<int>(pt.x()), static_cast<int>(pt.y()) );
        }
    }
}


void Chart::paintEvent( QPaintEvent* )
{
    QPainter painter( this );

    if( size() != d->currentLayoutSize ){
        d->resizeLayout( size() );
        reLayoutFloatingLegends();
    }

    //FIXME(khz): Paint the background/frame too!
    //            (can we derive Chart from AreaWidget ??)
    d->paintAll( &painter );
}

void Chart::addHeaderFooter( HeaderFooter* headerFooter )
{
    d->headerFooters.append( headerFooter );
    headerFooter->setParent( this );
    connect( headerFooter, SIGNAL( destroyedHeaderFooter( HeaderFooter* ) ),
             d, SLOT( slotUnregisterDestroyedHeaderFooter( HeaderFooter* ) ) );
    connect( headerFooter, SIGNAL( positionChanged( HeaderFooter* ) ),
             d, SLOT( slotRelayout() ) );
    d->slotRelayout();
}

void Chart::replaceHeaderFooter( HeaderFooter* headerFooter,
                                 HeaderFooter* oldHeaderFooter_ )
{
    if( headerFooter && oldHeaderFooter_ != headerFooter ){
        HeaderFooter* oldHeaderFooter = oldHeaderFooter_;
        if( d->headerFooters.count() ){
            if( ! oldHeaderFooter ){
                oldHeaderFooter =  d->headerFooters.first();
                if( oldHeaderFooter == headerFooter )
                    return;
            }
            takeHeaderFooter( oldHeaderFooter );
        }
        delete oldHeaderFooter;
        addHeaderFooter( headerFooter );
    }
}

void Chart::takeHeaderFooter( HeaderFooter* headerFooter )
{
    const int idx = d->headerFooters.indexOf( headerFooter );
    if( idx != -1 ){
        d->headerFooters.takeAt( idx );
        disconnect( headerFooter, SIGNAL( destroyedHeaderFooter( HeaderFooter* ) ),
                    d, SLOT( slotUnregisterDestroyedHeaderFooter( HeaderFooter* ) ) );
        headerFooter->setParent( 0 );
    }
    d->slotRelayout();
    // Need to emit the signal: In case somebody has connected the signal
    // to her own slot for e.g. calling update() on a widget containing the chart.
    emit propertiesChanged();
}

HeaderFooter* Chart::headerFooter()
{
    if( d->headerFooters.isEmpty() ) {
        return 0;
    } else {
        return d->headerFooters.first();
    }
}

HeaderFooterList Chart::headerFooters()
{
    return d->headerFooters;
}

void Chart::addLegend( Legend* legend )
{
    if( ! legend ) return;

    //qDebug() << "adding the legend";
    d->legends.append( legend );
    legend->setParent( this );

    TextAttributes textAttrs( legend->textAttributes() );

    KDChart::Measure measure( textAttrs.fontSize() );
    measure.setRelativeMode( this, KDChartEnums::MeasureOrientationMinimum );
    measure.setValue( 20 );
    textAttrs.setFontSize( measure );
    legend->setTextAttributes( textAttrs );

    textAttrs = legend->titleTextAttributes();
    measure.setRelativeMode( this, KDChartEnums::MeasureOrientationMinimum );
    measure.setValue( 24 );
    textAttrs.setFontSize( measure );

    legend->setTitleTextAttributes( textAttrs );

    legend->setReferenceArea( this );

/*
    future: Use relative sizes for the markers too!

    const uint nMA = Legend::datasetCount();
    for( uint iMA = 0; iMA < nMA; ++iMA ){
        MarkerAttributes ma( legend->markerAttributes( iMA ) );
        ma.setMarkerSize( ... )
        legend->setMarkerAttributes( iMA, ma )
    }
*/

    connect( legend, SIGNAL( destroyedLegend( Legend* ) ),
             d, SLOT( slotUnregisterDestroyedLegend( Legend* ) ) );
    connect( legend, SIGNAL( positionChanged( AbstractAreaWidget* ) ),
             d, SLOT( slotLayoutPlanes() ) ); //slotRelayout() ) );
    connect( legend, SIGNAL( propertiesChanged() ),
             this,   SIGNAL( propertiesChanged() ) );
    legend->setVisible( true );
    d->slotRelayout();
}


void Chart::replaceLegend( Legend* legend, Legend* oldLegend_ )
{
    if( legend && oldLegend_ != legend ){
        Legend* oldLegend = oldLegend_;
        if( d->legends.count() ){
            if( ! oldLegend ){
                oldLegend = d->legends.first();
                if( oldLegend == legend )
                    return;
            }
            takeLegend( oldLegend );
        }
        delete oldLegend;
        addLegend( legend );
    }
}

void Chart::takeLegend( Legend* legend )
{
    const int idx = d->legends.indexOf( legend );
    if( idx != -1 ){
        d->legends.takeAt( idx );
        disconnect( legend, SIGNAL( destroyedLegend( Legend* ) ),
                    d, SLOT( slotUnregisterDestroyedLegend( Legend* ) ) );
        disconnect( legend, SIGNAL( positionChanged( AbstractAreaWidget* ) ),
                    d, SLOT( slotLayoutPlanes() ) ); //slotRelayout() ) );
        disconnect( legend, SIGNAL( propertiesChanged() ),
                    this,   SIGNAL( propertiesChanged() ) );
        legend->setParent( 0 );
        legend->setVisible( false );
    }
    d->slotRelayout();

    // Need to emit the signal: In case somebody has connected the signal
    // to her own slot for e.g. calling update() on a widget containing the chart.
    // Note:
    // We do this ourselves in examples/DrawIntoPainter/mainwindow.cpp
    emit propertiesChanged();
}

Legend* Chart::legend()
{
    if ( d->legends.isEmpty() )
    {
        return 0;
    } else {
        return d->legends.first();
    }
}

LegendList Chart::legends()
{
    return d->legends;
}


void Chart::mousePressEvent( QMouseEvent* event )
{
    const QPoint pos = mapFromGlobal( event->globalPos() );

    KDAB_FOREACH( AbstractCoordinatePlane* plane, d->coordinatePlanes )
    {
        if ( plane->geometry().contains( event->pos() ) )
        {
            if ( plane->diagrams().size() > 0 )
            {
                QMouseEvent ev( QEvent::MouseButtonPress, pos, event->globalPos(),
                                event->button(), event->buttons(),
                                event->modifiers() );

                plane->mousePressEvent( &ev );
                d->mouseClickedPlanes.append( plane );
           }
       }
    }
}

/*
// Unused code trying to use a push-model: This did not work
// since we can not re-layout the planes each time when
// Qt layouting is calling sizeHint()
void Chart::Private::slotAdjustLeftRightColumnsForOverlappingLabels(
        CartesianAxis* axis, int leftOverlap, int rightOverlap)
{
    const QLayout* axisLayout = axis ? axis->parentLayout() : 0;

    if( (! leftOverlap && ! rightOverlap) || ! axis || ! axisLayout->parent() )
        return;

    bool needUpdate = false;
    // access the planeLayout:
    QGridLayout* grid = qobject_cast<QGridLayout*>(axisLayout->parent());
    if( grid ){
        // find the index of the parent layout in the planeLayout:
        int idx = -1;
        for (int i = 0; i < grid->count(); ++i)
            if( grid->itemAt(i) == axisLayout )
                idx = i;
        // set the min widths of the neighboring column:
        if( idx > -1 ){
            int row, column, rowSpan, columnSpan;
            grid->getItemPosition( idx, &row, &column, &rowSpan, &columnSpan );
            const int leftColumn = column-1;
            const int rightColumn = column+columnSpan;
            // find the left/right axes layouts
            QHBoxLayout* leftAxesLayout=0;
            QHBoxLayout* rightAxesLayout=0;
            for( int i = 0;
                 (!leftAxesLayout || !rightAxesLayout) && i < grid->count();
                 ++i )
            {
                int r, c, rs, cs;
                grid->getItemPosition( i, &r, &c, &rs, &cs );
                if( c+cs-1 == leftColumn )
                    leftAxesLayout = dynamic_cast<QHBoxLayout*>(grid->itemAt(i));
                if( c == rightColumn )
                    rightAxesLayout = dynamic_cast<QHBoxLayout*>(grid->itemAt(i));
            }
            if( leftAxesLayout ){
                const int leftColumnMinWidth = leftOverlap;
                QLayoutItem* item = leftAxesLayout->count()
                        ? dynamic_cast<QLayoutItem*>(leftAxesLayout->itemAt(leftAxesLayout->count()-1))
                    : 0;
                QSpacerItem* spacer = dynamic_cast<QSpacerItem*>(item);
                if( spacer ){
                    if( spacer->sizeHint().width() < leftColumnMinWidth ){
                        needUpdate = true;
                        spacer->changeSize(leftColumnMinWidth, 1);
                        qDebug() << "adjusted left spacer->sizeHint().width() to" << spacer->sizeHint().width();
                    }
                }else{
                    AbstractAxis* axis = dynamic_cast<AbstractAxis*>(item);
                    if( !axis || axis->sizeHint().width() < leftColumnMinWidth ){
                        needUpdate = true;
                        leftAxesLayout->insertSpacing( -1, leftColumnMinWidth );
                        qDebug() << "adjusted column" << leftColumn << "min width to" << leftColumnMinWidth;
                    }
                }
            }
            if( rightAxesLayout ){
                const int rightColumnMinWidth = rightOverlap;
                QLayoutItem* item = rightAxesLayout->count()
                        ? dynamic_cast<QLayoutItem*>(rightAxesLayout->itemAt(0))
                    : 0;
                QSpacerItem* spacer = dynamic_cast<QSpacerItem*>(item);
                if( spacer ){
                    if( spacer->sizeHint().width() < rightColumnMinWidth ){
                        needUpdate = true;
                        spacer->changeSize(rightColumnMinWidth, 1);
                        qDebug() << "adjusted right spacer->sizeHint().width() to" << spacer->sizeHint().width();
                    }
                }else{
                    AbstractAxis* axis = dynamic_cast<AbstractAxis*>(item);
                    if( !axis || axis->sizeHint().width() < rightColumnMinWidth ){
                        needUpdate = true;
                        rightAxesLayout->insertSpacing( 0, rightColumnMinWidth );
                        qDebug() << "adjusted column" << rightColumn << "min width to" << rightColumnMinWidth;
                    }
                }
            }
        }
    }
    if( needUpdate ){
        ;// do something ...?
    }
}


void Chart::Private::slotAdjustTopBottomRowsForOverlappingLabels(
        CartesianAxis* axis, int topOverlap, int bottomOverlap)
{
    const QLayout* axisLayout = axis ? axis->parentLayout() : 0;

    if( (! topOverlap && ! bottomOverlap) || ! axisLayout || ! axisLayout->parent() )
        return;

    // access the planeLayout:
    QGridLayout* grid = qobject_cast<QGridLayout*>(axisLayout->parent());
    if( grid ){
            // find the index of the parent layout in the planeLayout:
        int idx = -1;
        for (int i = 0; i < grid->count(); ++i)
            if( grid->itemAt(i) == axisLayout )
                idx = i;
            // set the min widths of the neighboring column:
        if( idx > -1 ){
            int row, column, rowSpan, columnSpan;
            grid->getItemPosition( idx, &row, &column, &rowSpan, &columnSpan );
            const int topRow = row-1;
            const int bottomRow = row+rowSpan;
                // find the left/right axes layouts
            QVBoxLayout* topAxesLayout=0;
            QVBoxLayout* bottomAxesLayout=0;
            for( int i = 0;
                 (!topAxesLayout || !bottomAxesLayout) && i < grid->count();
                 ++i )
            {
                int r, c, rs, cs;
                grid->getItemPosition( i, &r, &c, &rs, &cs );
                if( r+rs-1 == topRow )
                    topAxesLayout = dynamic_cast<QVBoxLayout*>(grid->itemAt(i));
                if( r == bottomRow )
                    bottomAxesLayout = dynamic_cast<QVBoxLayout*>(grid->itemAt(i));
            }
            if( topAxesLayout ){
                const int topRowMinWidth = topOverlap;
                QLayoutItem* item = topAxesLayout->count()
                        ? dynamic_cast<QLayoutItem*>(topAxesLayout->itemAt(topAxesLayout->count()-1))
                    : 0;
                QSpacerItem* spacer = dynamic_cast<QSpacerItem*>(item);
                if( spacer ){
                    if( spacer->sizeHint().height() < topRowMinWidth ){
                        spacer->changeSize(1, topRowMinWidth);
                        qDebug() << "adjusted top spacer->sizeHint().height() to" << spacer->sizeHint().height();
                    }
                }else{
                    AbstractAxis* axis = dynamic_cast<AbstractAxis*>(item);
                    if( !axis || axis->sizeHint().height() < topRowMinWidth ){
                        topAxesLayout->insertSpacing( -1, topRowMinWidth );
                        qDebug() << "adjusted row" << topRow << "min height to" << topRowMinWidth;
                    }
                }
            }
            if( bottomAxesLayout ){
                const int bottomRowMinWidth = bottomOverlap;
                QLayoutItem* item = bottomAxesLayout->count()
                        ? dynamic_cast<QLayoutItem*>(bottomAxesLayout->itemAt(0))
                    : 0;
                QSpacerItem* spacer = dynamic_cast<QSpacerItem*>(item);
                if( spacer ){
                    if( spacer->sizeHint().height() < bottomRowMinWidth ){
                        spacer->changeSize(1, bottomRowMinWidth);
                        qDebug() << "adjusted bottom spacer->sizeHint().height() to" << spacer->sizeHint().height();
                    }
                }else{
                    AbstractAxis* axis = dynamic_cast<AbstractAxis*>(item);
                    if( !axis || axis->sizeHint().height() < bottomRowMinWidth ){
                        bottomAxesLayout->insertSpacing( 0, bottomRowMinWidth );
                        qDebug() << "adjusted row" << bottomRow << "min height to" << bottomRowMinWidth;
                    }
                }
            }
        }
    }
}
*/

void Chart::mouseDoubleClickEvent( QMouseEvent* event )
{
    const QPoint pos = mapFromGlobal( event->globalPos() );

    KDAB_FOREACH( AbstractCoordinatePlane* plane, d->coordinatePlanes )
    {
        if ( plane->geometry().contains( event->pos() ) )
        {
            if ( plane->diagrams().size() > 0 )
            {
                QMouseEvent ev( QEvent::MouseButtonPress, pos, event->globalPos(),
                                event->button(), event->buttons(),
                                event->modifiers() );
                plane->mouseDoubleClickEvent( &ev );
            }
        }
    }
}

void Chart::mouseMoveEvent( QMouseEvent* event )
{
    QSet< AbstractCoordinatePlane* > eventReceivers = QSet< AbstractCoordinatePlane* >::fromList( d->mouseClickedPlanes );

    KDAB_FOREACH( AbstractCoordinatePlane* plane, d->coordinatePlanes )
    {
        if( plane->geometry().contains( event->pos() ) )
        {
            if( plane->diagrams().size() > 0 )
            {
                eventReceivers.insert( plane );
            }
        }
    }

    const QPoint pos = mapFromGlobal( event->globalPos() );

    KDAB_FOREACH( AbstractCoordinatePlane* plane, eventReceivers )
    {
        QMouseEvent ev( QEvent::MouseMove, pos, event->globalPos(),
                         event->button(), event->buttons(),
                         event->modifiers() );
        plane->mouseMoveEvent( &ev );
    }
}

void Chart::mouseReleaseEvent( QMouseEvent* event )
{
    QSet< AbstractCoordinatePlane* > eventReceivers = QSet< AbstractCoordinatePlane* >::fromList( d->mouseClickedPlanes );

    KDAB_FOREACH( AbstractCoordinatePlane* plane, d->coordinatePlanes )
    {
        if ( plane->geometry().contains( event->pos() ) )
        {
            if( plane->diagrams().size() > 0 )
            {
                eventReceivers.insert( plane );
            }
        }
    }

    const QPoint pos = mapFromGlobal( event->globalPos() );

    KDAB_FOREACH( AbstractCoordinatePlane* plane, eventReceivers )
    {
        QMouseEvent ev( QEvent::MouseButtonRelease, pos, event->globalPos(),
                         event->button(), event->buttons(),
                         event->modifiers() );
        plane->mouseReleaseEvent( &ev );
    }

    d->mouseClickedPlanes.clear();
}

bool Chart::event( QEvent* event )
{
    switch( event->type() )
    {
    case QEvent::ToolTip:
    {
        const QHelpEvent* const helpEvent = static_cast< QHelpEvent* >( event );
        KDAB_FOREACH( const AbstractCoordinatePlane* const plane, d->coordinatePlanes )
        {
            for (int i = plane->diagrams().count() - 1; i >= 0; --i) {
                const QModelIndex index = plane->diagrams().at(i)->indexAt( helpEvent->pos() );
                const QVariant toolTip = index.data( Qt::ToolTipRole );
                if( toolTip.isValid() )
                {
                    QPoint pos = mapFromGlobal(helpEvent->pos());
                    QRect rect(pos-QPoint(1,1), QSize(3,3));
                    QToolTip::showText( QCursor::pos(), toolTip.toString(), this, rect );
                    return true;
                }
            }
        }
        // fall-through intended
    }
    default:
        return QWidget::event( event );
    }
}
