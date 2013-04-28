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

#include "datetimeedit.h"

#include <koproperty/EditorDataModel.h>
// Qt
#include <QLocale>

using namespace KoProperty;


DateTimeEdit::DateTimeEdit(const Property* prop, QWidget* parent)
  : QDateTimeEdit(parent)
{
    Q_UNUSED(prop);

    setFrame(false);
    setCalendarPopup(true);

    const QDateTime minDateTime = prop->option("min").toDateTime();
    if (minDateTime.isValid()) {
        setMinimumDateTime(minDateTime);
    }
    const QDateTime maxDateTime = prop->option("max").toDateTime();
    if (maxDateTime.isValid()) {
        setMaximumDateTime(maxDateTime);
    }

    connect(this, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(onDateTimeChanged()));
}

DateTimeEdit::~DateTimeEdit()
{
}

QVariant DateTimeEdit::value() const
{
    return QVariant(dateTime());
}

void DateTimeEdit::setValue(const QVariant& value)
{
    blockSignals(true);
    setDateTime(value.toDateTime());
    blockSignals(false);
}

void DateTimeEdit::paintEvent(QPaintEvent* event)
{
    QDateTimeEdit::paintEvent(event);
    Factory::paintTopGridLine(this);
}


void DateTimeEdit::onDateTimeChanged()
{
    emit commitData(this);
}


//! @todo Port to KLocale, be inspired by KexiDateTimeTableEdit (with Kexi*Formatter)
DateTimeDelegate::DateTimeDelegate()
{
}

QString DateTimeDelegate::displayTextForProperty(const Property* prop) const
{
    const QLocale locale;
    const QString defaultDateTimeFormat = locale.dateTimeFormat(QLocale::ShortFormat);
    return prop->value().toDateTime().toString(defaultDateTimeFormat);
}

QWidget* DateTimeDelegate::createEditor(int type, QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(type);
    Q_UNUSED(option);

    const EditorDataModel *editorModel
        = dynamic_cast<const EditorDataModel*>(index.model());
    Property *prop = editorModel->propertyForItem(index);

    return new DateTimeEdit(prop, parent);
}
