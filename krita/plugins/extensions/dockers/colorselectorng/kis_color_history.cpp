#include "kis_color_history.h"
#include "kis_canvas2.h"
#include "kis_view2.h"
#include "kis_canvas_resource_provider.h"

#include <QColor>

KisColorHistory::KisColorHistory(QWidget *parent) :
    KisColorPatches("lastUsedColors", parent)
{
}

void KisColorHistory::setCanvas(KisCanvas2 *canvas)
{
    KisColorPatches::setCanvas(canvas);
    connect(canvas->view()->resourceProvider(), SIGNAL(sigFGColorUsed(KoColor)),
            this,                               SLOT(commitColor(KoColor)));
}

KisColorSelectorBase* KisColorHistory::createPopup() const
{
    KisColorHistory* ret = new KisColorHistory();
    ret->setCanvas(m_canvas);
    ret->setColors(colors());
    ret->m_colorHistory=m_colorHistory;
    return ret;
}

void KisColorHistory::commitColor(const KoColor& color)
{
    m_colorHistory.removeAll(color);
    m_colorHistory.prepend(color);
    
    //the history holds 200 colors, but not all are displayed
    if(m_colorHistory.size()>200)
        m_colorHistory.removeLast();
    
    setColors(m_colorHistory);
}
