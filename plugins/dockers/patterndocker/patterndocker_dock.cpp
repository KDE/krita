/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
    : KDDockWidgets::DockWidget(i18n("Patterns"))
{
    m_patternChooser = new KisPatternChooser(this);
    m_patternChooser->setPreviewOrientation(Qt::Vertical);
    m_patternChooser->setCurrentItem(0);
    m_patternChooser->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_patternChooser->setMinimumHeight(160);

    setWidget(m_patternChooser);
}

void PatternDockerDock::setViewManager(KisViewManager* kisview)
{
    KisCanvasResourceProvider* resourceProvider = kisview->canvasResourceProvider();
    connect(resourceProvider, SIGNAL(sigPatternChanged(KoPatternSP)),
            this, SLOT(patternChanged(KoPatternSP)));

    connect(m_patternChooser, SIGNAL(resourceSelected(KoResourceSP )),
            resourceProvider, SLOT(slotPatternActivated(KoResourceSP )));
}


void PatternDockerDock::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);
}


void PatternDockerDock::unsetCanvas()
{
    setEnabled(false);
}


void PatternDockerDock::patternChanged(KoPatternSP pattern)
{
    m_patternChooser->setCurrentPattern(pattern);
}

