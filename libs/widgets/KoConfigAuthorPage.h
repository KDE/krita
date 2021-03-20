/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2002 Laurent Montel <lmontel@mandrakesoft.com>
   SPDX-FileCopyrightText: 2006-2007 Jan Hambrecht <jaham@gmx.net>
   SPDX-FileCopyrightText: 2012 C. Boemann <cbo@boemann.dk>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
