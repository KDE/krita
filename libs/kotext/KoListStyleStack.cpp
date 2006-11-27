#include "KoListStyleStack.h"
#include <KoDom.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>

KoListStyleStack::KoListStyleStack()
    : m_initialLevel( 0 )
{
}

KoListStyleStack::~KoListStyleStack()
{
}

void KoListStyleStack::pop()
{
    m_stack.pop();
}

void KoListStyleStack::push( const KoXmlElement& style )
{
    m_stack.push( style );
}

void KoListStyleStack::setInitialLevel( int initialLevel )
{
    Q_ASSERT( m_stack.isEmpty() );
    m_initialLevel = initialLevel;
}

KoXmlElement KoListStyleStack::currentListStyle() const
{
    Q_ASSERT( !m_stack.isEmpty() );
    return m_stack.top();
}

KoXmlElement KoListStyleStack::currentListStyleProperties() const
{
    KoXmlElement style = currentListStyle();
    return KoDom::namedItemNS( style, KoXmlNS::style, "list-level-properties" );
}

KoXmlElement KoListStyleStack::currentListStyleTextProperties() const
{
    KoXmlElement style = currentListStyle();
    return KoDom::namedItemNS( style, KoXmlNS::style, "text-properties" );
}
