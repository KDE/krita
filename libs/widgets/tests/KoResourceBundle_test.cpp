
#include <QTest>
#include <QCoreApplication>
#include <qtest_kde.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <QCoreApplication>
#include "KoResourceServerProvider.h"
#include "KoResourceBundle_test.h"
#include "KoResourceBundle.h"
#include <iostream>
using namespace std;

void KoResourceBundle_test::testInitialization()
{

    KoResourceServer<KoResourceBundle>* serveur =  KoResourceServerProvider::instance()->bundleServer();
    KoResourceBundle* bund = new KoResourceBundle("test");
    bund->load();
    serveur->addResource(bund);
    cout<<serveur->resources().size()<<endl;

    QStringList fileNames=KGlobal::mainComponent().dirs()->findAllResources(serveur->type().toLatin1(),serveur->extensions(),KStandardDirs::Recursive|KStandardDirs::NoDuplicates);
    cout<<fileNames.size()<<endl;

    cout<<serveur->resources().size()<<endl;

}


QTEST_KDEMAIN(KoResourceBundle_test, GUI)

#include <KoResourceBundle_test.moc>
