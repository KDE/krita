/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_filter.h"

#include <kdebug.h>
#include <qdom.h>
#include <QString>

#include "kis_filter_registry.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "KoID.h"
#include "kis_types.h"
#include "kis_filter_config_widget.h"

struct KisFilterConfiguration::Private {
    QString name;
    qint32 version;
    QBitArray channelFlags;
};

KisFilterConfiguration::KisFilterConfiguration(const QString & name, qint32 version)
    : d(new Private)
{
 d->name = name;
 d->version = version;
}

KisFilterConfiguration::KisFilterConfiguration(const KisFilterConfiguration & rhs) : d(new Private)
{
    d->name = rhs.d->name;
    d->version = rhs.d->version;
}

void KisFilterConfiguration::toLegacyXML(QDomDocument& doc, QDomElement& root) const
{
    root.setAttribute( "name", d->name );
    root.setAttribute( "version", d->version );
    
    KisSerializableConfiguration::toLegacyXML(doc, root);
}

QString KisFilterConfiguration::toLegacyXML() const
{
    return KisSerializableConfiguration::toLegacyXML();
}

void KisFilterConfiguration::fromLegacyXML(const QDomElement& e)
{
    d->name = e.attribute("name");
    d->version = e.attribute("version").toInt();
    KisSerializableConfiguration::fromLegacyXML(e);
}

void KisFilterConfiguration::fromLegacyXML(QString str)
{
    return KisSerializableConfiguration::fromLegacyXML( str);
}

const QString & KisFilterConfiguration::name() const
{
    return d->name;
}

qint32 KisFilterConfiguration::version() const
{
    return d->version;
}

bool KisFilterConfiguration::isCompatible(const KisPaintDeviceSP) const
{
    return true;
}

QBitArray KisFilterConfiguration::channelFlags()
{
    return d->channelFlags;
}

void KisFilterConfiguration::setChannelFlags(QBitArray channelFlags)
{
    d->channelFlags = channelFlags;
}
