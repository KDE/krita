/* This file is part of the KDE project
   Copyright (c) 2003 Lukas Tinkl <lukas@kde.org>
   Copyright (c) 2003 David Faure <faure@kde.org>

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

#include "KoStyleStack.h"
#include "KoUnit.h"
#include "KoDom.h"
#include "KoXmlNS.h"

#include <kdebug.h>
//Added by qt3to4:
#include <Q3ValueList>

//#define DEBUG_STYLESTACK

KoStyleStack::KoStyleStack()
    : m_styleNSURI( KoXmlNS::style ), m_foNSURI( KoXmlNS::fo )
{
    clear();
}

KoStyleStack::KoStyleStack( const char* styleNSURI, const char* foNSURI )
    : m_propertiesTagName( "properties" ), m_styleNSURI( styleNSURI ), m_foNSURI( foNSURI )
{
    clear();
}

KoStyleStack::~KoStyleStack()
{
}

void KoStyleStack::clear()
{
    m_stack.clear();
#ifdef DEBUG_STYLESTACK
    kDebug(30003) << "clear!" << endl;
#endif
}

void KoStyleStack::save()
{
    m_marks.push( m_stack.count() );
#ifdef DEBUG_STYLESTACK
    kDebug(30003) << "save (level " << m_marks.count() << ") -> index " << m_stack.count() << endl;
#endif
}

void KoStyleStack::restore()
{
    Q_ASSERT( !m_marks.isEmpty() );
    int toIndex = m_marks.pop();
#ifdef DEBUG_STYLESTACK
    kDebug(30003) << "restore (level " << m_marks.count()+1 << ") -> to index " << toIndex << endl;
#endif
    Q_ASSERT( toIndex > -1 );
    Q_ASSERT( toIndex <= (int)m_stack.count() ); // If equal, nothing to remove. If greater, bug.
    for ( int index = (int)m_stack.count() - 1; index >= toIndex; --index )
        m_stack.pop_back();
}

void KoStyleStack::pop()
{
    Q_ASSERT( !m_stack.isEmpty() );
    m_stack.pop_back();
#ifdef DEBUG_STYLESTACK
    kDebug(30003) << "pop -> count=" << m_stack.count() << endl;
#endif
}

void KoStyleStack::push( const KoXmlElement& style )
{
    m_stack.append( style );
#ifdef DEBUG_STYLESTACK
    kDebug(30003) << "pushed " << style.attributeNS( m_styleNSURI, "name", QString::null ) << " -> count=" << m_stack.count() << endl;
#endif
}

bool KoStyleStack::hasAttribute( const QString& name, const QString& detail ) const
{
    QString fullName( name );
    if ( !detail.isEmpty() )
    {
        fullName += '-';
        fullName += detail;
    }
    Q3ValueList<KoXmlElement>::ConstIterator it = m_stack.end();
    while ( it != m_stack.begin() )
    {
        --it;
        KoXmlElement properties = (*it).namedItem( "style:"+m_propertiesTagName ).toElement();
        if ( properties.hasAttribute( name ) ||
             ( !detail.isEmpty() && properties.hasAttribute( fullName ) ) )
            return true;
    }
    return false;
}

QString KoStyleStack::attribute( const QString& name, const QString& detail ) const
{
    QString fullName( name );
    if ( !detail.isEmpty() )
    {
        fullName += '-';
        fullName += detail;
    }
    Q3ValueList<KoXmlElement>::ConstIterator it = m_stack.end();
    while ( it != m_stack.begin() )
    {
        --it;
        KoXmlElement properties = (*it).namedItem( "style:"+m_propertiesTagName ).toElement();
        if ( properties.hasAttribute( name ) )
            return properties.attribute( name );
        if ( !detail.isEmpty() && properties.hasAttribute( fullName ) )
            return properties.attribute( fullName );
    }
    return QString::null;
}

QString KoStyleStack::attributeNS( const char* nsURI, const char* name, const char* detail ) const
{
    QString fullName( name );
    if ( detail )
    {
        fullName += '-';
        fullName += detail;
    }
    Q3ValueList<KoXmlElement>::ConstIterator it = m_stack.end();
    while ( it != m_stack.begin() )
    {
        --it;
        KoXmlElement properties = KoDom::namedItemNS( *it, m_styleNSURI, m_propertiesTagName );
        if ( properties.hasAttributeNS( nsURI, name ) )
            return properties.attributeNS( nsURI, name, QString::null );
        if ( detail && properties.hasAttributeNS( nsURI, fullName ) )
            return properties.attributeNS( nsURI, fullName, QString::null );
    }
    return QString::null;
}

bool KoStyleStack::hasAttributeNS( const char* nsURI, const char* name, const char* detail ) const
{
    QString fullName( name );
    if ( detail )
    {
        fullName += '-';
        fullName += detail;
    }
    Q3ValueList<KoXmlElement>::ConstIterator it = m_stack.end();
    while ( it != m_stack.begin() )
    {
        --it;
        KoXmlElement properties = KoDom::namedItemNS( *it, m_styleNSURI, m_propertiesTagName );
        if ( properties.hasAttributeNS( nsURI, name ) ||
             ( detail && properties.hasAttributeNS( nsURI, fullName ) ) )
            return true;
    }
    return false;
}

// Font size is a bit special. "115%" applies to "the fontsize of the parent style".
// This can be generalized though (hasAttributeThatCanBePercentOfParent() ? :)
// Although, if we also add support for fo:font-size-rel here then it's not general anymore.
double KoStyleStack::fontSize() const
{
    const QString name = "font-size";
    double percent = 1;
    Q3ValueList<KoXmlElement>::ConstIterator it = m_stack.end(); // reverse iterator

    while ( it != m_stack.begin() )
    {
        --it;
        KoXmlElement properties = KoDom::namedItemNS( *it, m_styleNSURI, m_propertiesTagName ).toElement();
        if ( properties.hasAttributeNS( m_foNSURI, name ) ) {
            const QString value = properties.attributeNS( m_foNSURI, name, QString::null );
            if ( value.endsWith( "%" ) )
                percent *= value.left( value.length() - 1 ).toDouble() / 100.0;
            else
                return percent * KoUnit::parseValue( value ); // e.g. 12pt
        }
    }
    return 0;
}

bool KoStyleStack::hasChildNode(const QString & name) const
{
    Q3ValueList<KoXmlElement>::ConstIterator it = m_stack.end();
    while ( it != m_stack.begin() )
    {
        --it;
        KoXmlElement properties = (*it).namedItem( "style:"+m_propertiesTagName ).toElement();
        if ( !properties.namedItem( name ).isNull() )
            return true;
    }

    return false;
}

KoXmlElement KoStyleStack::childNode(const QString & name) const
{
    Q3ValueList<KoXmlElement>::ConstIterator it = m_stack.end();

    while ( it != m_stack.begin() )
    {
        --it;
        KoXmlElement properties = (*it).namedItem( "style:"+m_propertiesTagName ).toElement();
        if ( !properties.namedItem( name ).isNull() )
            return properties.namedItem( name ).toElement();
    }

    return KoXmlElement();          // a null element
}

bool KoStyleStack::hasChildNodeNS( const char* nsURI, const char* localName ) const
{
    Q3ValueList<KoXmlElement>::ConstIterator it = m_stack.end();
    while ( it != m_stack.begin() )
    {
        --it;
        KoXmlElement properties = KoDom::namedItemNS( *it, m_styleNSURI, m_propertiesTagName );
        if ( !KoDom::namedItemNS( properties, nsURI, localName ).isNull() )
            return true;
    }

    return false;
}

KoXmlElement KoStyleStack::childNodeNS( const char* nsURI, const char* localName) const
{
    Q3ValueList<KoXmlElement>::ConstIterator it = m_stack.end();

    while ( it != m_stack.begin() )
    {
        --it;
        KoXmlElement properties = KoDom::namedItemNS( *it, m_styleNSURI, m_propertiesTagName );
        KoXmlElement e = KoDom::namedItemNS( properties, nsURI, localName );
        if ( !e.isNull() )
            return e;
    }

    return KoXmlElement();          // a null element
}

bool KoStyleStack::isUserStyle( const KoXmlElement& e, const QString& family ) const
{
    if ( e.attributeNS( m_styleNSURI, "family", QString::null ) != family )
        return false;
    const KoXmlElement parent = e.parentNode().toElement();
    //kDebug(30003) << k_funcinfo << "tagName=" << e.tagName() << " parent-tagName=" << parent.tagName() << endl;
    return parent.localName() == "styles" /*&& parent.namespaceURI() == KoXmlNS::office*/;
}

QString KoStyleStack::userStyleName( const QString& family ) const
{
    Q3ValueList<KoXmlElement>::ConstIterator it = m_stack.end();
    while ( it != m_stack.begin() )
    {
        --it;
        //kDebug(30003) << k_funcinfo << (*it).attributeNS( m_styleNSURI, "name", QString::null) << endl;
        if ( isUserStyle( *it, family ) )
            return (*it).attributeNS( m_styleNSURI, "name", QString::null );
    }
    // Can this ever happen?
    return "Standard";
}

QString KoStyleStack::userStyleDisplayName( const QString& family ) const
{
    Q3ValueList<KoXmlElement>::ConstIterator it = m_stack.end();
    while ( it != m_stack.begin() )
    {
        --it;
        //kDebug(30003) << k_funcinfo << (*it).attributeNS( m_styleNSURI, "display-name") << endl;
        if ( isUserStyle( *it, family ) )
            return (*it).attributeNS( m_styleNSURI, "display-name", QString::null );
    }
    return QString::null; // no display name, this can happen since it's optional
}

void KoStyleStack::setTypeProperties( const char* typeProperties )
{
    m_propertiesTagName = typeProperties == 0 ? QByteArray( "properties" ) : ( QByteArray( typeProperties ) + "-properties" );
}
