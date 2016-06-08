/* This file is part of the KDE project
 * Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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
#ifndef DEFAULTTOOLWIDGET_H
#define DEFAULTTOOLWIDGET_H

#include <ui_DefaultToolWidget.h>
#include <KoFlake.h>

#include <QWidget>

class KoInteractionTool;

class DefaultToolWidget : public QWidget, Ui::DefaultToolWidget
{
    Q_OBJECT
public:
    explicit DefaultToolWidget(KoInteractionTool *tool, QWidget *parent = 0);

    /// Sets the unit used by the unit aware child widgets
    void setUnit(const KoUnit &unit);

private Q_SLOTS:
    void positionSelected(KoFlake::Position position);
    void updatePosition();
    void positionHasChanged();
    void updateSize();
    void sizeHasChanged();
    void resourceChanged(int key, const QVariant &res);
    void aspectButtonToggled(bool keepAspect);

private:
    KoInteractionTool *m_tool;
    bool m_blockSignals;
};

#endif
