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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <klocale.h>
#include <kactioncollection.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kdebug.h>
#include <kscan.h>
#include <KoView.h>
#include <kgenericfactory.h>
#include <kicon.h>
#include <scan.moc>

typedef KGenericFactory<Scan> ScanFactory;
K_EXPORT_COMPONENT_FACTORY( kofficescan, ScanFactory( "kscan_plugin" ) )

Scan::Scan(QObject *parent, const QStringList &)
    : KParts::Plugin(parent), scanDialog( 0 )
{
    setComponentData(ScanFactory::componentData());

    KAction *action  = new KAction(KIcon("scanner"), i18n("&Scan Image..."), this);
    actionCollection()->addAction("scan_image", action );
    connect(action, SIGNAL(triggered(bool)), SLOT(slotScan()));
}

Scan::~Scan()
{
    delete scanDialog;
}

void Scan::slotScan()
{
    if ( !scanDialog )
    {
	scanDialog = KScanDialog::getScanDialog( dynamic_cast<QWidget*>( parent() ) );
	if ( scanDialog )
	{
	    scanDialog->setMinimumSize(300, 300);

	    connect(scanDialog, SIGNAL(finalImage(const QImage &, int)),
		    this, SLOT(slotShowImage(const QImage &)));
	}
	else
        {
	    KMessageBox::sorry(0L, i18n("No scan-service available"),
			       i18n("Scanner Plugin"));
	    kDebug(31000) <<"*** No Scan-service available, aborting!";
	    return;
	}
    }

    if ( scanDialog->setup() )
        scanDialog->show();
}

void Scan::slotShowImage(const QImage &img)
{
    KTemporaryFile temp;
    temp.setPrefix("scandialog");
    temp.setSuffix(".png");
    temp.setAutoRemove(false);
    temp.open();
    img.save(temp.fileName(), "PNG");

    KoView *view = dynamic_cast<KoView *>(parent());
    if(!view)
	return;

    emit view->embedImage(temp.fileName());
}
