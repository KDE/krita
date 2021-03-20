/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
