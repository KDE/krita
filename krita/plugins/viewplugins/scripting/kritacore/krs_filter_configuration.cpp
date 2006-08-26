/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "krs_filter_configuration.h"

#include <kis_filter_configuration.h>

using namespace Kross::KritaCore;

FilterConfiguration::FilterConfiguration(KisFilterConfiguration* fConfig)
    : QObject(), m_fConfig(fConfig)
{
    setObjectName("KritaFilterConfiguration");
}

FilterConfiguration::~FilterConfiguration()
{
}

void FilterConfiguration::setProperty(const QString& name, const QVariant& value)
{
    m_fConfig->setProperty(name, value);
}

const QVariant FilterConfiguration::getProperty(const QString& name)
{
    QVariant value;
    return m_fConfig->getProperty(name, value) ? value : QVariant();
}

void FilterConfiguration::fromXML(const QString& xml)
{
    m_fConfig->fromXML( xml );
}

#if 0
const QString FilterConfiguration::toXML()
{
    return m_fConfig->toXML();
}
#endif

#include "krs_filter_configuration.moc"
