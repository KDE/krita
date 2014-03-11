/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
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

#include <QTest>
#include <QCoreApplication>
#include <qtest_kde.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include "KoResourceBundle_test.h"
#include "KoResourceBundle.h"
#include "KoResourceBundleManager.h"
#include "KoResourceServerProvider.h"
#include "KoResourceManagerWidget.h"
#include <iostream>
using namespace std;

void KoResourceBundle_test::testInitialization()
{
    /*KoResourceBundleManager *rm=new KoResourceBundleManager();*/
    
    /*KoResourceServer<KoAbstractGradient> *serv=KoResourceServerProvider::instance()->gradientServer();
    KoResourceBundle *test=new KoResourceBundle("/home/metabolic/pack.zip");
    test->load();
    //test->addFile("patterns","/home/metabolic/kde4/src/calligra/krita/data/patterns/01_canvas.png");
    test->addFile("gradients","/home/metabolic/kde4/src/calligra/krita/data/gradients/BG-Krita2.ggr");
    test->save();
    test->load();
    cout<<serv->fileNames().size()<<endl;
    test->install();
    cout<<qPrintable(serv->fileNames().at(0))<<endl;
    QStringList fileNames=KGlobal::mainComponent().dirs()->findAllResources(serv->type().toLatin1(),serv->extensions(),KStandardDirs::Recursive|KStandardDirs::NoDuplicates);
    cout<<fileNames.size()<<endl;*/

    /*KoResourceServer<KoResourceBundle>* serveur = new Ko
    KoResourceBundle* bund = new KoResourceBundle("test");
    bund->load();
    serveur->addResource(bund);
    cout<<serveur->resources().size()<<endl;

    QStringList fileNames=KGlobal::mainComponent().dirs()->findAllResources(serveur->type().toLatin1(),serveur->extensions(),KStandardDirs::Recursive|KStandardDirs::NoDuplicates);
    cout<<fileNames.size()<<endl;

    cout<<serveur->resources().size()<<endl;*/

    KoResourceManagerWidget w;
    w.show();
}


QTEST_KDEMAIN(KoResourceBundle_test, GUI)

#include <KoResourceBundle_test.moc>
