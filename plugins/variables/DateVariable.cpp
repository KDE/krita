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
    m_offset(0)
{
    m_time = QDateTime::currentDateTime();
}

void DateVariable::setProperties(const KoProperties *props) {
    m_definition = qvariant_cast<QString> (props->getProperty("definition"));
    m_offset = props->getProperty("offset").toInt();
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

void DateVariable::setOffset(int offset) {
    m_offset = offset;
    update();
}

void DateVariable::update() {
    switch(m_type) {
        case Fixed:
            setValue(m_time.addDays(m_offset).toString(m_definition));
            break;
    }
}
