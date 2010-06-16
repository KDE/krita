/****************************************************************************
 ** Copyright (C) 2007 Klarälvdalens Datakonsult AB.  All rights reserved.
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

#ifndef KDCHARTLEGEND_P_H
#define KDCHARTLEGEND_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KD Chart API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "KDChartLegend.h"
#include <KDChartDiagramObserver.h>
#include "KDChartAbstractAreaWidget_p.h"
#include <KDChartTextAttributes.h>
#include <KDChartMarkerAttributes.h>
#include <QList>
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QVector>

#include <KDABLibFakes>


class QGridLayout;
class KDTextDocument;
class QTextDocument;

namespace KDChart {
    class AbstractDiagram;
    class DiagramObserver;
    class AbstractLayoutItem;

    class DiagramsObserversList : public QList<DiagramObserver*> {};
}

/**
 * \internal
 */
class KDChart::Legend::Private : public AbstractAreaWidget::Private
{
    friend class KDChart::Legend;
public:
    Private();
    ~Private();

    Private( const Private& rhs ) :
        AbstractAreaWidget::Private( rhs ),
        referenceArea( 0 ),
        position( rhs.position ),
        alignment( rhs.alignment ),
        textAlignment( rhs.textAlignment ),
        relativePosition( rhs.relativePosition ),
        orientation( rhs.orientation ),
        order( rhs.order ),
        showLines( rhs.showLines ),
        texts( rhs.texts ),
        brushes( rhs.brushes ),
        pens( rhs.pens ),
        markerAttributes( rhs.markerAttributes ),
        textAttributes( rhs.textAttributes ),
        titleText( rhs.titleText ),
        titleTextAttributes( rhs.titleTextAttributes ),
        spacing( rhs.spacing ),
        useAutomaticMarkerSize( rhs.useAutomaticMarkerSize ),
        legendStyle( MarkersOnly )
        //needRebuild( true )
        {
        }

    DiagramObserver* findObserverForDiagram( AbstractDiagram* diagram )
    {
        for (int i = 0; i < observers.size(); ++i) {
            DiagramObserver * obs = observers.at(i);
            if( obs->diagram() == diagram )
                return obs;
        }
        return 0;
    }

private:
    // user-settable
    const QWidget* referenceArea;
    Position position;
    Qt::Alignment alignment;
    Qt::Alignment textAlignment;
    RelativePosition relativePosition;
    Qt::Orientation orientation;
    Qt::SortOrder order;
    bool showLines;
    QMap<uint,QString> texts;
    QMap<uint,QBrush> brushes;
    QMap<uint,QPen> pens;
    QMap<uint, MarkerAttributes> markerAttributes;
    QList<uint> hiddenDatasets;
    TextAttributes textAttributes;
    QString titleText;
    TextAttributes titleTextAttributes;
    uint spacing;
    bool useAutomaticMarkerSize;
    LegendStyle legendStyle;

    // internal
//    bool needRebuild;
    mutable QStringList modelLabels;
    mutable QList<QBrush> modelBrushes;
    mutable QList<QPen> modelPens;
    mutable QList<MarkerAttributes> modelMarkers;
    mutable QSize cachedSizeHint;
    //QVector<KDChart::AbstractLayoutItem*> layoutItems;
    QVector<KDChart::AbstractLayoutItem*> layoutItems;
    QGridLayout* layout;
    DiagramsObserversList observers;
};

inline KDChart::Legend::Legend( Private* p, QWidget* parent )
    : AbstractAreaWidget( p, parent ) { init(); }
inline KDChart::Legend::Private * KDChart::Legend::d_func()
{ return static_cast<Private*>( AbstractAreaWidget::d_func() ); }
inline const KDChart::Legend::Private * KDChart::Legend::d_func() const
{ return static_cast<const Private*>( AbstractAreaWidget::d_func() ); }




#endif /* KDCHARTLEGEND_P_H */

