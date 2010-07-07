#ifndef KIS_COLOR_SELECTOR_RING_H
#define KIS_COLOR_SELECTOR_RING_H

#include "kis_color_selector_component.h"

#include <QImage>
#include <QPixmap>

class KisColorSelectorRing : public KisColorSelectorComponent
{
    Q_OBJECT
public:
    explicit KisColorSelectorRing(KisColorSelectorBase *parent);
protected:
    void paintEvent(QPaintEvent *);
private:
    void paintCache();
    
    QPixmap m_cache;
    const KoColorSpace* m_cachedColorSpace;
};

#endif // KIS_COLOR_SELECTOR_RING_H
