#ifndef KIS_COLSELNG_COLOR_PATCHES_H
#define KIS_COLSELNG_COLOR_PATCHES_H

#include <QWidget>

class KisColSelNgColorPatches : public QWidget
{
Q_OBJECT
public:
    explicit KisColSelNgColorPatches(QWidget *parent = 0);

signals:

public slots:
protected:
    void paintEvent(QPaintEvent *);
private:
    int m_numCols;
    int m_numRows;
    int m_patchWidth;
    int m_patchHeight;

};

#endif // KIS_COLSELNG_COLOR_PATCHES_H
