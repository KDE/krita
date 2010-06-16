/* This file is part of the KDE project

   Copyright 2007 Stefan Nikolaus     <stefan.nikolaus@kdemail.net>
   Copyright 2007-2010 Inge Wallin    <inge@lysator.liu.se>
   Copyright 2007-2008 Johannes Simon <johannes.simon@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCHART_SHAPE_H
#define KCHART_SHAPE_H


// Qt
#include <Qt>

// KOffice
#include <KoShapeContainer.h>
#include <KoFrameShape.h>

// KChart
#include "kchart_export.h"
#include "kchart_global.h"
#include "KoChartInterface.h"


class QAbstractItemModel;

class QPointF;
class QSizeF;
class QPen;
class QBrush;
class QColor;
class QString;
class QFont;

class KoResourceManager;
class KoShapeLoadingContext;
class KoShapeSavingContext;
class KoStore;
#include "KoXmlReaderForward.h"
class KoXmlWriter;
class KoGenStyles;
class KoOdfStylesReader;

// FIXME: Remove all mentions of KDChart from the public API.
namespace KDChart {
    class Chart;
    class Legend;
    class CartesianAxis;
    class CartesianCoordinatePlane;
    class AbstractCoordinatePlane;
    class AbstractDiagram;
}

// Interface to SimpleTextShape plugin
class KoTextShapeData;
#define TextShapeId "TextShapeID"
typedef KoTextShapeData TextLabelData;

namespace KChart {

class DataSet;
class ChartProxyModel;
class Legend;
class PlotArea;
class Surface;
class Axis;
class ThreeDScene;
class CellRegion;
class ChartTableModel;
class Layout;

extern const char *ODF_CHARTTYPES[ NUM_CHARTTYPES ];

extern const ChartSubtype defaultSubtypes[ NUM_CHARTTYPES ];

extern bool isPolar( ChartType type );
extern bool isCartesian( ChartType type );
extern QString saveOdfFont( KoGenStyles& mainStyles, const QFont& font, const QColor& color );
extern QColor defaultDataSetColor( int dataSetNum );


class CHARTSHAPELIB_EXPORT ChartShape
    : public QObject
    , public KoChart::ChartInterface // The public interface within KOffice
    , public KoFrameShape            // For saving as a frame
    , public KoShapeContainer        // The chart shape embeds other shapes.
{
    Q_OBJECT
    Q_INTERFACES(KoChart::ChartInterface)

public:
    ChartShape(KoResourceManager *documentResourceManager);
    ~ChartShape();

    // Getter methods
    QAbstractItemModel  *model()      const;
    ChartProxyModel     *proxyModel() const;

    // Parts of the chart
    KoShape        *title() const;
    TextLabelData  *titleData() const;
    KoShape        *subTitle() const;
    TextLabelData  *subTitleData() const;
    KoShape        *footer() const;
    TextLabelData  *footerData() const;
    Legend         *legend() const;
    PlotArea       *plotArea() const;
    Layout         *layout() const;

    /**
     * Returns a list of all labels in this chart, visible and hidden.
     * Use this method with caution, as it re-creates the list every
     * time you call it.
     */
    QList<KoShape*> labels() const;

    void showTitle(bool doShow);
    void showSubTitle(bool doShow);
    void showFooter(bool doShow);

    // Setter methods
    void setModel( QAbstractItemModel *model, bool takeOwnershipOfModel = false );
    void setModel( QAbstractItemModel *model, const QVector<QRect> &selection );
    bool addAxis( Axis *axis );
    bool removeAxis( Axis *axis );

    ChartType     chartType() const;
    ChartSubtype  chartSubType() const;
    bool          isThreeD() const;

    // Inherited from chart interface
    void setFirstRowIsLabel( bool isLabel );
    void setFirstColumnIsLabel( bool isLabel );
    void setDataDirection( Qt::Orientation orientation );

    void setChartType( ChartType type );
    void setChartSubType( ChartSubtype subType );
    void setThreeD( bool threeD );

    /// reimplemented
    void paintComponent( QPainter &painter, const KoViewConverter &converter );
    void paintDecorations( QPainter &painter, const KoViewConverter &converter,
                           const KoCanvasBase *canvas );

    /// reimplemented
    bool loadOdf( const KoXmlElement &element, KoShapeLoadingContext &context );
    bool loadOdfFrameElement( const KoXmlElement &element, KoShapeLoadingContext &context );
    bool loadOdfData( const KoXmlElement &tableElement, KoShapeLoadingContext &context );

    bool loadEmbeddedDocument( KoStore *store, const KoXmlElement &objectElement, const KoXmlDocument &manifestDocument );
    bool loadOdfEmbedded( const KoXmlElement &chartElement, KoShapeLoadingContext &context );
    /// reimplemented
    void saveOdf( KoShapeSavingContext &context ) const;
    void saveOdfData( KoXmlWriter &bodyWriter, KoGenStyles &mainStyles ) const;

    using KoShapeContainer::update;
    /// reimplemented
    void update() const;
    void relayout() const;

    void requestRepaint() const;

    /// the document resource manager we got on construction
    KoResourceManager *resourceManager() const;

signals:
    void chartTypeChanged( ChartType );

private:
    class Private;
    Private *const d;

    void showLabel( KoShape *label );
};

} // Namespace KChart

#endif
