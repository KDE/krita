/* This file is part of the KDE project
   Copyright (C) 2001 Nikolas Zimmermann <wildfox@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kscan.h>
#include <koView.h>

#include "scan_factory.h"
#include "scan.moc"

Scan::Scan(QObject *parent, const char *name) : KParts::Plugin(parent, name),
						scanDialog( 0 )
{
    setInstance(ScanFactory::pluginInstance());

    (void) new KAction(i18n("&Scan Image"), BarIcon("scanner"), 0, this, SLOT(slotScan()), actionCollection(), "scan_image");
}

Scan::~Scan()
{
    delete scanDialog;
}

void Scan::slotScan()
{
    if ( !scanDialog )
    {
	scanDialog = KScanDialog::getScanDialog();
	if ( scanDialog )
	    connect(scanDialog, SIGNAL(finalImage(const QImage &, int)), 
		    this, SLOT(slotShowImage(const QImage &)));
	else
        {
	    KMessageBox::sorry(0L, i18n("No Scan-Service available"), 
			       i18n("Scanner Plugin"));
	    kdDebug(31000) << "*** No Scan-service available, aborting!" << endl;
	    return;
	}
    }

    scanDialog->show();
}

void Scan::slotShowImage(const QImage &img)
{
    KTempFile temp(locateLocal("tmp", "scandialog"), ".png");
    img.save(temp.name(), "PNG");

    KoView *view = dynamic_cast<KoView *>(parent());
    if(!view)
	return;

    emit view->embeddImage(temp.name());
}
