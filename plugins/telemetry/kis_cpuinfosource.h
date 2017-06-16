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

#ifndef KISUSERFEEDBACK_CPUINFOSOURCE_H
#define KISUSERFEEDBACK_CPUINFOSOURCE_H

#include "abstractdatasource.h"
#include "kuserfeedbackcore_export.h"

namespace KisUserFeedback {

/*! Data source reporting the type and amount of CPUs.
 *
 *  The default telemetry mode for this source is Provider::DetailedSystemInformation.
 */
class CpuInfoSource : public KUserFeedback::AbstractDataSource {
public:
    CpuInfoSource();
    QString description() const override;
    QVariant data() override;
};
}

#endif // KISUSERFEEDBACK_CPUINFOSOURCE_H
