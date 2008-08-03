#ifndef TESTSTYLES_H
#define TESTSTYLES_H

#include <QObject>
//#include <QPainter>
#include <qtest_kde.h>

//#include <KoTextShapeData.h>
//#include <KoTextDocumentLayout.h>
//#include <KoShape.h>
//#include <KoViewConverter.h>

//class MockTextShape;
//class QTextDocument;

class TestStyles : public QObject {
    Q_OBJECT
public:
    TestStyles() {}

private slots:
    void testApplyParagraphStyle();
    void testApplyParagraphStyleWithParent();

};

#endif
