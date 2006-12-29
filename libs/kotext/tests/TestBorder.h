#ifndef TESTBORDER_H
#define TESTBORDER_H

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

class TestBorder : public QObject {
    Q_OBJECT
public:
    TestBorder() {}

private slots:
    void testBorder();

};

#endif
