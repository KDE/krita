/*
 *    This file is part of the KDE project
 *    Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *    Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
 *    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *    Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
 *    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 */

#ifndef KOTAGFILTERWIDGET_H
#define KOTAGFILTERWIDGET_H

#include <QWidget>

class KoTagFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KoTagFilterWidget(QWidget* parent);
    ~KoTagFilterWidget() override;
    void allowSave(bool allow);
    void clear();
Q_SIGNALS:
    void filterTextChanged(const QString &filterText);
    void saveButtonClicked();
private Q_SLOTS:
    void onTextChanged(const QString &lineEditText);
    void onSaveButtonClicked();
private:
    class Private;
    Private* const d;
};

#endif // KOTAGFILTERWIDGET_H
