/* This file is part of the KDE project
   Copyright (C) 2017 Alexey Kapustin <akapust1n@mail.ru>


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

#ifndef KISUSERFEEDBACK_ASSERTINFOSOURCE_H
#define KISUSERFEEDBACK_ASSERTINFOSOURCE_H

#include "abstractdatasource.h"
#include "kuserfeedbackcore_export.h"
#include <exception>

namespace UserFeedback {

/*! Data source reporting the assert info
 */
class TelemetryAssertInfoSource : public KUserFeedback::AbstractDataSource {
public:
    TelemetryAssertInfoSource();
    QString description() const override;
    QVariant data() override;
};
}

class NoFatalError : public std::exception {
public:
    NoFatalError() = default;
    virtual const char* what() const throw();
};

#endif // KISUSERFEEDBACK_ASSERTINFOSOURCE_H
