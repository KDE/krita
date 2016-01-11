/* This file is part of the KDE project
 * Copyright (C) 2011 Ganesh Paramasivam <ganesh@crystalfab.com>
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
#ifndef __CHANGE_TRACKING_OPTIONS_WIDGET_H__
#define __CHANGE_TRACKING_OPTIONS_WIDGET_H__

#include <ui_ChangeTrackingOptionsWidget.h>
#include <TextTool.h>

class ChangeTrackingOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChangeTrackingOptionsWidget(TextTool *tool, QWidget *parent = 0);

private Q_SLOTS:
    void recordChangesChanged(int isChecked);
    void showChangesChanged(int isChecked);
    void configureSettingsPressed();

public Q_SLOTS:
    void toggleShowChanges(bool on);
    void toggleRecordChanges(bool on);

Q_SIGNALS:
    void doneWithFocus();

private:
    Ui::ChangeTrackingOptions widget;
    TextTool *m_tool;
};

#endif

