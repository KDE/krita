/* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2001-2007 Klarälvdalens Datakonsult AB.  All rights reserved.
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

#include <KDChartWidget.h>
#include <KDChartWidget_p.h>

#include <KDChartAbstractDiagram.h>
#include <KDChartBarDiagram.h>
#include <KDChartChart.h>
#include <KDChartAbstractCoordinatePlane.h>
#include <KDChartLineDiagram.h>
#include <KDChartPlotter.h>
#include <KDChartPieDiagram.h>
#include <KDChartPolarDiagram.h>
#include <KDChartRingDiagram.h>
#include <KDChartLegend.h>

#include <QDebug>

#include <KDABLibFakes>

#define d d_func()

using namespace KDChart;

Widget::Private::Private( Widget * qq )
    : q( qq ),
      layout( q ),
      m_model( q ),
      m_chart( q ),
      m_cartPlane( &m_chart ),
      m_polPlane( &m_chart ),
      usedDatasetWidth( 0 )
{
    KDAB_SET_OBJECT_NAME( layout );
    KDAB_SET_OBJECT_NAME( m_model );
    KDAB_SET_OBJECT_NAME( m_chart );

    layout.addWidget( &m_chart );
}

Widget::Private::~Private() {}


/**
* \class Widget KDChartWidget.h
* \brief The KDChart widget for usage without Interwiev.
*
* If you want to use KDChart with Interview, use KDChart::Chart instead.
*/

/**
 * Constructor. Creates a new widget with all data initialized empty.
 *
 * \param parent the widget parent; passed on to QWidget
 */
Widget::Widget( QWidget* parent ) :
    QWidget(parent), _d( new Private( this ) )
{
    // as default we have a cartesian coordinate plane ...
    // ... and a line diagram
    setType( Line );
}

/**
 * Destructor.
 */
Widget::~Widget()
{
    delete _d; _d = 0;
}

void Widget::init()
{
}

void Widget::setDataset( int column, const QVector< double > & data, const QString& title )
{
    if ( ! checkDatasetWidth( 1 ) )
        return;

    QStandardItemModel & model = d->m_model;

    justifyModelSize( data.size(), column + 1 );

    for( int i = 0; i < data.size(); ++i )
    {
        const QModelIndex index = model.index( i, column );
        model.setData( index, QVariant( data[i] ), Qt::DisplayRole );
    }
    if ( ! title.isEmpty() )
        model.setHeaderData( column, Qt::Horizontal, QVariant( title ) );
}

void Widget::setDataset( int column, const QVector< QPair< double, double > > & data, const QString& title )
{
    if ( ! checkDatasetWidth( 2 ))
        return;

    QStandardItemModel & model = d->m_model;

    justifyModelSize( data.size(), (column + 1) * 2 );

    for( int i = 0; i < data.size(); ++i )
    {
        QModelIndex index = model.index( i, column * 2 );
        model.setData( index, QVariant( data[i].first ), Qt::DisplayRole );

        index = model.index( i, column * 2 + 1 );
        model.setData( index, QVariant( data[i].second ), Qt::DisplayRole );
    }
    if ( ! title.isEmpty() ){
        model.setHeaderData( column,   Qt::Horizontal, QVariant( title ) );
    }
}

void Widget::setDataCell( int row, int column, double data )
{
    if ( ! checkDatasetWidth( 1 ) )
        return;

    QStandardItemModel & model = d->m_model;

    justifyModelSize( row + 1, column + 1 );

    const QModelIndex index = model.index( row, column );
    model.setData( index, QVariant( data ), Qt::DisplayRole );
}

void Widget::setDataCell( int row, int column, QPair< double, double > data )
{
    if ( ! checkDatasetWidth( 2 ))
        return;

    QStandardItemModel & model = d->m_model;

    justifyModelSize( row + 1, (column + 1) * 2 );

    QModelIndex index = model.index( row, column * 2 );
    model.setData( index, QVariant( data.first ), Qt::DisplayRole );

    index = model.index( row, column * 2 + 1 );
    model.setData( index, QVariant( data.second ), Qt::DisplayRole );
}

/*
 * Resets all data.
 */
void Widget::resetData()
{
    d->m_model.clear();
    d->usedDatasetWidth = 0;
}

/**
 * Sets all global leadings (borders).
 */
void Widget::setGlobalLeading( int left, int top, int right, int bottom )
{
    d->m_chart.setGlobalLeading( left, top, right, bottom );
}

/**
 * Sets the left leading (border).
 */
void Widget::setGlobalLeadingLeft( int leading )
{
    d->m_chart.setGlobalLeadingLeft( leading );
}

/**
 * Returns the left leading (border).
 */
int Widget::globalLeadingLeft() const
{
    return d->m_chart.globalLeadingLeft();
}

/**
 * Sets the top leading (border).
 */
void Widget::setGlobalLeadingTop( int leading )
{
    d->m_chart.setGlobalLeadingTop( leading );
}

/**
 * Returns the top leading (border).
 */
int Widget::globalLeadingTop() const
{
    return d->m_chart.globalLeadingTop();
}

/**
 * Sets the right leading (border).
 */
void Widget::setGlobalLeadingRight( int leading )
{
    d->m_chart.setGlobalLeadingRight( leading );
}

/**
 * Returns the right leading (border).
 */
int Widget::globalLeadingRight() const
{
    return d->m_chart.globalLeadingRight();
}

/**
 * Sets the bottom leading (border).
 */
void Widget::setGlobalLeadingBottom( int leading )
{
    d->m_chart.setGlobalLeadingBottom( leading );
}

/**
 * Returns the bottom leading (border).
 */
int Widget::globalLeadingBottom() const
{
    return d->m_chart.globalLeadingBottom();
}

/**
 * Returns the first of all headers.
 */
KDChart::HeaderFooter* Widget::firstHeaderFooter()
{
    return d->m_chart.headerFooter();
}

/**
 * Returns a list with all headers.
 */
QList<KDChart::HeaderFooter*> Widget::allHeadersFooters()
{
    return d->m_chart.headerFooters();
}

/**
 * Adds a new header/footer with the given text to the position.
 */
void Widget::addHeaderFooter( const QString& text,
                              HeaderFooter::HeaderFooterType type,
                              Position position)
{
    HeaderFooter* newHeader = new HeaderFooter( &d->m_chart );
    newHeader->setType( type );
    newHeader->setPosition( position );
    newHeader->setText( text );
    d->m_chart.addHeaderFooter( newHeader ); // we need this explicit call !
}

/**
 * Adds an existing header / footer object.
 */
void Widget::addHeaderFooter( HeaderFooter* header )
{
    header->setParent( &d->m_chart );
    d->m_chart.addHeaderFooter( header ); // we need this explicit call !
}

void Widget::replaceHeaderFooter( HeaderFooter* header, HeaderFooter* oldHeader )
{
    header->setParent( &d->m_chart );
    d->m_chart.replaceHeaderFooter( header, oldHeader );
}

void Widget::takeHeaderFooter( HeaderFooter* header )
{
    d->m_chart.takeHeaderFooter( header );
}

/**
 * Returns the first of all legends.
 */
KDChart::Legend* Widget::legend()
{
    return d->m_chart.legend();
}

/**
 * Returns a list with all legends.
 */
QList<KDChart::Legend*> Widget::allLegends()
{
    return d->m_chart.legends();
}

/**
 * Adds an empty legend on the given position.
 */
void Widget::addLegend( Position position )
{
    Legend* legend = new Legend( diagram(), &d->m_chart );
    legend->setPosition( position );
    d->m_chart.addLegend( legend );
}

/**
 * Adds a new, already existing, legend.
 */
void Widget::addLegend( Legend* legend )
{
    legend->setDiagram( diagram() );
    legend->setParent( &d->m_chart );
    d->m_chart.addLegend( legend );
}

void Widget::replaceLegend( Legend* legend, Legend* oldLegend )
{
    legend->setDiagram( diagram() );
    legend->setParent( &d->m_chart );
    d->m_chart.replaceLegend( legend, oldLegend );
}

void Widget::takeLegend( Legend* legend )
{
    d->m_chart.takeLegend( legend );
}

AbstractDiagram* Widget::diagram()
{
    if ( coordinatePlane() == 0 )
        qDebug() << "diagram(): coordinatePlane() was NULL";

    return coordinatePlane()->diagram();
}

BarDiagram* Widget::barDiagram()
{
    return dynamic_cast<BarDiagram*>( diagram() );
}
LineDiagram* Widget::lineDiagram()
{
    return dynamic_cast<LineDiagram*>( diagram() );
}
Plotter* Widget::plotter()
{
    return dynamic_cast<Plotter*>( diagram() );
}
PieDiagram* Widget::pieDiagram()
{
    return dynamic_cast<PieDiagram*>( diagram() );
}
RingDiagram* Widget::ringDiagram()
{
    return dynamic_cast<RingDiagram*>( diagram() );
}
PolarDiagram* Widget::polarDiagram()
{
    return dynamic_cast<PolarDiagram*>( diagram() );
}

AbstractCoordinatePlane* Widget::coordinatePlane()
{
    return d->m_chart.coordinatePlane();
}

static bool isCartesian( KDChart::Widget::ChartType type )
{
    return (type == KDChart::Widget::Bar) || (type == KDChart::Widget::Line);
}

static bool isPolar( KDChart::Widget::ChartType type )
{
    return     (type == KDChart::Widget::Pie)
            || (type == KDChart::Widget::Ring)
            || (type == KDChart::Widget::Polar);
}

void Widget::setType( ChartType chartType, SubType chartSubType )
{
    AbstractDiagram* diag = 0;
    const ChartType oldType = type();

    if ( chartType != oldType ){
        if( chartType != NoType ){
            if ( isCartesian( chartType ) && ! isCartesian( oldType ) )
            {
                if( coordinatePlane() == &d->m_polPlane ){
                    d->m_chart.takeCoordinatePlane( &d->m_polPlane );
                    d->m_chart.addCoordinatePlane(  &d->m_cartPlane );
                }else{
                    d->m_chart.replaceCoordinatePlane( &d->m_cartPlane );
                }
            }
            else if ( isPolar( chartType ) && ! isPolar( oldType ) )
            {
                if( coordinatePlane() == &d->m_cartPlane ){
                    d->m_chart.takeCoordinatePlane( &d->m_cartPlane );
                    d->m_chart.addCoordinatePlane(  &d->m_polPlane );
                }else{
                    d->m_chart.replaceCoordinatePlane( &d->m_polPlane );
                }
            }
        }
        switch ( chartType ){
            case Bar:
                diag = new BarDiagram( &d->m_chart, &d->m_cartPlane );
                break;
            case Line:
                diag = new LineDiagram( &d->m_chart, &d->m_cartPlane );
                break;
            case Plot:
                diag = new Plotter( &d->m_chart, &d->m_cartPlane );
                break;
            case Pie:
                diag = new PieDiagram( &d->m_chart, &d->m_polPlane );
                break;
            case Polar:
                diag = new PolarDiagram( &d->m_chart, &d->m_polPlane );
                break;
            case Ring:
                diag = new RingDiagram( &d->m_chart, &d->m_polPlane );
                break;
            case NoType:
                break;
        }
        if ( diag != NULL ){
            if ( isCartesian( oldType ) && isCartesian( chartType ) ){
                AbstractCartesianDiagram *oldDiag =
                        qobject_cast<AbstractCartesianDiagram*>( coordinatePlane()->diagram() );
                AbstractCartesianDiagram *newDiag =
                        qobject_cast<AbstractCartesianDiagram*>( diag );
                Q_FOREACH( CartesianAxis* axis, oldDiag->axes() ) {
                    oldDiag->takeAxis( axis );
                    newDiag->addAxis ( axis );
                }
            }
            diag->setModel( &d->m_model );
            coordinatePlane()->replaceDiagram( diag );

            LegendList legends = d->m_chart.legends();
            Q_FOREACH(Legend* l, legends)
                l->setDiagram( diag );
            //checkDatasetWidth( d->usedDatasetWidth );
        }
        //coordinatePlane()->setGridNeedsRecalculate();
    }

    if ( chartType != NoType ){
        if ( chartType != oldType || chartSubType != subType() )
            setSubType( chartSubType );
        d->m_chart.resize( size() ); // triggering immediate update
    }
}

void Widget::setSubType( SubType subType )
{
    BarDiagram*  barDia     = qobject_cast< BarDiagram* >(   diagram() );
    LineDiagram* lineDia    = qobject_cast< LineDiagram* >(  diagram() );
    Plotter*     plotterDia = qobject_cast< Plotter* >(      diagram() );

//FIXME(khz): Add the impl for these chart types - or remove them from here:
//    PieDiagram*   pieDia   = qobject_cast< PieDiagram* >(   diagram() );
//    PolarDiagram* polarDia = qobject_cast< PolarDiagram* >( diagram() );
//    RingDiagram*  ringDia  = qobject_cast< RingDiagram* >(  diagram() );

#define SET_SUB_TYPE(DIAGRAM, SUBTYPE) \
{ \
    if( DIAGRAM ) \
        DIAGRAM->setType( SUBTYPE ); \
}
    switch ( subType )
    {
        case Normal:
           SET_SUB_TYPE( barDia,     BarDiagram::Normal );
           SET_SUB_TYPE( lineDia,    LineDiagram::Normal );
           SET_SUB_TYPE( plotterDia, Plotter::Normal );
           break;
        case Stacked:
           SET_SUB_TYPE( barDia,  BarDiagram::Stacked );
           SET_SUB_TYPE( lineDia, LineDiagram::Stacked );
           //SET_SUB_TYPE( plotterDia, Plotter::Stacked );
           break;
        case Percent:
           SET_SUB_TYPE( barDia,  BarDiagram::Percent );
           SET_SUB_TYPE( lineDia, LineDiagram::Percent );
           SET_SUB_TYPE( plotterDia, Plotter::Percent );
           break;
        case Rows:
           SET_SUB_TYPE( barDia, BarDiagram::Rows );
           break;
        default:
           Q_ASSERT_X ( false,
                        "Widget::setSubType", "Sub-type not supported!" );
           break;
    }
//    coordinatePlane()->show();
}

/**
 * Returns the type of the chart.
 */
Widget::ChartType Widget::type() const
{
    // PENDING(christoph) save the type out-of-band:
    AbstractDiagram * const dia = const_cast<Widget*>( this )->diagram();
    if ( qobject_cast< BarDiagram* >( dia ) )
        return Bar;
    else if ( qobject_cast< LineDiagram* >( dia ) )
        return Line;
    else if ( qobject_cast< Plotter* >( dia ) )
        return Plot;
    else if( qobject_cast< PieDiagram* >( dia ) )
        return Pie;
    else if( qobject_cast< PolarDiagram* >( dia ) )
        return Polar;
    else if( qobject_cast< RingDiagram* >( dia ) )
        return Ring;
    else
        return NoType;
}

Widget::SubType Widget::subType() const
{
    // PENDING(christoph) save the type out-of-band:
    Widget::SubType retVal = Normal;

    AbstractDiagram * const dia = const_cast<Widget*>( this )->diagram();
    BarDiagram*  barDia     = qobject_cast< BarDiagram* >(   dia );
    LineDiagram* lineDia    = qobject_cast< LineDiagram* >(  dia );
    Plotter*     plotterDia = qobject_cast< Plotter* >(  dia );

//FIXME(khz): Add the impl for these chart types - or remove them from here:
//    PieDiagram*   pieDia   = qobject_cast< PieDiagram* >(   diagram() );
//    PolarDiagram* polarDia = qobject_cast< PolarDiagram* >( diagram() );
//    RingDiagram*  ringDia  = qobject_cast< RingDiagram* >(  diagram() );

#define TEST_SUB_TYPE(DIAGRAM, INTERNALSUBTYPE, SUBTYPE) \
{ \
    if( DIAGRAM && DIAGRAM->type() == INTERNALSUBTYPE ) \
        retVal = SUBTYPE; \
}
    const Widget::ChartType mainType = type();
    switch ( mainType )
    {
        case Bar:
           TEST_SUB_TYPE( barDia, BarDiagram::Normal,  Normal );
           TEST_SUB_TYPE( barDia, BarDiagram::Stacked, Stacked );
           TEST_SUB_TYPE( barDia, BarDiagram::Percent, Percent );
           TEST_SUB_TYPE( barDia, BarDiagram::Rows,    Rows );
           break;
        case Line:
            TEST_SUB_TYPE( lineDia, LineDiagram::Normal,  Normal );
            TEST_SUB_TYPE( lineDia, LineDiagram::Stacked, Stacked );
            TEST_SUB_TYPE( lineDia, LineDiagram::Percent, Percent );
            break;
        case Plot:
            TEST_SUB_TYPE( plotterDia, Plotter::Normal,  Normal );
            TEST_SUB_TYPE( plotterDia, Plotter::Percent, Percent );
            break;
        case Pie:
           // no impl. yet
           break;
        case Polar:
           // no impl. yet
           break;
        case Ring:
           // no impl. yet
           break;
        default:
           Q_ASSERT_X ( false,
                        "Widget::subType", "Chart type not supported!" );
           break;
    }
    return retVal;
}


/**
 * Checks whether the given width matches with the one used until now.
 */
bool Widget::checkDatasetWidth( int width )
{
    if( width == diagram()->datasetDimension() )
    {
        d->usedDatasetWidth = width;
        return true;
    }
    qDebug() << "The current diagram type doesn't support this data dimension.";
    return false;
/*    if ( d->usedDatasetWidth == width || d->usedDatasetWidth == 0 ) {
        d->usedDatasetWidth = width;
        diagram()->setDatasetDimension( width );
        return true;
    }
    qDebug() << "It's impossible to mix up the different setDataset() methods on the same widget.";
    return false;*/
}

/**
 * Justifies the model, so that the given rows and columns fit into it.
 */
void Widget::justifyModelSize( int rows, int columns )
{
    QAbstractItemModel & model = d->m_model;
    const int currentRows = model.rowCount();
    const int currentCols = model.columnCount();

    if ( currentCols < columns )
        if ( ! model.insertColumns( currentCols, columns - currentCols ))
            qDebug() << "justifyModelSize: could not increase model size.";
    if ( currentRows < rows )
        if ( ! model.insertRows( currentRows, rows - currentRows ))
            qDebug() << "justifyModelSize: could not increase model size.";

    Q_ASSERT( model.rowCount() >= rows );
    Q_ASSERT( model.columnCount() >= columns );
}
