#ifndef KIS_TIMELINE_CELLS_H
#define KIS_TIMELINE_CELLS_H

#include <QWidget>

class KisTimeline;

class KisTimelineCells : public QWidget
{
    Q_OBJECT

public:
    KisTimelineCells(KisTimeline* parent = 0);
    int getFps(){ return m_fps; }
    int getLayerNumber(int y);
    int getLayerY(int layerNumber);
    int getFrameNumber(int x);
    int getFrameX(int frameNumber);
    int getMouseMoveY() {return m_mouseMoveY; }
    int getOffsetY() { return m_offsetY; }
    int getLayerHeight() { return m_layerHeight; }

signals:
    void mouseMovedY(int);

public slots:
    void updateContent();
    void updateFrame(int frameNumber);
    void lengthChange(QString);
    void frameSizeChange(int);
    void fontSizeChange(int);
    void scrubChange(int);
    void labelChange(int);
    void hScrollChange(int);
    void vScrollChange(int);
    void setMouseMoveY(int x){ m_mouseMoveY = x; }

protected:
    void drawContent();
    void paintLabel(QPainter &painter, int layer, int x, int y, int height, int width, bool selected, int allLayers);
    void paintTrack(QPainter &painter, int layer, int x, int y, int width, int height, bool selected, int frameSize);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    KisTimeline* m_dock;
    QString m_type;
    QPixmap* m_cache;
    bool m_drawFrameNumber;
    bool m_shortScrub;
    int m_frameLength;
    int m_frameSize;
    int m_fontSize;
    int m_fps;
    bool m_scrubbing;
    int m_layerHeight;
    int m_offsetX, m_offsetY;
    int m_startY, m_endY, m_startLayerNumber;
    int m_mouseMoveY;
    int m_frameOffset, m_layerOffset;
    int m_frameclicked, m_selectionOffset;
    int m_layerCount;
};

#endif // KIS_TIMELINE_CELLS_H
