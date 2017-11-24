/* This file is part of the KDE project
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2012 C. Boemann <cbo@boemann.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOCONFIGAUTHORPAGE_H
#define KOCONFIGAUTHORPAGE_H

#include <QWidget>
#include <QStyledItemDelegate>

#include "kritawidgets_export.h"

class KRITAWIDGETS_EXPORT KoConfigAuthorPage : public QWidget
{
    Q_OBJECT

public:
    KoConfigAuthorPage();
    ~KoConfigAuthorPage() override;

    void apply();

private Q_SLOTS:
    void profileChanged(int i);
    void addUser();
    void deleteUser();
    void addContactEntry();
    void removeContactEntry();

private:
    class Private;
    Private * const d;
};

/**
 * @brief The KoContactInfoDelegate class
 *
 * quick delegate subclass to allow a qcombobox for the type.
 */
class KoContactInfoDelegate : public QStyledItemDelegate
{
public:
    KoContactInfoDelegate(QWidget *parent, QStringList contactModes);
    ~KoContactInfoDelegate() override;

public:
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QStringList m_contactModes;
};


#endif // KOCONFIGAUTHORPAGE_H
