#ifndef TESTSTYLES_H
#define TESTSTYLES_H

#include <QObject>
#include <qtest_kde.h>

class TestStyles : public QObject {
    Q_OBJECT
public:
    TestStyles() {}

private slots:
    void testApplyParagraphStyle();
    void testApplyParagraphStyleWithParent();
    void testCopyParagraphStyle();
};

#endif
