#ifndef KIS_ANIMATION_LAYERBOX_H
#define KIS_ANIMATION_LAYERBOX_H

#include <QWidget>
#include <QPointer>
class KisNodeModel;
class KisNodeManager;
class KisTimeline;
#include <kis_types.h>
#include <QToolButton>

class KisAnimationLayerBox : public QWidget
{
    Q_OBJECT
public:
    KisAnimationLayerBox(KisTimeline* parent = 0);
    void makeConnections();

protected:
    void paintEvent(QPaintEvent *event);

private:
    KisTimeline* m_dock;
    QPointer<KisNodeModel> m_nodeModel;
    QPointer<KisNodeManager> m_nodeManager;
    int m_numberofLayers;

private slots:
    void updateUI();

private:
    inline void connectActionToButton(QToolButton* button, const QString &id);
};

#endif // KIS_ANIMATION_LAYERBOX_H
