/*
 * This file is part of the KDE project
 *
 * This file was copied from ksnapshot
 *
 * Copyright (C) 2001 Nikolas Zimmermann <wildfox@kde.org>
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <KoView.h>
#include <KoDocument.h>
#include <kgenericfactory.h>

#include "ksnapshot.h"
#include <kimageio.h>
#include <kis_view.h>
#include <kis_image.h>
#include "screenshot.moc"


K_EXPORT_COMPONENT_FACTORY( kritascreenshot, KGenericFactory<Screenshot>( "krita" ) )

Screenshot::Screenshot(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
{
    setInstance(KGenericFactory<Screenshot>::instance());
    
setXMLFile(KStandardDirs::locate("data","kritaplugins/screenshot-krita.rc"), 
true);


    snapshot = new KSnapshot();
    Q_CHECK_PTR(snapshot);
    connect( snapshot, SIGNAL( screenGrabbed() ), SLOT( slotScreenGrabbed() ) );

    (void) new KAction(i18n("&Screenshot..."), SmallIcon("tool_screenshot"), 0, this, SLOT(slotScreenshot()), actionCollection(), "screenshot");

}

Screenshot::~Screenshot()
{
    delete snapshot;
}

void Screenshot::slotScreenshot()
{
    snapshot->show();
}

void Screenshot::slotScreenGrabbed()
{
    KTempFile temp(KStandardDirs::locateLocal("tmp", "screenshot"), 
".png");
    snapshot->save(temp.name());

    KisView *view = dynamic_cast<KisView *>(parent());
    if(view)
        view->koDocument()->import(temp.name());
}
