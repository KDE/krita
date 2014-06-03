/* This file is part of the KDE project
 * Copyright (C) 2011 Sebastian Sauer <mail@dipe.org>
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

#ifndef USERVARIABLEOPTIONSWIDGET_H
#define USERVARIABLEOPTIONSWIDGET_H

#include <QWidget>

class QLineEdit;
class QComboBox;
class QPushButton;
class KoVariableManager;
class UserVariable;

class UserVariableOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UserVariableOptionsWidget(UserVariable *userVariable, QWidget *parent = 0);
    virtual ~UserVariableOptionsWidget();
private Q_SLOTS:
    void nameChanged();
    void typeChanged();
    void valueChanged();
    void newClicked();
    void deleteClicked();
private:
    KoVariableManager *variableManager();
    void updateNameEdit();

    UserVariable *userVariable;
    QComboBox *nameEdit;
    QComboBox *typeEdit;
    QLineEdit *valueEdit;
    QPushButton *newButton, *deleteButton;
};

#endif

