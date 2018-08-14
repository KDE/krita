/*
 *  Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISTEST
#define KISTEST

#include <KoConfig.h>
#include <QApplication>
#include <QTest>
#include <QtTest/qtestsystem.h>
#include <set>

#ifndef QT_NO_OPENGL
#  define QTEST_ADD_GPU_BLACKLIST_SUPPORT_DEFS \
    extern Q_TESTLIB_EXPORT std::set<QByteArray> *(*qgpu_features_ptr)(const QString &); \
    extern Q_GUI_EXPORT std::set<QByteArray> *qgpu_features(const QString &);
#  define QTEST_ADD_GPU_BLACKLIST_SUPPORT \
    qgpu_features_ptr = qgpu_features;
#else
#  define QTEST_ADD_GPU_BLACKLIST_SUPPORT_DEFS
#  define QTEST_ADD_GPU_BLACKLIST_SUPPORT
#endif

#if defined(QT_NETWORK_LIB)
#  include <QtTest/qtest_network.h>
#endif
#include <QtTest/qtest_widgets.h>

#ifdef QT_KEYPAD_NAVIGATION
#  define QTEST_DISABLE_KEYPAD_NAVIGATION QApplication::setNavigationMode(Qt::NavigationModeNone);
#else
#  define QTEST_DISABLE_KEYPAD_NAVIGATION
#endif

#define KISTEST_MAIN(TestObject) \
int main(int argc, char *argv[]) \
{ \
\
    if (qEnvironmentVariableIsSet("QT_LOGGING_RULES")) { \
        qWarning() << "Disable extra debugging output!!!"; \
        qputenv("QT_LOGGING_RULES", \
                qgetenv("QT_LOGGING_RULES") + \
                QByteArrayLiteral(";krita.lib.plugin.debug=false;krita.lib.resources.debug=false;krita.lib.pigment.debug=false")); \
    } \
\
    qputenv("EXTRA_RESOURCE_DIRS", QByteArray(KRITA_EXTRA_RESOURCE_DIRS)); \
    QApplication app(argc, argv); \
    app.setAttribute(Qt::AA_Use96Dpi, true); \
    QTEST_DISABLE_KEYPAD_NAVIGATION \
    TestObject tc; \
    QTEST_SET_MAIN_SOURCE_PATH \
    return QTest::qExec(&tc, argc, argv); \
}





#endif
