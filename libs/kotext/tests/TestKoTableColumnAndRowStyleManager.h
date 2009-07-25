#ifndef TESTKOTABLECOLUMNANDROWSTYLEMANAGER_H
#define TESTKOTABLECOLUMNANDROWSTYLEMANAGER_H

#include <QObject>
#include <QtTest/QtTest>

class TestKoTableColumnAndRowStyleManager : public QObject
{
    Q_OBJECT
public:
    TestKoTableColumnAndRowStyleManager() {}

private slots:
    // basic testing of the manager.
    void testManager();

};

#endif // TESTKOTABLECOLUMNANDROWSTYLEMANAGER_H
