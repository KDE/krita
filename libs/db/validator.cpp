/* This file is part of the KDE project
   Copyright (C) 2004, 2006 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "validator.h"
#include <KLocale>

namespace KexiDB
{
class Validator::Private
{
public:
    Private()
            : acceptsEmptyValue(false) {
    }
    bool acceptsEmptyValue;
};
}

//-----------------------------------------------------------

using namespace KexiDB;

Validator::Validator(QObject * parent)
        : QValidator(parent)
        , d(new Private)
{
}

Validator::~Validator()
{
    delete d;
}

Validator::Result Validator::check(const QString &valueName, const QVariant& v,
                                   QString &message, QString &details)
{
    if (v.isNull() || (v.type() == QVariant::String && v.toString().isEmpty())) {
        if (!d->acceptsEmptyValue) {
            message = Validator::msgColumnNotEmpty().arg(valueName);
            return Error;
        }
        return Ok;
    }
    return internalCheck(valueName, v, message, details);
}

Validator::Result Validator::internalCheck(const QString & /*valueName*/,
        const QVariant& /*v*/, QString & /*message*/, QString & /*details*/)
{
    return Error;
}

QValidator::State Validator::validate(QString & , int &) const
{
    return QValidator::Acceptable;
}

void Validator::setAcceptsEmptyValue(bool set)
{
    d->acceptsEmptyValue = set;
}

bool Validator::acceptsEmptyValue() const
{
    return d->acceptsEmptyValue;
}

const QString Validator::msgColumnNotEmpty()
{
    return I18N_NOOP("\"%1\" value has to be entered.");
}

