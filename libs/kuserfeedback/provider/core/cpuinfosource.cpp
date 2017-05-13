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

#include "cpuinfosource.h"

#include <QSysInfo>
#include <QThread>
#include <QVariant>

using namespace UserFeedback;

CpuInfoSource::CpuInfoSource()
    : AbstractDataSource(QStringLiteral("cpu"))
{
}

QString CpuInfoSource::description() const
{
    return tr("The amount and type of CPUs in the system.");
}

QVariant CpuInfoSource::data()
{
    QVariantMap m;
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    m.insert(QStringLiteral("architecture"), QSysInfo::currentCpuArchitecture());
#else
    m.insert(QStringLiteral("architecture"), QStringLiteral("unknown"));
#endif
    m.insert(QStringLiteral("count"), QThread::idealThreadCount());
    return m;
}
