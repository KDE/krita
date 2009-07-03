/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOPABACKGROUNDTOOLWIDGET_H
#define KOPABACKGROUNDTOOLWIDGET_H

#include <QWidget>

#include "ui_BackgroundToolWidget.h"

class KoPABackgroundTool;

class KoPABackgroundToolWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KoPABackgroundToolWidget( KoPABackgroundTool *tool, QWidget *parent = 0 );
    virtual ~KoPABackgroundToolWidget();

public slots:
    void slotActivePageChanged();

private slots:
    void setBackgroundImage();
    void useMasterBackground( int state );
    void displayMasterShapes( int state );

private:
    Ui::BackgroundToolWidget widget;
    KoPABackgroundTool * m_tool;
};

#endif /* KOPABACKGROUNDTOOLWIDGET_H */
