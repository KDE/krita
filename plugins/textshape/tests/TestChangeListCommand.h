#ifndef TESTCHANGELISTCOMMAND_H
#define TESTCHANGELISTCOMMAND_H

#include <QObject>
#include <QtTest>

class TestChangeListCommand : public QObject
{
    Q_OBJECT
public:
    TestChangeListCommand() {}

private Q_SLOTS:
    void addList();
    void addListWithLevel2();
    void removeList();
    void joinList();
    void joinList2();
    void splitList();
};

#endif
