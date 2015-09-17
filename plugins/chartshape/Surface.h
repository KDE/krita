/* This file is part of the KDE project

   Copyright 2007 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2009 Inge Wallin    <ingwa@lysator.liu.se>

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

#ifndef KCHART_SURFACE_H
#define KCHART_SURFACE_H

// Local
#include "ChartShape.h"

class KoStyleStack;
class KoOdfLoadingContext;
class QSizeF;

namespace KoChart {


/**
 * @brief The Surface class represents either a chart wall, or chart floor.
 * 
 * Currently only the wall can be visualized, though support for a
 * chart floor is needed to load and save in ODF without loss of data.
 */

class Surface
{
public:
    explicit Surface(PlotArea *parent);
    ~Surface();

    // QT5TODO: reenable the KDE_DEPRECATED flag or better remove this method
    /**
     * Old method of loading a pattern style; Use the Flake KoPatternBackground now.
     * Not all properties can be stored in a QBrush so this concept is obsolete.
     */
    /*KDE_DEPRECATED*/ static QBrush loadOdfPatternStyle(const KoStyleStack &styleStack, KoOdfLoadingContext &context, const QSizeF &size);

    QPointF position() const;
    int width() const;
    QBrush brush() const;
    QPen framePen() const;

    void setPosition(const QPointF &position);
    void setWidth(int);
    void setBrush(const QBrush &brush);
    void setFramePen(const QPen &pen);
    
    bool loadOdf(const KoXmlElement &surfaceElement, KoShapeLoadingContext &context);
    void saveOdf(KoShapeSavingContext &context, const char *elementName);

private:
    class Private;
    Private *const d;
};

}

#endif

