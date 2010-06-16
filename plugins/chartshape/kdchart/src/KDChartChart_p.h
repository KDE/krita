/****************************************************************************
 ** Copyright (C) 2007 Klaralvdalens Datakonsult AB.  All rights reserved.
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

#ifndef KDCHARTCHART_P_H
#define KDCHARTCHART_P_H

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

#include <QObject>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "KDChartChart.h"
#include "KDChartAbstractArea.h"
#include "KDChartTextArea.h"
#include "KDChartFrameAttributes.h"
#include "KDChartBackgroundAttributes.h"
#include "KDChartLayoutItems.h"

#include <KDABLibFakes>


namespace KDChart {

/*
  struct PlaneInfo can't be declared inside Chart::Private, otherwise MSVC.net says:
  qhash.h(195) : error C2248: 'KDChart::Chart::Private' : cannot access protected class declared in class 'KDChart::Chart'
  KDChartChart_p.h(58) : see declaration of 'KDChart::Chart::Private'
  KDChartChart.h(61) : see declaration of 'KDChart::Chart'
  KDChartChart.cpp(262) : see reference to class template instantiation 'QHash<Key,T>' being compiled, with
            Key=KDChart::AbstractCoordinatePlane *,
            T=KDChart::Chart::Private::PlaneInfo
*/
/**
 * \internal
 */
struct PlaneInfo {
    PlaneInfo()
        : referencePlane( 0 ),
          horizontalOffset( 1 ),
          verticalOffset( 1 ),
          gridLayout( 0 ),
          topAxesLayout( 0 ),
          bottomAxesLayout( 0 ),
          leftAxesLayout( 0 ),
          rightAxesLayout( 0 )
    {}
    AbstractCoordinatePlane *referencePlane;
    int horizontalOffset;
    int verticalOffset;
    QGridLayout* gridLayout;
    QVBoxLayout* topAxesLayout;
    QVBoxLayout* bottomAxesLayout;
    QHBoxLayout* leftAxesLayout;
    QHBoxLayout* rightAxesLayout;
};


/**
 * \internal
 */
class Chart::Private : public QObject
{
    Q_OBJECT
    public:
        CoordinatePlaneList coordinatePlanes;
        HeaderFooterList headerFooters;
        LegendList legends;

        Chart* chart;
        QHBoxLayout* layout;
        QVBoxLayout* vLayout;
        QBoxLayout*  planesLayout;
        QGridLayout* headerLayout;
        QGridLayout* footerLayout;
        QGridLayout* dataAndLegendLayout;

        QVBoxLayout* innerHdFtLayouts[2][3][3]; // auxiliary pointers

        QMap< int, QMap< int, HorizontalLineLayoutItem > > dummyHeaders;
        QMap< int, QMap< int, HorizontalLineLayoutItem > > dummyFooters;

        QVector<KDChart::TextArea*> textLayoutItems;
        QVector<KDChart::AbstractArea*> layoutItems;
        QVector<KDChart::AbstractLayoutItem*> planeLayoutItems;
        QVector<KDChart::Legend*> legendLayoutItems;

        QSize currentLayoutSize;

        // since we do not want to derive Chart from AbstractAreaBase,
        // we store the attributes here, and then we call two static painting
        // methods to drawn the background (or frame, resp.).
        KDChart::FrameAttributes frameAttributes;
        KDChart::BackgroundAttributes backgroundAttributes;

        int globalLeadingLeft, globalLeadingRight, globalLeadingTop, globalLeadingBottom;

        QList< AbstractCoordinatePlane* > mouseClickedPlanes;

        Private ( Chart* );

        virtual ~Private();

        void removeDummyHeaderFooters();

        void createLayouts( QWidget * parent );
        void layoutLegends();
        void layoutHeadersAndFooters();
        void resizeLayout( const QSize& sz );
        void paintAll( QPainter* painter );

        struct AxisInfo {
            AxisInfo()
                :plane(0)
            {}
            AbstractCoordinatePlane *plane;
        };

        QHash<AbstractCoordinatePlane*, PlaneInfo> buildPlaneLayoutInfos();

    public Q_SLOTS:
        void slotLayoutPlanes();
        void slotRelayout();
        void slotUnregisterDestroyedLegend( Legend * legend );
        void slotUnregisterDestroyedHeaderFooter( HeaderFooter* headerFooter );
        void slotUnregisterDestroyedPlane( AbstractCoordinatePlane* plane );
        /*
        // Unused code trying to use a push-model: This did not work
        // since we can not re-layout the planes each time when
        // Qt layouting is calling sizeHint()
        void slotAdjustLeftRightColumnsForOverlappingLabels(
                CartesianAxis* axis, int left, int right);
        void slotAdjustTopBottomRowsForOverlappingLabels(
                CartesianAxis* axis, int top, int bottom);
        */
};

}

#endif
