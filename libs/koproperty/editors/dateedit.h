/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004  Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2012  Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef KPROPERTY_DATEEDIT_H
#define KPROPERTY_DATEEDIT_H

#include "koproperty/Factory.h"
#include <QDateEdit>


namespace KoProperty
{

class KOPROPERTY_EXPORT DateEdit : public QDateEdit
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue USER true)

public:
    DateEdit(const Property* prop, QWidget* parent);
    virtual ~DateEdit();

    QVariant value() const;

signals:
    void commitData(QWidget* editor);

public slots:
    void setValue(const QVariant& value);

protected:
    virtual void paintEvent(QPaintEvent* event);

protected slots:
    void onDateChanged();
};


class KOPROPERTY_EXPORT DateDelegate : public EditorCreatorInterface,
                                       public ValueDisplayInterface
{
public:
    DateDelegate();

    virtual QString displayTextForProperty(const Property* prop) const;

    virtual QWidget* createEditor(int type, QWidget* parent,
        const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

}

#endif
