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

#include "kritaflake_export.h"
#include <QList>
#include <QtGlobal>
#include "KoXmlReaderForward.h"
#include <boost/operators.hpp>

class QPainter;
class KoViewConverter;
class QRectF;
class QColor;
class QDomElement;
class QDomDocument;

/**
 * XXX
 */
class KRITAFLAKE_EXPORT KoGuidesData : boost::equality_comparable<KoGuidesData>
{
public:
    KoGuidesData();
    ~KoGuidesData();

    KoGuidesData(const KoGuidesData &rhs);
    KoGuidesData& operator=(const KoGuidesData &rhs);
    bool operator==(const KoGuidesData &rhs) const;

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

    bool showGuides() const;
    void setShowGuides(bool value);
    bool lockGuides() const;
    void setLockGuides(bool value);
    bool snapToGuides() const;
    void setSnapToGuides(bool value);

    /// Returns the list of horizontal guide lines.
    const QList<qreal>& horizontalGuideLines() const;

    /// Returns the list of vertical guide lines.
    const QList<qreal>& verticalGuideLines() const;

    bool hasGuides() const;

    /**
     * Sets the color of the guide lines.
     * @param color the new guides color
     */
    void setGuidesColor(const QColor &color);

    /// Returns the color of the guide lines.
    QColor guidesColor() const;

    QDomElement saveToXml(QDomDocument& doc, const QString &tag) const;
    bool loadFromXml(const QDomElement &parent);

private:
    class Private;
    Private * const d;
};


#endif

