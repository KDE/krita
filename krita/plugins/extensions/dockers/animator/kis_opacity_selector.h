#ifndef KIS_OPACITY_SELECTOR_H
#define KIS_OPACITY_SELECTOR_H

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QList>

class KisOpacitySelector : public QGraphicsItem
{
public:
    KisOpacitySelector(int x, int y, int width, int height, QGraphicsScene* scene = 0, int frames = 0);
    ~KisOpacitySelector();
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setOpacityValue(QList<int> l);
    QList<int>* getOpacityValues();
    void setOpacityValue(int frame, int val);
private:
    int m_width, m_height, m_x, m_y, m_frames;
    QList<int> *m_opacityValues;
};

#endif // KIS_OPACITY_SELECTOR_H
