/*
 * SPDX-FileCopyrightText: 2025 Your Name <your.email@example.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoPluginLoaderTest.h"
#include "KoPluginLoader.h"
#include <QTest>

#include <kpluginfactory.h>
#include <DummyTrivialInterface.h>

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <KisMpl.h>

#include <kis_debug.h>

void KoPluginLoaderTest::initTestCase()
{
    qDebug() << QT_STRINGIFY(DUMMY_PLUGINS_PATH);
    qputenv("KRITA_PLUGIN_PATH", QT_STRINGIFY(DUMMY_PLUGINS_PATH));
}

void KoPluginLoaderTest::testLoadSinglePlugin_data()
{
    QTest::addColumn<QString>("requestedPluginId");
    QTest::addColumn<QString>("expectedPluginId");
    QTest::addColumn<int>("expectedVersion");

    QTest::addRow("load_normal") << "dummyplugin1" << "dummyplugin1" << 1;
    QTest::addRow("load_latest_version") << "dummyplugin2" << "dummyplugin2" << 2;
    QTest::addRow("load_version_as_int") << "dummyplugin3" << "dummyplugin3" << 3;
}

void KoPluginLoaderTest::testLoadSinglePlugin()
{
    QFETCH(QString, requestedPluginId);
    QFETCH(QString, expectedPluginId);
    QFETCH(int, expectedVersion);

    KoPluginLoader *loader = KoPluginLoader::instance();

    KPluginFactory *factory = loader->loadSinglePlugin(requestedPluginId, "Krita/DummyPlugin");
    QVERIFY(factory);

    std::unique_ptr<DummyTrivialInterface> plugin1(factory->create<DummyTrivialInterface>());
    QVERIFY(plugin1);

    QCOMPARE(plugin1->version(), expectedVersion);
    QCOMPARE(plugin1->name(), expectedPluginId);
}

void KoPluginLoaderTest::testLoadAll_data()
{
    QTest::addColumn<QStringList>("blacklistedIds");

    QTest::addRow("load_all") << QStringList{};
    QTest::addRow("load_blacklist") << QStringList{"dummyplugin1"};
    QTest::addRow("load_blacklist_duplicated") << QStringList{"dummyplugin2"};
}

void KoPluginLoaderTest::testLoadAll()
{
    QFETCH(QStringList, blacklistedIds);

    KoPluginLoader::PluginsConfig config;

    if (!blacklistedIds.isEmpty()) {
        config.group = "dummy";
        config.blacklist = "dummyblacklist";

        KConfigGroup configGroup(KSharedConfig::openConfig(), config.group);
        configGroup.writeEntry(config.blacklist, blacklistedIds);
    }

    std::unique_ptr<QObject> parent(new QObject);

    KoPluginLoader *loader = KoPluginLoader::instance();
    loader->load("Krita/DummyPlugin", config, parent.get(), false);

    std::vector<std::pair<QString, int>> childrenIds;
    for (QObject *obj : parent->children()) {
        DummyTrivialInterface *iface = qobject_cast<DummyTrivialInterface*>(obj);
        QVERIFY(iface);
        childrenIds.emplace_back(iface->name(), iface->version());
    }
    std::sort(childrenIds.begin(), childrenIds.end());

    std::vector<std::pair<QString, int>> expectedIds = {
        {"dummyplugin1", 1},
        {"dummyplugin2", 2},
        {"dummyplugin3", 3}
    };

    if (!blacklistedIds.isEmpty()) {
        auto it = expectedIds.begin();
        while (it != expectedIds.end()) {
            if (blacklistedIds.contains(it->first)) {
                it = expectedIds.erase(it);
            } else {
                ++it;
            }
        }

        KSharedConfig::openConfig()->deleteGroup(config.group);
    }

    if (childrenIds != expectedIds) {
        qDebug() << ppVar(childrenIds);
        qDebug() << ppVar(expectedIds);
    }


    QCOMPARE(childrenIds, expectedIds);
}

QTEST_GUILESS_MAIN(KoPluginLoaderTest)