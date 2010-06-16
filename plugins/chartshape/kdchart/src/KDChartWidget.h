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
#ifndef __KDCHARTWIDGET_H__
#define __KDCHARTWIDGET_H__

#include "KDChartGlobal.h"

#include <QWidget>

#include "KDChartEnums.h"
#include "KDChartHeaderFooter.h"

template <typename T> class QVector;
template <typename T1, typename T2> struct QPair;

namespace KDChart {

    // some forward declarations
    class AbstractDiagram;
    class Chart;
    class AbstractCoordinatePlane;
    class TableModel;
    class BarDiagram;
    class LineDiagram;
    class Plotter;
    class PieDiagram;
    class RingDiagram;
    class PolarDiagram;
    class Legend;
    class Position;

    /**
     * \class Widget KDChartWidget.h
     * \brief The KD Chart widget for usage without Model/View.
     *
     * If you want to use KD Chart with Model/View, use KDChart::Chart instead.
     */
    class KDCHART_EXPORT Widget : public QWidget
    {
        Q_OBJECT

        Q_DISABLE_COPY( Widget )
        KDCHART_DECLARE_PRIVATE_BASE_POLYMORPHIC_QWIDGET( Widget )

    public:
        /**
         * Standard Qt-style Constructor
         *
         * Creates a new widget with all data initialized empty.
         *
         * \param parent the widget parent; passed on to QWidget
         */
        explicit Widget( QWidget* parent = 0 );

        /** Destructor. */
        ~Widget();
        /** Sets the data in the given column using a QVector of double for the Y values. */
        void setDataset( int column, const QVector< double > & data, const QString& title = QString()  );
        /** Sets the data in the given column using a QVector of QPairs
         *  of double for the (X, Y) values. */
        void setDataset( int column, const QVector< QPair< double, double > > &  data, const QString& title = QString() );
        /** Sets the Y value data for a given cell. */
        void setDataCell( int row, int column, double data );
        /** Sets the data for a given column using an (X, Y) QPair of doubles. */
        void setDataCell( int row, int column, QPair< double, double > data );
        /** Resets all data. */
        void resetData();

    public Q_SLOTS:
        /** Sets all global leadings (borders). */
        void setGlobalLeading( int left, int top, int right, int bottom );
        /** Sets the left leading (border). */
        void setGlobalLeadingLeft( int leading );
        /** Sets the top leading (border). */
        void setGlobalLeadingTop( int leading );
        /** Sets the right leading (border). */
        void setGlobalLeadingRight( int leading );
        /** Sets the bottom leading (border). */
        void setGlobalLeadingBottom( int leading );

    public:
        /** Returns the left leading (border). */
        int globalLeadingLeft() const;
        /** Returns the top leading (border). */
        int globalLeadingTop() const;
        /** Returns the right leading (border). */
        int globalLeadingRight() const;
        /** Returns the bottom leading (border). */
        int globalLeadingBottom() const;

        /** Returns the first of all headers. */
        HeaderFooter* firstHeaderFooter();
        /** Returns a list with all headers. */
        QList<HeaderFooter*> allHeadersFooters();

        /** Adds a new header/footer with the given text to the position. */
        void addHeaderFooter( const QString& text,
                              HeaderFooter::HeaderFooterType type,
                              Position position );

        /**
          * Adds the existing header / footer object \a header.
          * \sa replaceHeaderFooter, takeHeaderFooter
        */
        void addHeaderFooter( HeaderFooter* header );

        /**
         * Replaces the old header (or footer, resp.), or appends the
         * new header or footer, it there is none yet.
         *
         * @param header The header or footer to be used instead of the old one.
         * This parameter must not be zero, or the method will do nothing.
         *
         * @param oldHeader The header or footer to be removed by the new one. This
         * header or footer will be deleted automatically. If the parameter is omitted,
         * the very first header or footer will be replaced. In case, there was no
         * header and no footer yet, the new header or footer will just be added.
         *
         * \note If you want to re-use the old header or footer, call takeHeaderFooter and
         * addHeaderFooter, instead of using replaceHeaderFooter.
         *
         * \sa addHeaderFooter, takeHeaderFooter
         */
        void replaceHeaderFooter( HeaderFooter* header,
                                  HeaderFooter* oldHeader = 0 );

        /** Remove the header (or footer, resp.) from the widget,
         * without deleting it.
         * The chart no longer owns the header or footer, so it is
         * the caller's responsibility to delete the header or footer.
         *
         * \sa addHeaderFooter, replaceHeaderFooter
         */
        void takeHeaderFooter( HeaderFooter* header );

        /** Returns the first of all legends. */
        Legend* legend();
        /** Returns a list with all legends. */
        QList<Legend*> allLegends();

        /** Adds an empty legend on the given position. */
        void addLegend( Position position );
        /** Adds a new, already existing, legend. */
        void addLegend (Legend* legend );

        void replaceLegend( Legend* legend, Legend* oldLegend = 0 );
        void takeLegend( Legend* legend );


        /** Returns a pointer to the current diagram. */
        AbstractDiagram* diagram();

        /** If the current diagram is a BarDiagram, it is returnd; otherwise 0 is returned.
          * This function provides type-safe casting.
          */
        BarDiagram* barDiagram();
        /** If the current diagram is a LineDiagram, it is returnd; otherwise 0 is returned.
         * This function provides type-safe casting.
         */
        LineDiagram* lineDiagram();
        /** If the current diagram is a LineDiagram, it is returnd; otherwise 0 is returned.
         * This function provides type-safe casting.
         *
         * \note Do not use lineDiagram for multi-dimensional diagrams, but use plotter instead
         *
         * \sa plotter
         */
        Plotter* plotter();
        /** If the current diagram is a Plotter, it is returnd; otherwise 0 is returned.
          * This function provides type-safe casting.
          */
        PieDiagram* pieDiagram();
        /** If the current diagram is a RingDiagram, it is returnd; otherwise 0 is returned.
          * This function provides type-safe casting.
          */
        RingDiagram* ringDiagram();
        /** If the current diagram is a PolarDiagram, it is returnd; otherwise 0 is returned.
          * This function provides type-safe casting.
          */
        PolarDiagram* polarDiagram();

        /** Returns a pointer to the current coordinate plane. */
        AbstractCoordinatePlane* coordinatePlane();


        enum ChartType { NoType, Bar, Line, Plot, Pie, Ring, Polar };

        /** Returns the type of the chart. */
        ChartType type() const;

        /** Sub type values, matching the values defines for the respective Diagram classes. */
        enum SubType { Normal, Stacked, Percent, Rows };

        /** Returns the sub-type of the chart. */
        SubType subType() const;

    public Q_SLOTS:
        /** Sets the type of the chart. */
        void setType( ChartType chartType, SubType subType=Normal );
        /** \brief Sets the type of the chart without changing the main type.
          *
          * Make sure to use a sub-type that matches the main type,
          * so e.g. setting sub-type Rows makes sense for Bar charts only,
          * and it will be ignored for all other chart types.
          *
          * \sa KDChartBarDiagram::BarType, KDChartLineDiagram::LineType
          * \sa KDChartPieDiagram::PieType, KDChartRingDiagram::RingType
          * \sa KDChartPolarDiagram::PolarType
          */
        void setSubType( SubType subType );

    private:
        /** Justifies the model, so that the given rows and columns fit into it. */
        void justifyModelSize( int rows, int columns );
        /** Checks wether the given width matches with the one used until now. */
        bool checkDatasetWidth( int width );
    };
}

#endif // KDChartWidget_H
