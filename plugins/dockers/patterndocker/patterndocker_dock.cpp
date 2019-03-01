/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "patterndocker_dock.h"

#include <QHBoxLayout>
#include <QPushButton>

#include <klocalizedstring.h>

#include <kis_canvas_resource_provider.h>
#include <kis_pattern_chooser.h>
#include <KisViewManager.h>
#include <resources/KoPattern.h>

PatternDockerDock::PatternDockerDock( )
    : QDockWidget(i18n("Patterns"))
{
    m_patternChooser = new KisPatternChooser(this);
    m_patternChooser->setPreviewOrientation(Qt::Vertical);
    m_patternChooser->setCurrentItem(0,0);
    m_patternChooser->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_patternChooser->setMinimumHeight(160);

    setWidget(m_patternChooser);
}

void PatternDockerDock::setViewManager(KisViewManager* kisview)
{
    KisCanvasResourceProvider* resourceProvider = kisview->canvasResourceProvider();
    connect(resourceProvider, SIGNAL(sigPatternChanged(KoPattern*)),
            this, SLOT(patternChanged(KoPattern*)));

    connect(m_patternChooser, SIGNAL(resourceSelected(KoResource*)),
            resourceProvider, SLOT(slotPatternActivated(KoResource*)));
}


void PatternDockerDock::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);
}


void PatternDockerDock::unsetCanvas()
{
    setEnabled(false);
}


void PatternDockerDock::patternChanged(KoPattern *pattern)
{
    m_patternChooser->setCurrentPattern(pattern);
}

