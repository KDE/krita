/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QObject>

class Constants : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int AnimationDuration READ animationDuration CONSTANT)
    Q_PROPERTY(int GridRows READ gridRows NOTIFY gridSizeChanged)
    Q_PROPERTY(int GridColumns READ gridColumns CONSTANT)
    Q_PROPERTY(bool IsLandscape READ isLandscape NOTIFY gridSizeChanged)
    Q_PROPERTY(qreal DefaultMargin READ defaultMargin NOTIFY gridSizeChanged)
    Q_PROPERTY(qreal GridHeight READ gridHeight NOTIFY gridSizeChanged)
    Q_PROPERTY(qreal GridWidth READ gridWidth NOTIFY gridSizeChanged)
    Q_PROPERTY(qreal ToolbarButtonSize READ toolbarButtonSize NOTIFY gridSizeChanged)
    Q_PROPERTY(qreal SmallFontSize READ smallFontSize NOTIFY gridSizeChanged)
    Q_PROPERTY(qreal DefaultFontSize READ defaultFontSize NOTIFY gridSizeChanged)
    Q_PROPERTY(qreal LargeFontSize READ largeFontSize NOTIFY gridSizeChanged)
    Q_PROPERTY(qreal HugeFontSize READ hugeFontSize NOTIFY gridSizeChanged)

public:
    Constants(QObject* parent = 0);

    int animationDuration() const;
    qreal gridHeight() const;
    qreal gridWidth() const;
    qreal toolbarButtonSize() const;
    int gridRows() const;
    int gridColumns() const;
    qreal defaultMargin() const;
    qreal smallFontSize() const;
    qreal defaultFontSize() const;
    qreal largeFontSize() const;
    qreal hugeFontSize() const;
    bool isLandscape() const;

    Q_INVOKABLE void setGridWidth(qreal width);
    Q_INVOKABLE void setGridHeight(qreal height);

Q_SIGNALS:
    void gridSizeChanged();

private:
    qreal m_gridWidth;
    qreal m_gridHeight;
    qreal m_toolbarButtonSize;
};

#endif // CONSTANTS_H
