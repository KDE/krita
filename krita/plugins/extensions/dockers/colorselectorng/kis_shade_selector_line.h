#ifndef KIS_SHADE_SELECTOR_LINE_H
#define KIS_SHADE_SELECTOR_LINE_H

#include <QWidget>

class KisShadeSelectorLine : public QWidget
{
    Q_OBJECT
public:
    explicit KisShadeSelectorLine(qreal hueDelta, qreal satDelta, qreal valDelta, QWidget *parent = 0);
    void setDelta(qreal hue, qreal sat, qreal val);
    void setColor(const QColor& color);
    void updateSettings();

protected:
    void paintEvent(QPaintEvent *);

private:
    qreal m_hueDelta;
    qreal m_saturationDelta;
    qreal m_valueDelta;

    QColor m_color;
    QColor m_backgroundColor;

    bool m_gradient;
    int m_patchCount;
    int m_lineHeight;
};

#endif // KIS_SHADE_SELECTOR_LINE_H
