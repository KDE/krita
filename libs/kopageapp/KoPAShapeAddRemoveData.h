#ifndef KOPASHAPEADDREMOVEDATA_H
#define KOPASHAPEADDREMOVEDATA_H

#include <KoShapeAddRemoveData.h>

class KoPAPage;
class KoShapeLayer;

class KoPAShapeAddRemoveData : public KoShapeAddRemoveData
{
public:    
    KoPAShapeAddRemoveData( KoPAPage * activePage, KoShapeLayer * activeLayer );
    virtual ~KoPAShapeAddRemoveData() {}

    virtual KoShapeAddRemoveData * clone() const;

    void setPage( KoPAPage * activePage ) { m_activePage = activePage; } 
    KoPAPage * page() const { return m_activePage; }

    void setLayer( KoShapeLayer * activeLayer ) { m_activeLayer = activeLayer; }
    KoShapeLayer * layer() const { return m_activeLayer; }

private:    
    KoPAPage * m_activePage;
    KoShapeLayer * m_activeLayer;
};

#endif /* KOPASHAPEADDREMOVEDATA_H */
