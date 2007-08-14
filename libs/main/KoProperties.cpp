/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KoProperties.h"

#include <qdom.h>
#include <QString>

class KoProperties::Private {
public:
    QMap<QString, QVariant> properties;
};

KoProperties::KoProperties()
    : d(new Private())
{
}

KoProperties::KoProperties(const KoProperties & rhs)
    : d(new Private())
{
    d->properties = rhs.d->properties;
}

KoProperties::~KoProperties() {
    delete d;
}

QMapIterator<QString, QVariant> KoProperties::propertyIterator() const
{
    return QMapIterator<QString, QVariant>( d->properties );
}


bool KoProperties::isEmpty() const
{
    return d->properties.isEmpty();
}

bool KoProperties::load(const QString & s)
{

    QDomDocument doc;

    if ( !doc.setContent( s ) )
        return false;
    d->properties.clear();

    QDomElement e = doc.documentElement();
    QDomNode n = e.firstChild();

    while (!n.isNull()) {
        // We don't nest elements.
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            if (e.tagName() == "property") {
                const QString name = e.attribute("name");
                const QString type = e.attribute("type");
                const QString value = e.text();
                QDataStream in( value.toAscii() );
                QVariant v;
                in >> v;
                d->properties[name] = v;
            }
        }
        n = n.nextSibling();
    }

    return true;
}

QString KoProperties::store( const QString & s )
{
    QDomDocument doc = QDomDocument( s );
    QDomElement root = doc.createElement( s );

    doc.appendChild( root );

    QMap<QString, QVariant>::Iterator it;
    for ( it = d->properties.begin(); it != d->properties.end(); ++it ) {
        QDomElement e = doc.createElement( "property" );
        e.setAttribute( "name", QString(it.key().toLatin1()) );
        QVariant v = it.value();
        e.setAttribute( "type", v.typeName() );

        QByteArray bytes;
        QDataStream out( &bytes, QIODevice::WriteOnly );
        out << v;
        QDomText text = doc.createCDATASection( QString::fromAscii( bytes, bytes.size() ) );
        e.appendChild(text);
        root.appendChild(e);
    }

    return doc.toString();

}

QString KoProperties::store()
{
    // Legacy...
    return store( "filterconfig" );
}


void KoProperties::setProperty(const QString & name, const QVariant & value)
{
    // If there's an existing value for this name already, replace it.
    d->properties.insert( name, value );
}

bool KoProperties::property(const QString & name, QVariant & value) const
{
   QMap<QString, QVariant>::const_iterator it = d->properties.find( name );
   if ( it == d->properties.end() ) {
       return false;
   }
   else {
       value = *it;
       return true;
   }
}

QVariant KoProperties::property(const QString & name) const
{
    return d->properties.value( name, QVariant() );
}


int KoProperties::intProperty(const QString & name, int def) const
{
    const QVariant v = property(name);
    if (v.isValid())
        return v.toInt();
    else
        return def;

}

double KoProperties::doubleProperty(const QString & name, double def) const
{
    const QVariant v = property(name);
    if (v.isValid())
        return v.toDouble();
    else
        return def;
}

bool KoProperties::boolProperty(const QString & name, bool def) const
{
    const QVariant v = property(name);
    if (v.isValid())
        return v.toBool();
    else
        return def;
}

QString KoProperties::stringProperty(const QString & name, const QString & def) const
{
    const QVariant v = property(name);
    if (v.isValid())
        return v.toString();
    else
        return def;
}

bool KoProperties::contains( const QString & key ) const
{
    return d->properties.contains( key );
}

QVariant KoProperties::value( const QString & key ) const
{
    return d->properties.value( key );
}

