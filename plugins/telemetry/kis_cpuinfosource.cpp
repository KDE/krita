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

#include "kis_cpuinfosource.h"

#include <QSysInfo>
#include <QThread>
#include <QVariant>
#include <Vc/cpuid.h>

using namespace KisUserFeedback;
using namespace KUserFeedback;


CpuInfoSource::CpuInfoSource()
    : AbstractDataSource(QStringLiteral("cpu"), Provider::DetailedSystemInformation)
{
}

QString CpuInfoSource::description() const
{
    return QObject::tr("The amount and type of CPUs in the system.");
}

QVariant CpuInfoSource::data()
{
    QVariantMap m;
    Vc_VERSIONED_NAMESPACE::CpuId::init();
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    m.insert(QStringLiteral("architecture"), QSysInfo::currentCpuArchitecture());
#else
    m.insert(QStringLiteral("architecture"), QStringLiteral("unknown"));
#endif
    m.insert(QStringLiteral("count"), QThread::idealThreadCount());
    m.insert(QStringLiteral("model"),Vc_VERSIONED_NAMESPACE::CpuId::processorModel());
    m.insert(QStringLiteral("family"),Vc_VERSIONED_NAMESPACE::CpuId::processorFamily());
    m.insert(QStringLiteral("isIntel"),Vc_VERSIONED_NAMESPACE::CpuId::isIntel());

    return m;
}
