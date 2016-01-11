/* This file is part of the KDE project
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#ifndef ARTISTICTEXTSHAPEONPATHWIDGET_H
#define ARTISTICTEXTSHAPEONPATHWIDGET_H

#include <QWidget>

namespace Ui
{
class ArtisticTextShapeOnPathWidget;
}

class ArtisticTextTool;

class ArtisticTextShapeOnPathWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ArtisticTextShapeOnPathWidget(ArtisticTextTool *tool, QWidget *parent = 0);
    ~ArtisticTextShapeOnPathWidget();

public Q_SLOTS:
    void updateWidget();

Q_SIGNALS:
    /// triggered whenever the start offset has changed
    void offsetChanged(int);

private:
    Ui::ArtisticTextShapeOnPathWidget *ui;
    ArtisticTextTool *m_textTool;
};

#endif // ARTISTICTEXTSHAPEONPATHWIDGET_H
