/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2011 Pierre Ducroquet <pinaraf@pinaraf.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOTEXTLAYOUTCELLHELPER_H
#define KOTEXTLAYOUTCELLHELPER_H

#include "kritatextlayout_export.h"

#include <KoBorder.h>

#include <QObject>

class KoTableCellStyle;
class QPainter;

class KRITATEXTLAYOUT_EXPORT KoTextLayoutCellHelper : public QObject
{
    Q_OBJECT
public:
    explicit KoTextLayoutCellHelper(const KoTableCellStyle &cellStyle, QObject *parent = 0);

    /// draws a horizontal wave line
    void drawHorizontalWave(KoBorder::BorderStyle style, QPainter &painter, qreal x, qreal w, qreal t) const;

    /// draws a vertical wave line
    void drawVerticalWave(KoBorder::BorderStyle style, QPainter &painter, qreal y, qreal h, qreal t) const;

    /**
     * Paint the borders.
     *
     * @param painter the painter to draw with.
     * @param bounds the bounding rectangle to draw.
     * @param blanks a painterpath where blank borders should be added to.
     */
    void paintBorders(QPainter &painter, const QRectF &bounds, QVector<QLineF> *blanks) const;

    /**
     * Paint the diagonal borders.
     *
     * @param painter the painter to draw with.
     * @param bounds the bounding rectangle to draw.
     */
    void paintDiagonalBorders(QPainter &painter, const QRectF &bounds) const;

    /**
     * Paint the top border.
     *
     * @param painter the painter to draw with.
     * @param x the x position.
     * @param y the y position.
     * @param w the width.
     * @param blanks a painterpath where blank borders should be added to.
     */
    void drawTopHorizontalBorder(QPainter &painter, qreal x, qreal y, qreal w, QVector<QLineF> *blanks = 0) const;

    /**
     * Paint the border that is shared.
     * It only draws the thickest and it always draws it below the y position.
     *
     * @param painter the painter to draw with.
     * @param styleBelow the table cell style.
     * @param x the x position.
     * @param y the y position.
     * @param w the width.
     * @param blanks a painterpath where blank borders should be added to.
     */
    void drawSharedHorizontalBorder(QPainter &painter, const KoTableCellStyle &styleBelow,  qreal x, qreal y, qreal w, QVector<QLineF> *blanks = 0) const;

    /**
     * Paint the bottom border.
     *
     * @param painter the painter to draw with.
     * @param x the x position.
     * @param y the y position.
     * @param w the width.
     * @param blanks a painterpath where blank borders should be added to.
     */
    void drawBottomHorizontalBorder(QPainter &painter, qreal x, qreal y, qreal w, QVector<QLineF> *blanks = 0) const;

    /**
     * Paint the leftmost border.
     *
     * @param painter the painter to draw with.
     * @param x the x position.
     * @param y the y position.
     * @param h the height.
     * @param blanks a painterpath where blank borders should be added to.
     */
    void drawLeftmostVerticalBorder(QPainter &painter, qreal x, qreal y, qreal h, QVector<QLineF> *blanks = 0) const;

    /**
     * Paint the border that is shared.
     * It only draws the thickest and it always draws it to the right of the x position.
     *
     * @param painter the painter to draw with.
     * @param styleRight the table cell style.
     * @param x the x position.
     * @param y the y position.
     * @param h the height.
     * @param blanks a painterpath where blank borders should be added to.
     */
    void drawSharedVerticalBorder(QPainter &painter, const KoTableCellStyle &styleRight,  qreal x, qreal y, qreal h, QVector<QLineF> *blanks = 0) const;

    /**
     * Paint the rightmost border.
     *
     * @param painter the painter to draw with.
     * @param x the x position.
     * @param y the y position.
     * @param h the height.
     * @param blanks a painterpath where blank borders should be added to.
     */
    void drawRightmostVerticalBorder(QPainter &painter, qreal x, qreal y, qreal h, QVector<QLineF> *blanks = 0) const;

private:
    const KoTableCellStyle &m_cellStyle;
};

#endif // KOTEXTLAYOUTCELLHELPER_H
