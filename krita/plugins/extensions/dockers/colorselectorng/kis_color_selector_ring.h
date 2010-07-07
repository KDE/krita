#ifndef KIS_COLOR_SELECTOR_RING_H
#define KIS_COLOR_SELECTOR_RING_H

#include "kis_color_selector_component.h"

#include <QImage>

class KisColorSelectorRing : public KisColorSelectorComponent
{
    Q_OBJECT
public:
    explicit KisColorSelectorRing(KisColorSelectorBase *parent);
protected:
    void paintEvent(QPaintEvent *);
private:
    void paintCache();
    void colorCache();
    
    QImage m_pixelCache;
    const KoColorSpace* m_cachedColorSpace;
    int m_cachedSize;
    QList<QRgb> m_cachedColors;
};

#endif // KIS_COLOR_SELECTOR_RING_H
