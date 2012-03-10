/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "patterndocker_dock.h"

#include <QHBoxLayout>
#include <QPushButton>

#include <klocale.h>

#include <KoCanvasResourceManager.h>
#include <KoCanvasBase.h>

#include <kis_canvas2.h>
#include <kis_view2.h>
#include <kis_paintop_box.h>
#include <kis_canvas_resource_provider.h>
#include <kis_pattern_chooser.h>
#include <kis_pattern.h>


PatternDockerDock::PatternDockerDock( )
    : QDockWidget(i18n("Pattern Palette"))
    , m_canvas(0)
{
    m_patternChooser = new KisPatternChooser(this);
    m_patternChooser->setPreviewOrientation(Qt::Vertical);
    m_patternChooser->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setWidget(m_patternChooser);
}

void PatternDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_patternChooser->disconnect(m_canvas->view()->paintOpBox());
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    Q_ASSERT(m_canvas);
    if (!m_canvas) return;

    connect(m_canvas->resourceManager(), SIGNAL(resourceChanged(int, const QVariant&)),
           this, SLOT(resourceChanged(int, const QVariant&)));

    connect(m_patternChooser, SIGNAL(resourceSelected(KoResource*)),
            m_canvas->view()->paintOpBox(), SLOT(resourceSelected(KoResource*)));
}

void PatternDockerDock::resourceChanged(int /*key*/, const QVariant& /*v*/)
{
//    if (m_canvas) {
//        KisPattern *pattern = m_canvas->resourceManager()->resource(KisCanvasResourceProvider::CurrentPattern).value<KisPattern*>();
//    }
}

#include "patterndocker_dock.moc"
