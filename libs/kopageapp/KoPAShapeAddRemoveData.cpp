#include "KoPAShapeAddRemoveData.h"

KoPAShapeAddRemoveData::KoPAShapeAddRemoveData( KoPAPage * activePage, KoShapeLayer * activeLayer )
: m_activePage( activePage )    
, m_activeLayer( activeLayer )
{
}

KoShapeAddRemoveData * KoPAShapeAddRemoveData::clone() const
{
    return new KoPAShapeAddRemoveData( *this );
}
