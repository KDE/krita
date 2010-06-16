/* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2005-2007 Klar�vdalens Datakonsult AB.  All rights reserved.
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

#ifndef KDCHARTLEGEND_H
#define KDCHARTLEGEND_H

#include "KDChartAbstractAreaWidget.h"
#include "KDChartPosition.h"
#include "KDChartMarkerAttributes.h"

class QTextTable;

namespace KDChart {

    class AbstractDiagram;
    typedef QList<AbstractDiagram*> DiagramList;
    typedef QList<const AbstractDiagram*> ConstDiagramList;

/**
  * @brief Legend defines the interface for the legend drawing class.
  *
  * Legend is the class for drawing legends for all kinds of diagrams ("chart types").
  *
  * Legend is drawn on chart level, not per diagram, but you can have more than one
  * legend per chart, using KDChart::Chart::addLegend().
  *
  * \note Legend is different from all other classes ofd KD Chart, since it can be
  * displayed outside of the Chart's area.  If you want to, you can embedd the legend
  * into your own widget, or into another part of a bigger grid, into which you might
  * have inserted the Chart.
  *
  * On the other hand, please note that you MUST call Chart::addLegend to get your
  * legend positioned into the correct place of your chart - if you want to have
  * the legend shown inside of the chart (that's probably true for most cases).
  */
class KDCHART_EXPORT Legend : public AbstractAreaWidget
{
    Q_OBJECT

    Q_DISABLE_COPY( Legend )
    KDCHART_DECLARE_PRIVATE_DERIVED_QWIDGET( Legend )

public:
    explicit Legend( QWidget* parent = 0 );
    explicit Legend( KDChart::AbstractDiagram* diagram, QWidget* parent );
    virtual ~Legend();


    enum LegendStyle { MarkersOnly     = 0,
                       LinesOnly       = 1,
                       MarkersAndLines = 2 };


    void setLegendStyle( LegendStyle style );
    LegendStyle legendStyle() const;


    virtual Legend * clone() const;

    /**
     * Returns true if both legends have the same settings.
     */
    bool compare( const Legend* other )const;

    //QSize calcSizeHint() const;
    virtual void resizeEvent( QResizeEvent * event ); // TODO: should be protected

    virtual void paint( QPainter* painter );
    virtual void setVisible( bool visible );

    /**
        Specifies the reference area for font size of title text,
        and for font size of the item texts, IF automatic area
        detection is set.

        \note This parameter is ignored, if the Measure given for
        setTitleTextAttributes (or setTextAttributes, resp.) is
        not specifying automatic area detection.

        If no reference area is specified, but automatic area
        detection is set, then the size of the legend's parent
        widget will be used.

        \sa KDChart::Measure, KDChartEnums::MeasureCalculationMode
    */
    void setReferenceArea( const QWidget* area );
    /**
        Returns the reference area, that is used for font size of title text,
        and for font size of the item texts, IF automatic area
        detection is set.

        \sa setReferenceArea
    */
    const QWidget* referenceArea() const;

    /**
      * The first diagram of the legend or 0 if there was none added to the legend.
      * @return The first diagram of the legend or 0.
      *
      * \sa diagrams, addDiagram, removeDiagram, removeDiagrams, replaceDiagram, setDiagram
      */
    KDChart::AbstractDiagram* diagram() const;

    /**
      * The list of all diagrams associated with the legend.
      * @return The list of all diagrams associated with the legend.
      *
      * \sa diagram, addDiagram, removeDiagram, removeDiagrams, replaceDiagram, setDiagram
      */
    DiagramList diagrams() const;

    /**
     * @return The list of diagrams associated with this coordinate plane.
     */
    ConstDiagramList constDiagrams() const;

    /**
      * Add the given diagram to the legend.
      * @param newDiagram The diagram to add.
      *
      * \sa diagram, diagrams, removeDiagram, removeDiagrams, replaceDiagram, setDiagram
      */
    void addDiagram( KDChart::AbstractDiagram* newDiagram );

    /**
      * Removes the diagram from the legend's list of diagrams.
      *
      * \sa diagram, diagrams, addDiagram, removeDiagrams, replaceDiagram, setDiagram
      */
    void removeDiagram( KDChart::AbstractDiagram* oldDiagram );

    /**
      * Removes all of the diagram from the legend's list of diagrams.
      *
      * \sa diagram, diagrams, addDiagram, removeDiagram, replaceDiagram, setDiagram
      */
    void removeDiagrams();

    /**
      * Replaces the old diagram, or appends the
      * new diagram, it there is none yet.
      *
      * @param newDiagram The diagram to be used instead of the old one.
      * If this parameter is zero, the first diagram will just be removed.
      *
      * @param oldDiagram The diagram to be removed by the new one. This
      * diagram will be deleted automatically. If the parameter is omitted,
      * the very first diagram will be replaced. In case, there was no
      * diagram yet, the new diagram will just be added.
      *
      * \sa diagram, diagrams, addDiagram, removeDiagram, removeDiagrams, setDiagram
      */
    void replaceDiagram( KDChart::AbstractDiagram* newDiagram,
                         KDChart::AbstractDiagram* oldDiagram = 0 );

    /**
      * @brief A convenience method doing the same as replaceDiagram( newDiagram, 0 );
      *
      * Replaces the first diagram by the given diagram.
      * If the legend's list of diagram is empty the given diagram is added to the list.
      *
      * \sa diagram, diagrams, addDiagram, removeDiagram, removeDiagrams, replaceDiagram
      */
    void setDiagram( KDChart::AbstractDiagram* newDiagram );

    /**
     * \brief Specify the position of a non-floating legend.
     *
     * Use setFloatingPosition to set position and alignment
     * if your legend is floating.
     *
     * \sa setAlignment, setFloatingPosition
     */
    void setPosition( Position position );

    /**
     * Returns the position of a non-floating legend.
     * \sa setPosition
     */
    Position position() const;

    /**
     * \brief Specify the alignment of a non-floating legend.
     *
     * Use setFloatingPosition to set position and alignment
     * if your legend is floating.
     *
     * \sa alignment, setPosition, setFloatingPosition
     */
    void setAlignment( Qt::Alignment );

    /**
     * Returns the alignment of a non-floating legend.
     * \sa setAlignment
     */
    Qt::Alignment alignment() const;

    /**
     * \brief Specify the alignment of the text elements within the legend
     *
     * \sa textAlignment()
     */
    void setTextAlignment( Qt::Alignment );

    /**
     * \brief Returns the alignment used while rendering text elements within the legend.
     *
     * \sa setTextAlignment()
     */
     Qt::Alignment textAlignment() const;

    /**
     * \brief Specify the position and alignment of a floating legend.
     *
     * Use setPosition and setAlignment to set position and alignment
     * if your legend is non-floating.
     *
     * \note When setFloatingPosition is called, the Legend's position value is set to
     * KDChart::Position::Floating automatically.
     *
     * If your Chart is pointed to by m_chart, your could have the floating legend
     * aligned exactly to the chart's coordinate plane's top-right corner
     * with the following commands:
\verbatim
KDChart::RelativePosition relativePosition;
relativePosition.setReferenceArea( m_chart->coordinatePlane() );
relativePosition.setReferencePosition( Position::NorthEast );
relativePosition.setAlignment( Qt::AlignTop | Qt::AlignRight );
relativePosition.setHorizontalPadding(
    KDChart::Measure( -1.0, KDChartEnums::MeasureCalculationModeAbsolute ) );
relativePosition.setVerticalPadding(
    KDChart::Measure( 0.0, KDChartEnums::MeasureCalculationModeAbsolute ) );
m_legend->setFloatingPosition( relativePosition );
\endverbatim
     *
     * To have the legend positioned at a fixed point, measured from the QPainter's top left corner,
     * you could use the following code code:
     *
\verbatim
KDChart::RelativePosition relativePosition;
relativePosition.setReferencePoints( PositionPoints( QPointF( 0.0, 0.0 ) ) );
relativePosition.setReferencePosition( Position::NorthWest );
relativePosition.setAlignment( Qt::AlignTop | Qt::AlignLeft );
relativePosition.setHorizontalPadding(
    KDChart::Measure( 4.0, KDChartEnums::MeasureCalculationModeAbsolute ) );
relativePosition.setVerticalPadding(
    KDChart::Measure( 4.0, KDChartEnums::MeasureCalculationModeAbsolute ) );
m_legend->setFloatingPosition( relativePosition );
\endverbatim
     * Actually that's exactly the code KD Chart is using as default position for any floating legends,
     * so if you just say setPosition( KDChart::Position::Floating ) without calling setFloatingPosition
     * your legend will be positioned at point 4/4.
     *
     * \sa setPosition, setAlignment
     */
    void setFloatingPosition( const RelativePosition& relativePosition );

    /**
     * Returns the position of a floating legend.
     * \sa setFloatingPosition
     */
    const RelativePosition floatingPosition() const;

    void setOrientation( Qt::Orientation orientation );
    Qt::Orientation orientation() const;


    void setSortOrder( Qt::SortOrder order );
    Qt::SortOrder sortOrder() const;

    void setShowLines( bool legendShowLines );
    bool showLines() const;

    void resetTexts();
    void setText( uint dataset, const QString& text );
    QString text( uint dataset ) const;
    const QMap<uint,QString> texts() const;

    /**
     * Sets a list of datasets that are to be hidden in the legend.
     *
     * By passing an empty list, you show all datasets.
     * Note that by default, all datasets are shown, which means
     * that hiddenDatasets() == QList<uint>()
     */
    void setHiddenDatasets( const QList<uint> hiddenDatasets );
    const QList<uint> hiddenDatasets() const;
    void setDatasetHidden( uint dataset, bool hidden );
    bool datasetIsHidden( uint dataset ) const;

    uint datasetCount() const;

    void setDefaultColors();
    void setRainbowColors();
    void setSubduedColors( bool ordered = false );

    void setBrushesFromDiagram( KDChart::AbstractDiagram* diagram );

    /**
     * Note: there is no color() getter method, since setColor
     * just sets a QBrush with the respective color, so the
     * brush() getter method is sufficient.
     */
    void setColor( uint dataset, const QColor& color );

    void setBrush( uint dataset, const QBrush& brush );
    QBrush brush( uint dataset ) const;
    const QMap<uint,QBrush> brushes() const;

    void setPen( uint dataset, const QPen& pen );
    QPen pen( uint dataset ) const;
    const QMap<uint,QPen> pens() const;

    /**
     * Note that any sizes specified via setMarkerAttributes are ignored,
     * unless you disable the automatic size calculation, by saying
     * setUseAutomaticMarkerSize( false )
     */
    void setMarkerAttributes( uint dataset, const MarkerAttributes& );
    MarkerAttributes markerAttributes( uint dataset ) const;
    const QMap<uint, MarkerAttributes> markerAttributes() const;

    /**
     * This option is on by default, it means that Marker sizes in the Legend
     * will be the same as the font height used for their respective label texts.
     *
     * Set this to false, if you want to specify the marker sizes via setMarkerAttributes
     * or if you want the Legend to use the same marker sizes as they are used in the Diagrams.
     */
    void setUseAutomaticMarkerSize( bool useAutomaticMarkerSize );
    bool useAutomaticMarkerSize() const;

    void setTextAttributes( const TextAttributes &a );
    TextAttributes textAttributes() const;

    void setTitleText( const QString& text );
    QString titleText() const;

    void setTitleTextAttributes( const TextAttributes &a );
    TextAttributes titleTextAttributes() const;

    // FIXME same as frameSettings()->padding()?
    void setSpacing( uint space );
    uint spacing() const;

    // called internally by KDChart::Chart, when painting into a custom QPainter
    virtual void forceRebuild();

    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;
    virtual void needSizeHint();
    virtual void resizeLayout( const QSize& size );

/*public static*/
//    static LegendPosition stringToPosition( QString name, bool* ok=0 );

Q_SIGNALS:
    void destroyedLegend( Legend* );
    /** Emitted upon change of a property of the Legend or any of its components. */
    void propertiesChanged();

private Q_SLOTS:
    void emitPositionChanged();
    void resetDiagram( AbstractDiagram* );
    void activateTheLayout();
    void setNeedRebuild();
    void buildLegend();
}; // End of class Legend

}


#endif // KDCHARTLEGEND_H
