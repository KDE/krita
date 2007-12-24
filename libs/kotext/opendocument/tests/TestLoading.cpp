#include "TestLoading.h"

#include <QBuffer>
#include <QTextDocument>
#include <QTextCursor>
#include <QDebug>

// #include <QList>
// #include <kcomponentdata.h>
#include <KoStyleManager.h>
#include <KoDocument.h>
#include <KoOdfStylesReader.h>
#include <KoStore.h>
#include <KoOdfStylesReader.h>
#include <KoTextLoader.h>
#include <KoTextLoadingContext.h>
#include <KoXmlReader.h>

TestLoading::TestLoading() {
}

void TestLoading::testLoadLists1() {
    KoStyleManager stylemanager;
    KoTextLoader loader(&stylemanager);

    KoDocument* doc = 0;//provide a impl of KoDocument for testing only? :-/
    KoOdfStylesReader styles;
    QByteArray byteArray;
    QBuffer device(&byteArray);
    KoStore *store = KoStore::createStore(&device, KoStore::Read);
    KoTextLoadingContext context(&loader, doc, styles, store);

    // see here also the KWOpenDocumentLoader::load() method in kword/part/*

    KoXmlElement bodyElem; //=KoOdfReadStore(store).contentDoc()
    QTextDocument textdoc;
    QTextCursor textcursor(&textdoc);
    loader.loadBody(context, bodyElem, textcursor);
}

QTEST_MAIN(TestLoading)
#include "TestLoading.moc"
