/*
    Copyright (C) 2017 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef USERFEEDBACK_CPUINFOSOURCE_H
#define USERFEEDBACK_CPUINFOSOURCE_H

#include "userfeedbackcore_export.h"
#include "abstractdatasource.h"

namespace UserFeedback {

/*! Data source reporting the type and amount of CPUs. */
class USERFEEDBACKCORE_EXPORT CpuInfoSource : public AbstractDataSource
{
    Q_DECLARE_TR_FUNCTIONS(UserFeedback::CpuInfoSource)
public:
    CpuInfoSource();
    QString description() const override;
    QVariant data() override;
};

}

#endif // USERFEEDBACK_CPUINFOSOURCE_H
