/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Martin Pfeiffer <hubipete@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KOZOOMTOOLWIDGET_H
#define KOZOOMTOOLWIDGET_H

#include <QWidget>
#include <QPixmap>
#include "ui_KoZoomToolWidget.h"

class KoZoomTool;

class KoZoomToolWidget : public QWidget, Ui::ZoomToolWidget
{
    Q_OBJECT
public:
    explicit KoZoomToolWidget(KoZoomTool* tool, QWidget *parent = 0);
    ~KoZoomToolWidget() override;

private Q_SLOTS:
    void changeZoomMode();
private:
    KoZoomTool *m_tool;
};

#endif
