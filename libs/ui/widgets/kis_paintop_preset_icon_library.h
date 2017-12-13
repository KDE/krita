/* This file is part of the KDE project
 * Copyright (C) 2017 Wolthera van Hövell tot Westerflier
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
#ifndef KIS_PAINTOP_PRESET_ICON_LIBRARY_H
#define KIS_PAINTOP_PRESET_ICON_LIBRARY_H

#include <QWidget>
#include <QStandardItemModel>
#include "ui_wdgpreseticonlibrary.h"

class Ui_wdgpreseticonlibrary;

class KisPaintopPresetIconLibrary : public QWidget
{
    Q_OBJECT
public:
    KisPaintopPresetIconLibrary(QWidget *parent);
    ~KisPaintopPresetIconLibrary();

    Ui_wdgpreseticonlibrary *ui;

    QImage getImage();
public Q_SLOTS:
    QImage hueTransform(QImage img);
    void updateIcon();

private:
    QStandardItemModel *m_baseModel;
    QStandardItemModel *m_optionalModel;
    QImage m_background;
};

#endif // KIS_PAINTOP_PRESET_ICON_LIBRARY_H
