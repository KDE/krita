/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
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

#ifndef _LINESTYLESELECTOR_H_
#define _LINESTYLESELECTOR_H_

#include <QComboBox>

/**
 * A custom combobox widget for selecting line styles.
 */
class LineStyleSelector : public QComboBox
{
    Q_OBJECT
public:
    LineStyleSelector( QWidget * parent = 0 );
    virtual ~LineStyleSelector();

    /**
     * Adds a new line style to the combobox.
     *
     * If the style already exists, it is not added to the selector.
     *
     * @param style the line style to add
     * @return true if style is unique among the existing styles and was added, else false
     */
    bool addCustomStyle( const QVector<qreal> &style );

    /**
     * Selects the specified style.
     *
     * If the style was already added it gets selected. If the style was not added already
     * it gets temporary added and selected.
     *
     * @param style the style to display
     * @param dashes the dashes of the style if style == Qt::CustomDashLine
     */
    void setLineStyle( Qt::PenStyle style, const QVector<qreal> &dashes );

    /// Returns the current line style
    Qt::PenStyle lineStyle() const;
    /// Returns the dashes of the current line style
    QVector<qreal> lineDashes() const;

protected:
    void paintEvent( QPaintEvent *pe );

private:

    class Private;
    Private * const d;
};

#endif // _LINESTYLESELECTOR_H_
