/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#ifndef FIXEDDATEFORMAT_H
#define FIXEDDATEFORMAT_H

#include <QWidget>

#include <ui_FixedDateFormat.h>

class DateVariable;
class QListWidgetItem;
class QMenu;

class FixedDateFormat : public QWidget
{
    Q_OBJECT
public:
    explicit FixedDateFormat(DateVariable *variable);

private slots:
    void customClicked(int state);
    void listClicked(QListWidgetItem *item);
    void offsetChanged(int offset);
    void insertCustomButtonPressed();
    void customTextChanged(const QString& text);

private:
    Ui::FixedDateFormat widget;
    DateVariable *m_variable;
    QMenu *m_popup;
};

#endif
