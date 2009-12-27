/* This file is part of the KDE project
   Copyright (C) 2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOGUIDESDATA_H
#define KOGUIDESDATA_H

#include "flake_export.h"
#include <QtCore/QList>
#include <QtCore/Qt>
#include "KoXmlReaderForward.h"

class QPainter;
class KoViewConverter;
class QRectF;
class QColor;
class KoXmlWriter;

/**
 * XXX
 */
class FLAKE_EXPORT KoGuidesData
{
public:
    KoGuidesData();
    ~KoGuidesData();

    /**
     * @brief Set the positions of the horizontal guide lines
     *
     * @param lines a list of positions of the horizontal guide lines
     */
    void setHorizontalGuideLines(const QList<qreal> &lines);

    /**
     * @brief Set the positions of the vertical guide lines
     *
     * @param lines a list of positions of the vertical guide lines
     */
    void setVerticalGuideLines(const QList<qreal> &lines);

    /**
     * @brief Add a guide line to the canvas.
     *
     * @param orientation the orientation of the guide line
     * @param position the position in document coordinates of the guide line
     */
    void addGuideLine(Qt::Orientation orientation, qreal position);

    /**
     * @brief Display or not guide lines
     */
    bool showGuideLines() const;

    /**
     * @param show display or not guide line
     */
    void setShowGuideLines(bool show);

    /// Returns the list of horizontal guide lines.
    QList<qreal> horizontalGuideLines() const;

    /// Returns the list of vertical guide lines.
    QList<qreal> verticalGuideLines() const;

    /**
     * Paints the guides using the given painter and viewconverter.
     * Only guides intersecting the given area are painted.
     * @param painter the painter
     * @param converter the view converter
     * @param area the area in need of updating
     */
    void paintGuides(QPainter &painter, const KoViewConverter &converter, const QRectF &area) const;

    /**
     * Sets the color of the guide lines.
     * @param color the new guides color
     */
    void setGuidesColor(const QColor &color);

    /// Returns the color of the guide lines.
    QColor guidesColor() const;

    /// Loads guide lines from the given setting xml document
    bool loadOdfSettings(const KoXmlDocument &settingsDoc);

    /// Saves guide lines to the given settings xml writer
    void saveOdfSettings(KoXmlWriter &settingsWriter);

private:
    class Private;
    Private * const d;
};


#endif

