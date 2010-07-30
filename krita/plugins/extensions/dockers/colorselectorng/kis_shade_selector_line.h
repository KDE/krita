#ifndef KIS_SHADE_SELECTOR_LINE_H
#define KIS_SHADE_SELECTOR_LINE_H

#include <QWidget>

class KisCanvas2;

class KisShadeSelectorLine : public QWidget
{
    Q_OBJECT
public:
    explicit KisShadeSelectorLine(qreal hueDelta, qreal satDelta, qreal valDelta, QWidget *parent = 0);
    void setDelta(qreal hue, qreal sat, qreal val);
    void setColor(const QColor& color);
    void updateSettings();
    void setCanvas(KisCanvas2* canvas);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);

protected slots:
    void resourceChanged(int key, const QVariant &v);

private:
    qreal m_hueDelta;
    qreal m_saturationDelta;
    qreal m_valueDelta;

    QColor m_color;
    QColor m_backgroundColor;

    QImage m_pixelCache;

    bool m_gradient;
    int m_patchCount;
    int m_lineHeight;

    KisCanvas2* m_canvas;
};

#endif // KIS_SHADE_SELECTOR_LINE_H
