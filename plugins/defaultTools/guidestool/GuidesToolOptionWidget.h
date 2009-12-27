/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#ifndef GUIDESTOOLOPTIONWIDGET_H
#define GUIDESTOOLOPTIONWIDGET_H

#include "ui_GuidesToolOptionWidget.h"

#include <QtGui/QWidget>

class GuidesToolOptionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GuidesToolOptionWidget(QWidget *parent = 0);
    ~GuidesToolOptionWidget();

    /// Sets horizontal guide lines
    void setHorizontalGuideLines(const QList<qreal> &lines);

    /// Sets vertical guide lines
    void setVerticalGuideLines(const QList<qreal> &lines);

     /// Returns the list of horizontal guide lines.
    QList<qreal> horizontalGuideLines() const;

     /// Returns the list of vertical guide lines.
    QList<qreal> verticalGuideLines() const;

    /// Returns the current selected lines orientation
    Qt::Orientation orientation() const;

    /// Sets the current selected lines orientation
    void setOrientation(Qt::Orientation orientation);

    /// Selects the given guide line
    void selectGuideLine(Qt::Orientation orientation, int index);

    /// Sets the unit to be displayed
    void setUnit(const KoUnit &unit);

signals:
    /// Emitted whenever a specific guide line was selected
    void guideLineSelected(Qt::Orientation orientation, int index);
    /// Emitted whenever a guide line with the given orientation has changed
    void guideLinesChanged(Qt::Orientation orientation);

private slots:
    void updateList(int orientation);
    void updatePosition(int index);
    void positionChanged(qreal position);
    void removeLine();
    void addLine();

private:
    Ui_GuidesToolOptionWidget widget;
    QList<qreal> m_hGuides;
    QList<qreal> m_vGuides;
    KoUnit m_unit;
};

#endif // GUIDESTOOLOPTIONWIDGET_H
