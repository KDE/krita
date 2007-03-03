#ifndef ZOOMHANDLER_TEXT_H
#define ZOOMHANDLER_TEXT_H

#include <QtTest/QtTest>

class zoomhandler_test : public QObject
{
    Q_OBJECT

private slots:

    // tests
    void testConstruction();
    void testApi();
    void testOld();
    void testViewToDocument();
    void testDocumentToView();

};

#endif
