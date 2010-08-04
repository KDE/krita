#ifndef KIS_SHADE_SELECTOR_LINE_H
#define KIS_SHADE_SELECTOR_LINE_H

#include <QWidget>

class KisCanvas2;
class KisShadeSelectorLineComboBox;

class KisShadeSelectorLine : public QWidget
{
    Q_OBJECT
public:
    explicit KisShadeSelectorLine(QWidget *parent = 0);
    explicit KisShadeSelectorLine(qreal hueDelta, qreal satDelta, qreal valDelta, QWidget *parent = 0);
    void setDelta(qreal hue, qreal sat, qreal val);
    void setColor(const QColor& color);
    void updateSettings();
    void setCanvas(KisCanvas2* canvas);
    void setLineNumber(int n);
    QString toString() const;
    void fromString(const QString& string);

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
    int m_lineNumber;

    KisCanvas2* m_canvas;

    friend class KisShadeSelectorLineComboBox;
};

#endif // KIS_SHADE_SELECTOR_LINE_H
