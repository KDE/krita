#ifndef KIS_OPACITY_SELECTOR_VIEW_H
#define KIS_OPACITY_SELECTOR_VIEW_H
#include "kis_opacity_selector.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QSize>
#include <QList>

class KisOpacitySelectorView : public QGraphicsView
{
    Q_OBJECT
public:
    KisOpacitySelectorView(QWidget* parent);
    ~KisOpacitySelectorView();
    QGraphicsScene *m_opacitySelectorScene;
    KisOpacitySelector* m_opacitySelector;
    QSize sizeHint() const;
    void init();

private:
    int m_numberOfFrames;

protected:
    void mousePressEvent(QMouseEvent *event);

public slots:
    void setNumberOfFrames(int val);

};

#endif // KIS_OPACITY_SELECTOR_VIEW_H
