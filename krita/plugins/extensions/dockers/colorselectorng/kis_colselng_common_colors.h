#ifndef KIS_COLSELNG_COMMON_COLORS_H
#define KIS_COLSELNG_COMMON_COLORS_H

#include <QWidget>

class KisColSelNgCommonColors : public QWidget
{
Q_OBJECT
public:
    explicit KisColSelNgCommonColors(QWidget *parent = 0);
    int heightForWidth(int) const;
    QSize sizeHint() const;
private:
    QList<QColor> extractColors();
    QList<QRgb> getColors();

    int m_numColors;
    int m_patchWidth;
    int m_patchHeight;
    QList<QColor> m_extractedColors;
protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
signals:

public slots:

};

#endif // KIS_COLSELNG_COMMON_COLORS_H
