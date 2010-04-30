#ifndef TESTCHANGETRACKEDDELETE_H
#define TESTCHANGETRACKEDDELETE_H

#include <QObject>
#include <QtTest>

class QTextDocument;
class KoTextEditor;

class TestChangeTrackedDelete : public QObject
{
    Q_OBJECT
public:
    TestChangeTrackedDelete();
    ~TestChangeTrackedDelete();

private slots:
    void testSimpleDelete();
};

#endif
