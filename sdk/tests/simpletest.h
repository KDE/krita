#ifndef SIMPLETEST_H
#define SIMPLETEST_H

#include <QtTest>
#include <QTest>
#include <QStandardPaths>

#define SIMPLE_MAIN_IMPL(TestObject) \
    QT_PREPEND_NAMESPACE(QTest::Internal::callInitMain)<TestObject>(); \
    QApplication app(argc, argv); \
    app.setAttribute(Qt::AA_Use96Dpi, true); \
    QTEST_DISABLE_KEYPAD_NAVIGATION \
    TestObject tc; \
    QTEST_SET_MAIN_SOURCE_PATH \
    return QTest::qExec(&tc, argc, argv);

#define SIMPLE_TEST_MAIN(TestObject) \
int main(int argc, char *argv[]) \
{ \
    QStandardPaths::setTestModeEnabled(true); \
    SIMPLE_MAIN_IMPL(TestObject) \
}

#endif // SIMPLETEST_H
