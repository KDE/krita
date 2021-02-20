/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
