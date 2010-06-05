#include "kis_colselng_color_patches.h"
#include <QPainter>

KisColSelNgColorPatches::KisColSelNgColorPatches(QWidget *parent) :
    QWidget(parent)
{
    m_numCols = 1;
    m_numRows = 30;
    m_patchWidth = 20;
    m_patchHeight = 20;

//    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    resize(m_numCols*m_patchWidth, m_numRows*m_patchHeight);
    setMinimumSize(m_numCols*m_patchWidth, m_numRows*m_patchHeight);
}

void KisColSelNgColorPatches::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    for(int i=0; i<m_numCols; i++) {
        for(int j=0; j<m_numRows; j++) {
            QColor c(qrand()|0xff000000);
            painter.fillRect(i*m_patchWidth, j*m_patchHeight, m_patchWidth, m_patchHeight, c);
        }
    }
}
