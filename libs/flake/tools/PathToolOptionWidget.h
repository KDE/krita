/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#ifndef PATHTOOLOPTIONWIDGET_H
#define PATHTOOLOPTIONWIDGET_H

#include <QWidget>
#include <QFlags>

#include <ui_PathToolOptionWidgetBase.h>

class KoPathTool;

class PathToolOptionWidget : public QWidget
{
    Q_OBJECT
public:
    enum Type {
        PlainPath = 1,
        ParametricShape = 2
    };
    Q_DECLARE_FLAGS(Types, Type)

    explicit PathToolOptionWidget(KoPathTool *tool, QWidget *parent = 0);
    ~PathToolOptionWidget() override;

public Q_SLOTS:
    void setSelectionType(int type);

private:
    Ui::PathToolOptionWidgetBase widget;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PathToolOptionWidget::Types)

#endif
