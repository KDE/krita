#ifndef TESTCHANGETRACKEDDELETE_H
#define TESTCHANGETRACKEDDELETE_H

#include <QObject>
#include <QtTest>

class QTextDocument;
class KoTextEditor;
class TextTool;

class TestChangeTrackedDelete : public QObject
{
    Q_OBJECT
public:
    TestChangeTrackedDelete();
    ~TestChangeTrackedDelete();

private slots:
    void testDeletePreviousChar();
    void testDeleteNextChar();
    void testDeleteSelection();
    void testPrefixMerge();
    void testSuffixMerge();
};

#endif
