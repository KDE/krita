#ifndef TESTLISTSTYLE_H
#define TESTLISTSTYLE_H

#include <QObject>
#include <QtTest/QtTest>

//   #include <KoTextShapeData.h>
//   #include <KoTextDocumentLayout.h>
//   #include <KoShape.h>
//
//   class QPainter;
//   class KoViewConverter;
//   class KoStyleManager;
//   class MockTextShape;
//   class QTextDocument;
//   class QTextLayout;

class TestListStyle : public QObject {
    Q_OBJECT
public:
    TestListStyle() {}

private slots:
    void testListStyle();

};

#endif
