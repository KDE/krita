/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
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

#include "animator_dock.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolBar>

#include <klocale.h>

#include <kis_action.h>
#include <KisViewManager.h>
#include <kis_animation.h>
#include <kis_canvas2.h>
#include <kis_animation_doc.h>
#include <KisPart.h>
#include <kis_animation_model.h>

#include "kis_timeline.h"


AnimatorDock::AnimatorDock()
    : QDockWidget(i18n("Animator"))
{
    setMinimumHeight(120);
    m_timeLine = new KisTimelineWidget(this);
    this->setWidget(m_timeLine);
}

void AnimatorDock::setCanvas(KoCanvasBase *canvasBase)
{
    KisCanvas2 *canvas = dynamic_cast<KisCanvas2*>(canvasBase);
    if (canvas && canvas->viewManager() && canvas->viewManager()->document()) {
        m_timeLine->setCanvas(canvas);
    }
}

void AnimatorDock::unsetCanvas()
{
    m_timeLine->unsetCanvas();
}

#include "animator_dock.moc"
