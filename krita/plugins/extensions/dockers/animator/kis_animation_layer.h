#ifndef KIS_ANIMATION_LAYER_H
#define KIS_ANIMATION_LAYER_H

#include "kis_animation_layerbox.h"
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>

class KisAnimationLayer : public QWidget
{
    Q_OBJECT

public:
    KisAnimationLayer(KisAnimationLayerBox* parent = 0);

protected:
    void paintEvent(QPaintEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
    void onLayerNameEdited();

private:
    KisAnimationLayerBox *m_layerBox;
    QLabel* m_lblLayerName;
    QLineEdit* m_inputLayerName;
    QHBoxLayout* lay;
};

#endif // KIS_ANIMATION_LAYER_H
