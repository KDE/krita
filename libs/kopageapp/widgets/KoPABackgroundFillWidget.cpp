/* This file is part of the KDE project
 * Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
 * Copyright (C) 2013 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
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

#include "KoPABackgroundFillWidget.h"

#include <KoPageApp.h>
#include <KoPAViewBase.h>
#include <KoCanvasBase.h>
#include <KoShape.h>
#include <KoDocumentResourceManager.h>

#include <klocalizedstring.h>

KoPABackgroundFillWidget::KoPABackgroundFillWidget(QWidget *parent)
: KoFillConfigWidget(parent)
{
    setWindowTitle(i18n("Background"));
}

void KoPABackgroundFillWidget::setView(KoPAViewBase *view)
{
    Q_ASSERT(view);
    connect(view->proxyObject, SIGNAL(activePageChanged()),
             this, SLOT(shapeChanged()));
}

KoShape* KoPABackgroundFillWidget::currentShape()
{
    KoShape *slide = canvas()->resourceManager()->koShapeResource(KoPageApp::CurrentPage);
    return slide;
}

QList<KoShape*> KoPABackgroundFillWidget::currentShapes()
{
    KoShape *slide = canvas()->resourceManager()->koShapeResource(KoPageApp::CurrentPage);
    QList<KoShape*> list;
    list.append(slide);
    return list;
}
