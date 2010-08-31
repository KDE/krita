/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

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
