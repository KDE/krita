#ifndef SIMPLETEST_H
#define SIMPLETEST_H

#include <QtTest>
#include <QTest>

#define SIMPLE_TEST_MAIN(TestObject) \
int main(int argc, char *argv[]) \
{ \
    QStandardPaths::setTestModeEnabled(true); \
    QTEST_MAIN_IMPL(TestObject) \
}

#endif // SIMPLETEST_H
