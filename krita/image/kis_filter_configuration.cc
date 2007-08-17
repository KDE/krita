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

KisFilterConfiguration::KisFilterConfiguration(const KisFilterConfiguration & rhs)
    : KisPropertiesConfiguration( rhs )
    , d( new Private )
{
    d->name = rhs.d->name;
    d->version = rhs.d->version;
}

KisFilterConfiguration::~KisFilterConfiguration()
{
    delete d;
}

void KisFilterConfiguration::toLegacyXML(QDomDocument& doc, QDomElement& root) const
{
    root.setAttribute( "name", d->name );
    root.setAttribute( "version", d->version );

    QMap<QString, QVariant> properties = getProperties();
    QMap<QString, QVariant>::Iterator it;
    for ( it = properties.begin(); it != properties.end(); ++it ) {
        QDomElement e = doc.createElement( "property" );
        e.setAttribute( "name", QString(it.key().toLatin1()) );
        QVariant v = it.value();
        e.setAttribute( "type", v.typeName() );
        QString s = v.toString();
        QDomText text = doc.createCDATASection(v.toString() ); // XXX: Unittest this!
        e.appendChild(text);
        root.appendChild(e);
    }
}

QString KisFilterConfiguration::toLegacyXML() const
{
    QDomDocument doc = QDomDocument("filterconfig");
    QDomElement root = doc.createElement( "filterconfig" );
    doc.appendChild( root );
    toLegacyXML(doc, root);
    return doc.toString();
}

void KisFilterConfiguration::fromLegacyXML(const QDomElement& e)
{
    d->name = e.attribute("name");
    d->version = e.attribute("version").toInt();

    QDomNode n = e.firstChild();


    while (!n.isNull()) {
        // We don't nest elements in filter configuration. For now...
        QDomElement e = n.toElement();
        QString name;
        QString type;
        QString value;

        if (!e.isNull()) {
            if (e.tagName() == "property") {
                name = e.attribute("name");
                type = e.attribute("type");
                value = e.text();
                // XXX Convert the variant pro-actively to the right type?
                setProperty(name, QVariant(value));
            }
        }
        n = n.nextSibling();
    }
}

void KisFilterConfiguration::fromLegacyXML(QString s)
{
    clearProperties();

    QDomDocument doc;
    doc.setContent( s );
    QDomElement e = doc.documentElement();
    fromLegacyXML(e);
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

struct KisFilterConfigurationFactory::Private
{
    QString name;
    qint32 version;
};

KisFilterConfigurationFactory::KisFilterConfigurationFactory(const QString & name, qint32 version) : d(new Private)
{
    d->name = name;
    d->version = version;
}

KisFilterConfigurationFactory::~KisFilterConfigurationFactory()
{
    delete d;
}

KisSerializableConfiguration* KisFilterConfigurationFactory::create(const QDomElement& e)
{
    KisFilterConfiguration* fc = new KisFilterConfiguration(d->name, d->version);
    fc->fromXML( e );
    return fc;
}
