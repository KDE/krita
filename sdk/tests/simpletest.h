#ifndef SIMPLETEST_H
#define SIMPLETEST_H

#include <QtTest>
#include <QTest>
#include <QStandardPaths>
#include <QLocale>
// #include <KLocalizedString>
#include <KoConfig.h>

#define SIMPLE_MAIN_IMPL(TestObject) \
    qputenv("LANGUAGE", "en"); \
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates)); \
    QStandardPaths::setTestModeEnabled(true); \
    qputenv("EXTRA_RESOURCE_DIRS", QByteArray(KRITA_EXTRA_RESOURCE_DIRS)); \
    QApplication app(argc, argv); \
    app.setAttribute(Qt::AA_Use96Dpi, true); \
    /*QLocale en_US(QLocale::English, QLocale::UnitedStates); \
    KLocalizedString::setLanguages(QStringList() << QStringLiteral("en_US"));*/ \
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
