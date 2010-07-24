#include "kis_color_history.h"

#include <QColor>

KisColorHistory::KisColorHistory(QWidget *parent) :
    KisColorPatches("lastUsedColors", parent)
{
}

void KisColorHistory::commitColor(const QColor& color)
{
    m_colorHistory.removeAll(color);
    m_colorHistory.prepend(color);
    
    //the history holds 200 colors, but not all are displayed
    if(m_colorHistory.size()>200)
        m_colorHistory.removeLast();
    
    setColors(m_colorHistory);
}
