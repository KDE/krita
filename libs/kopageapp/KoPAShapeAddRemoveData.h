#ifndef KOPASHAPEADDREMOVEDATA_H
#define KOPASHAPEADDREMOVEDATA_H

#include <KoShapeAddRemoveData.h>

class KoPAPage;
class KoLayerShape;

class KoPAShapeAddRemoveData : public KoShapeAddRemoveData
{
public:    
    KoPAShapeAddRemoveData( KoPAPage * activePage, KoLayerShape * activeLayer );
    virtual ~KoPAShapeAddRemoveData() {}

    virtual KoShapeAddRemoveData * clone() const;

    void setPage( KoPAPage * activePage ) { m_activePage = activePage; } 
    KoPAPage * page() const { return m_activePage; }

    void setLayer( KoLayerShape * activeLayer ) { m_activeLayer = activeLayer; }
    KoLayerShape * layer() const { return m_activeLayer; }

private:    
    KoPAPage * m_activePage;
    KoLayerShape * m_activeLayer;
};

#endif /* KOPASHAPEADDREMOVEDATA_H */
