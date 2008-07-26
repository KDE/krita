/* This file is part of the KDE project
 * Copyright (c) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoSnapGuideConfigAction.h"
#include "KoSnapGuideConfigWidget.h"

#include <KoToolManager.h>
#include <KoCanvasController.h>
#include <KoCanvasBase.h>
#include <KoSnapGuide.h>

#include <KLocale>

KoSnapGuideConfigAction::KoSnapGuideConfigAction(QObject *parent)
    : QAction(parent)
{
    setText( i18n( "Configure Snap Guides..." ) );
    connect(this, SIGNAL(triggered()), this, SLOT(showSnapGuideDialog()));
}

KoSnapGuideConfigAction::~KoSnapGuideConfigAction()
{
}

void KoSnapGuideConfigAction::showSnapGuideDialog()
{
    KoCanvasController * controller = KoToolManager::instance()->activeCanvasController();
    if( ! controller )
        return;

    KoCanvasBase * canvas = controller->canvas();
    if( ! canvas )
        return;

    KoSnapGuide * snapGuide = canvas->snapGuide();
    if( ! snapGuide )
        return;

    int oldStrategies = snapGuide->enabledSnapStrategies();
    int oldSnapDistance = snapGuide->snapDistance();

    KoSnapGuideConfigWidget * widget = new KoSnapGuideConfigWidget( snapGuide, canvas->canvasWidget() );

    KDialog * dlg = new KDialog( canvas->canvasWidget() );
    dlg->setCaption( i18n( "Configure Snap Guides" ) );
    dlg->setButtons( KDialog::Ok | KDialog::Cancel );
    dlg->setMainWidget( widget );

    if( dlg->exec() != QDialog::Accepted )
    {
        snapGuide->setSnapDistance( oldSnapDistance );
        snapGuide->enableSnapStrategies( oldStrategies );
    }
}

#include "KoSnapGuideConfigAction.moc"
