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

#include "DateVariable.h"
#include "FixedDateFormat.h"

#include <KoProperties.h>

DateVariable::DateVariable(DateType type)
    : KoVariable(),
    m_type(type),
    m_displayType(Custom),
    m_daysOffset(0),
    m_monthsOffset(0),
    m_yearsOffset(0),
    m_secsOffset(0)
{
    m_time = QDateTime::currentDateTime();
}

void DateVariable::setProperties(const KoProperties *props) {
    m_definition = props->stringProperty("definition");
    if (!props->stringProperty("time").isEmpty())
        m_time = QDateTime::fromString(props->stringProperty("time"), Qt::ISODate);
    if (props->intProperty("id") == Fixed)
        m_type = Fixed;
    else
        m_type = AutoUpdate;
    QString displayTypeProp = props->stringProperty("displayType", "custom");
    if (displayTypeProp == "custom")
        m_displayType = Custom;
    else if (displayTypeProp == "date")
        m_displayType = Date;
    else if (displayTypeProp == "time")
        m_displayType = Time;
    QString adjustTime = props->stringProperty("adjust");
    if (!adjustTime.isEmpty()) {
        m_daysOffset = 0;
        m_monthsOffset = 0;
        m_yearsOffset = 0;
        m_secsOffset = 0;
        int multiplier = 1;
        if (adjustTime.contains("-"))
            multiplier = -1;
        QString timePart, datePart;
        QStringList parts = adjustTime.mid(adjustTime.indexOf('P') + 1).split('T');
        datePart = parts[0];
        if (parts.size() > 1)
            timePart = parts[1];
        QRegExp rx("([0-9]+)([DHMSY])");
        int value;
        bool valueOk;
        if (!timePart.isEmpty()) {
            int pos = 0;
            while ((pos = rx.indexIn(timePart, pos)) != -1) {
                value = rx.cap(1).toInt(&valueOk);
                if (valueOk) {
                    if (rx.cap(2) == "H")
                        m_secsOffset += multiplier * 3600 * value;
                    else if (rx.cap(2) == "M")
                        m_secsOffset += multiplier * 60 * value;
                    else if (rx.cap(2) == "S")
                        m_secsOffset += multiplier * value;
                }
                pos += rx.matchedLength();
            }
        }
        if (!datePart.isEmpty()) {
            int pos = 0;
            while ((pos = rx.indexIn(datePart, pos)) != -1) {
                value = rx.cap(1).toInt(&valueOk);
                if (valueOk) {
                    if (rx.cap(2) == "Y")
                        m_yearsOffset += multiplier * value;
                    else if (rx.cap(2) == "M")
                        m_monthsOffset += multiplier * value;
                    else if (rx.cap(2) == "D")
                        m_daysOffset += multiplier * value;
                }
                pos += rx.matchedLength();
            }
        }
    }
    update();
}

QWidget *DateVariable::createOptionsWidget() {
    switch(m_type) {
        case Fixed:
            return new FixedDateFormat(this);
    }
    return 0;
}

void DateVariable::setDefinition(const QString &definition) {
    m_definition = definition;
    update();
}

void DateVariable::setSecsOffset(int offset) {
    m_secsOffset = offset;
    update();
}

void DateVariable::setDaysOffset(int offset) {
    m_daysOffset = offset;
    update();
}

void DateVariable::setMonthsOffset(int offset) {
    m_monthsOffset = offset;
    update();
}

void DateVariable::setYearsOffset(int offset) {
    m_yearsOffset = offset;
    update();
}

void DateVariable::update() {
    QDateTime target;
    switch(m_type) {
        case Fixed:
            target = m_time;
            break;
        case AutoUpdate:
            target = QDateTime::currentDateTime();
            break;
    }
    target = target.addSecs(m_secsOffset);
    target = target.addDays(m_daysOffset);
    target = target.addMonths(m_monthsOffset);
    target = target.addYears(m_yearsOffset);
    switch (m_displayType) {
        case Custom:
            setValue(target.toString(m_definition));
            break;
        case Time:
            setValue(target.time().toString(Qt::LocalDate));
            break;
        case Date:
            setValue(target.date().toString(Qt::LocalDate));
            break;
    }
}
