/* This file is part of the KDE project
 * Copyright (C) 2009 Carlos Licea <carlos.licea@kdemail.net>
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

#ifndef INSERTGUIDESTOOLOPTIONWIDGET_H
#define INSERTGUIDESTOOLOPTIONWIDGET_H

#include <ui_InsertGuidesToolOptionWidget.h>

#include <QWidget>

//This is the resulting transaction to be applied.
//NOTE: it is a class instead of a struct so to be able to forward include i
class GuidesTransaction
{
public:
    bool insertVerticalEdgesGuides;
    bool insertHorizontalEdgesGuides;
    bool erasePreviousGuides;
    int verticalGuides;
    int horizontalGuides;
};

class InsertGuidesToolOptionWidget : public QWidget
{
    Q_OBJECT
public:

    explicit InsertGuidesToolOptionWidget(QWidget *parent = 0);
    ~InsertGuidesToolOptionWidget();

Q_SIGNALS:
    void createGuides(GuidesTransaction *transaction);

private Q_SLOTS:
    void onCreateButtonClicked(bool checked);

private:
    Ui_InsertGuidesToolOptionWidget m_widget;
};

#endif
