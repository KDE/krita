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
#include "FixedDateFormat.h"
#include "DateVariable.h"

#include <QMenu>
#include <QAction>
#include <kicon.h>
#include <kglobal.h>
#include <klocale.h>

static void createTimeAction(QMenu *parent, const QString &title, const QString &data)
{
    QAction *action = new QAction(title, parent);
    action->setData(data);
    parent->addAction(action);
}

FixedDateFormat::FixedDateFormat(DateVariable *variable)
        : m_variable(variable),
        m_popup(0)
{
    widget.setupUi(this);

    widget.normalPage->layout()->setMargin(0);
    widget.customPage->layout()->setMargin(0);

    QStringList listDateFormat;
    listDateFormat << i18n("Locale date format");
    listDateFormat << i18n("Short locale date format");
    listDateFormat << i18n("Locale date & time format");
    listDateFormat << i18n("Short locale date & time format");
    listDateFormat << "dd/MM/yy";
    listDateFormat << "dd/MM/yyyy";
    listDateFormat << "MMM dd,yy";
    listDateFormat << "MMM dd,yyyy";
    listDateFormat << "dd.MMM.yyyy";
    listDateFormat << "MMMM dd, yyyy";
    listDateFormat << "ddd, MMM dd,yy";
    listDateFormat << "dddd, MMM dd,yy";
    listDateFormat << "MM-dd";
    listDateFormat << "yyyy-MM-dd";
    listDateFormat << "dd/yy";
    listDateFormat << "MMMM";
    listDateFormat << "yyyy-MM-dd hh:mm";
    listDateFormat << "dd.MMM.yyyy hh:mm";
    listDateFormat << "MMM dd,yyyy h:mm AP";
    listDateFormat << "yyyy-MM-ddThh:mm:ss"; // ISO 8601
    widget.formatList->addItems(listDateFormat);
    widget.customString->setText(variable->definition());

    int index = listDateFormat.indexOf(variable->definition());
    if (index >= 0) {
        widget.widgetStack->setCurrentWidget(widget.normalPage);
        widget.formatList->setItemSelected(widget.formatList->item(index), true);
    } else {
        widget.widgetStack->setCurrentWidget(widget.customPage);
        widget.custom->setChecked(true);
    }

    widget.formatButton->setIcon(KIcon("list-add"));

    connect(widget.custom, SIGNAL(stateChanged(int)), this, SLOT(customClicked(int)));
    connect(widget.formatList, SIGNAL(itemPressed(QListWidgetItem*)), this, SLOT(listClicked(QListWidgetItem*)));
    connect(widget.correction, SIGNAL(valueChanged(int)), this, SLOT(offsetChanged(int)));
    connect(widget.formatButton, SIGNAL(clicked()), this, SLOT(insertCustomButtonPressed()));
    connect(widget.customString, SIGNAL(textChanged(const QString&)), this, SLOT(customTextChanged(const QString&)));
}

void FixedDateFormat::customClicked(int state)
{
    if (state == Qt::Unchecked)
        widget.widgetStack->setCurrentWidget(widget.normalPage);
    else
        widget.widgetStack->setCurrentWidget(widget.customPage);
}

void FixedDateFormat::listClicked(QListWidgetItem *item)
{
    // TODO parse out the first two values...
    QString format;
    switch (widget.formatList->row(item)) {
    case 0: format = KGlobal::locale()->dateFormat(); break;
    case 1: format = KGlobal::locale()->dateFormatShort(); break;
    case 2:
        format = KGlobal::locale()->dateFormat() + ' ' + KGlobal::locale()->timeFormat();
        break;
    case 3:
        format = KGlobal::locale()->dateFormatShort() + ' ' + KGlobal::locale()->timeFormat();
        break;
    default:
        format = item->text();
    }
    m_variable->setDefinition(format);
    widget.customString->setText(format);
}

void FixedDateFormat::offsetChanged(int offset)
{
    m_variable->setDaysOffset(offset);
}

void FixedDateFormat::insertCustomButtonPressed()
{
    if (m_popup == 0) {
        m_popup = new QMenu(this);
        QMenu *day = new QMenu(i18n("Day"), m_popup);
        QMenu *month = new QMenu(i18n("Month"), m_popup);
        QMenu *year = new QMenu(i18n("Year"), m_popup);
        QMenu *hour = new QMenu(i18n("Hour"), m_popup);
        QMenu *minute = new QMenu(i18n("Minute"), m_popup);
        QMenu *second = new QMenu(i18n("Second"), m_popup);
        m_popup->addMenu(day);
        m_popup->addMenu(month);
        m_popup->addMenu(year);
        m_popup->addMenu(hour);
        m_popup->addMenu(minute);
        m_popup->addMenu(second);

        createTimeAction(day, i18n("Flexible Digits (1-31)"), "d");
        createTimeAction(day, i18n("2 Digits (01-31)"), "dd");
        createTimeAction(day, i18n("Abbreviated Name"), "ddd");
        createTimeAction(day, i18n("Long Name"), "dddd");
        createTimeAction(month, i18n("Flexible Digits (1-12)"), "M");
        createTimeAction(month, i18n("2 Digits (01-12)"), "MM");
        createTimeAction(month, i18n("Abbreviated Name"), "MMM");
        createTimeAction(month, i18n("Long Name"), "MMMM");
        createTimeAction(month, i18n("Possessive Abbreviated Name"), "PPP");
        createTimeAction(month, i18n("Possessive Long Name"), "PPPP");
        createTimeAction(year, i18n("2 Digits (01-99)"), "yy");
        createTimeAction(year, i18n("4 Digits"), "yyyy");
        createTimeAction(hour, i18n("Flexible Digits (1-23)"), "h");
        createTimeAction(hour, i18n("2 Digits (01-23)"), "hh");
        createTimeAction(minute, i18n("Flexible Digits (1-59)"), "m");
        createTimeAction(minute, i18n("2 Digits (01-59)"), "mm");
        createTimeAction(second, i18n("Flexible Digits (1-59)"), "s");
        createTimeAction(second, i18n("2 Digits (01-59)"), "sss");
        createTimeAction(m_popup, i18n("am/pm"), "ap");
        createTimeAction(m_popup, i18n("AM/PM"), "AP");
    }
    QPoint position = widget.formatButton->mapToGlobal(QPoint(0, widget.formatButton->height()));
    QAction *action = m_popup->exec(position);
    if (action)
        widget.customString->insert(qvariant_cast<QString>(action->data()));
}

void FixedDateFormat::customTextChanged(const QString& text)
{
    m_variable->setDefinition(text);

    if (widget.custom->isChecked()) {
        // altering the custom text will deselect the list item so the user can easilly switch
        // back by selecting one.
        QListWidgetItem * item = widget.formatList->currentItem();
        if (item) // deselect it.
            widget.formatList->setItemSelected(item, false);
    }
}

#include <FixedDateFormat.moc>
