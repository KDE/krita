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
#include "kis_serializable_configuration.h"

#include <kdebug.h>
#include <qdom.h>
#include <QString>

#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "KoID.h"
#include "kis_types.h"

struct KisSerializableConfiguration::Private {
  QMap<QString, QVariant> properties;
};

KisSerializableConfiguration::KisSerializableConfiguration() : d(new Private)
{
  
}

KisSerializableConfiguration::KisSerializableConfiguration(const KisSerializableConfiguration & rhs) : d(new Private(*rhs.d))
{
}

void KisSerializableConfiguration::fromLegacyXML(const QString & s )
{
    d->properties.clear();

    QDomDocument doc;
    doc.setContent( s );
    QDomElement e = doc.documentElement();
    fromLegacyXML(e);
}

void KisSerializableConfiguration::fromLegacyXML(const QDomElement& e)
{
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
                d->properties[name] = QVariant(value);
            }
        }
        n = n.nextSibling();
    }
    //dump();
}

void KisSerializableConfiguration::toLegacyXML(QDomDocument& doc, QDomElement& root) const
{
    QMap<QString, QVariant>::Iterator it;
    for ( it = d->properties.begin(); it != d->properties.end(); ++it ) {
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

QString KisSerializableConfiguration::toLegacyXML() const
{
    QDomDocument doc = QDomDocument("filterconfig");
    QDomElement root = doc.createElement( "filterconfig" );
    doc.appendChild( root );
    toLegacyXML(doc, root);
    return doc.toString();
}

void KisSerializableConfiguration::setProperty(const QString & name, const QVariant & value)
{
    if ( d->properties.find( name ) == d->properties.end() ) {
        d->properties.insert( name, value );
    }
    else {
        d->properties[name] = value;
    }
}

bool KisSerializableConfiguration::getProperty(const QString & name, QVariant & value) const
{
   if ( d->properties.find( name ) == d->properties.end() ) {
       return false;
   }
   else {
       value = d->properties[name];
       return true;
   }
}

QVariant KisSerializableConfiguration::getProperty(const QString & name) const
{
    if ( d->properties.find( name ) == d->properties.end() ) {
        return QVariant();
    }
    else {
        return d->properties[name];
    }
}


int KisSerializableConfiguration::getInt(const QString & name, int def) const
{
    QVariant v = getProperty(name);
    if (v.isValid())
        return v.toInt();
    else
        return def;

}

double KisSerializableConfiguration::getDouble(const QString & name, double def) const
{
    QVariant v = getProperty(name);
    if (v.isValid())
        return v.toDouble();
    else
        return def;
}

bool KisSerializableConfiguration::getBool(const QString & name, bool def) const
{
    QVariant v = getProperty(name);
    if (v.isValid())
        return v.toBool();
    else
        return def;
}

QString KisSerializableConfiguration::getString(const QString & name, const QString & def) const
{
    QVariant v = getProperty(name);
    if (v.isValid())
        return v.toString();
    else
        return def;
}

void KisSerializableConfiguration::dump()
{
    QMap<QString, QVariant>::Iterator it;
    for ( it = d->properties.begin(); it != d->properties.end(); ++it ) {
    }

}

QMap<QString, QVariant> KisSerializableConfiguration::getProperties() const
{
    return d->properties;
}

KisSerializableConfigurationFactory::~KisSerializableConfigurationFactory()
{
}
