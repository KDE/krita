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

#include "dateedit.h"

#include <koproperty/EditorDataModel.h>
// KDE
#include <KLocale>
#include <KGlobal>

using namespace KoProperty;


DateEdit::DateEdit(const Property* prop, QWidget* parent)
  : QDateEdit(parent)
{
    setFrame(false);
    setCalendarPopup(true);

    const QDate minDate = prop->option("min").toDate();
    if (minDate.isValid()) {
        setMinimumDate(minDate);
    }
    const QDate maxDate = prop->option("max").toDate();
    if (maxDate.isValid()) {
        setMaximumDate(maxDate);
    }

    connect(this, SIGNAL(dateChanged(QDate)), this, SLOT(onDateChanged()));
}

DateEdit::~DateEdit()
{
}

QVariant DateEdit::value() const
{
    return QVariant(date());
}

void DateEdit::setValue(const QVariant& value)
{
    blockSignals(true);
    setDate(value.toDate());
    blockSignals(false);
}

void DateEdit::onDateChanged()
{
    emit commitData(this);
}


DateDelegate::DateDelegate()
{
}

QString DateDelegate::displayTextForProperty(const Property* prop) const
{
    return KGlobal::locale()->formatDate(prop->value().toDate(), KLocale::ShortDate);
}

QWidget* DateDelegate::createEditor(int type, QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(type);
    Q_UNUSED(option);

    const EditorDataModel *editorModel
        = dynamic_cast<const EditorDataModel*>(index.model());
    Property *prop = editorModel->propertyForItem(index);

    return new DateEdit(prop, parent);
}
