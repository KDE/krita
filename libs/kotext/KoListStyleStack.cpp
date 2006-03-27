#include "KoListStyleStack.h"
#include <KoDom.h>
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

void KoListStyleStack::push( const QDomElement& style )
{
    m_stack.push( style );
}

void KoListStyleStack::setInitialLevel( int initialLevel )
{
    Q_ASSERT( m_stack.isEmpty() );
    m_initialLevel = initialLevel;
}

QDomElement KoListStyleStack::currentListStyle() const
{
    Q_ASSERT( !m_stack.isEmpty() );
    return m_stack.top();
}

QDomElement KoListStyleStack::currentListStyleProperties() const
{
    QDomElement style = currentListStyle();
    return KoDom::namedItemNS( style, KoXmlNS::style, "list-level-properties" );
}

QDomElement KoListStyleStack::currentListStyleTextProperties() const
{
    QDomElement style = currentListStyle();
    return KoDom::namedItemNS( style, KoXmlNS::style, "text-properties" );
}
