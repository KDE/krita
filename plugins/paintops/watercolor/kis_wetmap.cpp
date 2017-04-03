#include "kis_wetmap.h"
#include "KoColorSpaceRegistry.h"
#include "kis_sequential_iterator.h"
#include <QVector2D>

KisWetMap::KisWetMap()
{
    m_wetMap = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb16());
}

void KisWetMap::addWater(QPoint pos, qreal radius)
{
    QRect rect(pos - QPointF(radius, radius).toPoint(),
               pos + QPointF(radius, radius).toPoint());
    KisSequentialIterator it(m_wetMap, rect);

    do {
        qint16 *mydata = reinterpret_cast<qint16*>(it.rawData());
        mydata[0] = 255;

        QPoint place(it.x(), it.y());
        QVector2D vec(place - pos);

        vec.normalize();

        mydata[1] = 255 * vec.length();
        mydata[2] = 255 * vec.length();
    } while (it.nextPixel());
}

void KisWetMap::update()
{
    KisSequentialIterator it(m_wetMap, m_wetMap->exactBounds());

    do {
        qint16 *mydata = reinterpret_cast<qint16*>(it.rawData());
        if (mydata[0] > 0) {
            mydata[0]--;
        } else {
            mydata[1] = 0;
            mydata[2] = 0;
        }
    } while (it.nextPixel());
}

int KisWetMap::getWater(int x, int y)
{
    KisSequentialIterator it(m_wetMap, m_wetMap->exactBounds());
    while (it.x() != x && it.y() != y)
        it.nextPixel();

    qint16 *mydata = reinterpret_cast<qint16*>(it.rawData());

    return mydata[0];
}
