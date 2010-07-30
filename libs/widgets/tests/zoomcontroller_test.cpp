/*
 *  Copyright (c) 2007-2010 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include <QTest>
#include <QCoreApplication>

#include <qtest_kde.h>
#include <kactioncollection.h>
#include <kdebug.h>

#include "KoCanvasControllerWidget.h"
#include "KoZoomHandler.h"
#include "KoZoomController.h"
#include "KoDpi.h"
#include "KoUnit.h"

#include "zoomcontroller_test.h"

void zoomcontroller_test::testApi()
{
    KoZoomHandler zoomHandler;
    KoZoomController zoomController(new KoCanvasControllerWidget(), &zoomHandler, new KActionCollection(this), KoZoomAction::AspectMode);
    Q_UNUSED(zoomController);

}

QTEST_KDEMAIN(zoomcontroller_test, GUI)

#include <zoomcontroller_test.moc>

