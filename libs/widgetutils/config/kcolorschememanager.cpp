/* This file is part of the KDE project
 * Copyright (C) 2013 Martin Gräßlin <mgraesslin@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kcolorschememanager.h"
#include "kcolorschememanager_p.h"

#include <kactionmenu.h>
#include <kconfiggroup.h>
#include <kcolorscheme.h>
#include <ksharedconfig.h>

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QStandardPaths>

KColorSchemeManagerPrivate::KColorSchemeManagerPrivate()
    : model(new KColorSchemeModel())
{
}

KColorSchemeModel::KColorSchemeModel(QObject *parent)
    : QAbstractListModel(parent)
{
    init();
}

KColorSchemeModel::~KColorSchemeModel()
{
}

int KColorSchemeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_data.count();
}

QVariant KColorSchemeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (index.row() >= m_data.count())) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return m_data.at(index.row()).name;
    case Qt::DecorationRole:
        return m_data.at(index.row()).preview;
    case Qt::UserRole:
        return m_data.at(index.row()).path;
    default:
        return QVariant();
    }
}

void KColorSchemeModel::init()
{
    beginResetModel();
    m_data.clear();

    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                             QStringLiteral("color-schemes"),
                             QStandardPaths::LocateDirectory);
    QStringList schemeFiles;
    Q_FOREACH (const QString &dir, dirs) {
        const QStringList fileNames = QDir(dir).entryList(QStringList() << QStringLiteral("*.colors"));
        Q_FOREACH (const QString &file, fileNames) {
            schemeFiles << dir + QDir::separator() + file;
        }
    }
    Q_FOREACH (const QString &schemeFile, schemeFiles) {
        KSharedConfigPtr config = KSharedConfig::openConfig(schemeFile);
        KConfigGroup group(config, QStringLiteral("General"));
        const QString name = group.readEntry("Name", QFileInfo(schemeFile).baseName());
        const KColorSchemeModelData data = {name, schemeFile, createPreview(schemeFile)};
        m_data.append(data);
    }
    std::sort(m_data.begin(), m_data.end(), [](const KColorSchemeModelData & first, const KColorSchemeModelData & second) {
        return first.name < second.name;
    });
    endResetModel();
}

QIcon KColorSchemeModel::createPreview(const QString &path)
{
    KSharedConfigPtr schemeConfig = KSharedConfig::openConfig(path);
    QIcon result;
    KColorScheme activeWindow(QPalette::Active, KColorScheme::Window, schemeConfig);
    KColorScheme activeButton(QPalette::Active, KColorScheme::Button, schemeConfig);
    KColorScheme activeView(QPalette::Active, KColorScheme::View, schemeConfig);
    KColorScheme activeSelection(QPalette::Active, KColorScheme::Selection, schemeConfig);

    auto pixmap = [&](int size) {
        QPixmap pix(size, size);
        pix.fill(Qt::black);
        QPainter p;
        p.begin(&pix);
        const int itemSize = size / 2 - 1;
        p.fillRect(1, 1, itemSize, itemSize, activeWindow.background());
        p.fillRect(1 + itemSize, 1, itemSize, itemSize, activeButton.background());
        p.fillRect(1, 1 + itemSize, itemSize, itemSize, activeView.background());
        p.fillRect(1 + itemSize, 1 + itemSize, itemSize, itemSize, activeSelection.background());
        p.end();
        result.addPixmap(pix);
    };
    // 16x16
    pixmap(16);
    // 24x24
    pixmap(24);

    return result;
}

KColorSchemeManager::KColorSchemeManager(QObject *parent)
    : QObject(parent)
    , d(new KColorSchemeManagerPrivate)
{
}

KColorSchemeManager::~KColorSchemeManager()
{
}

QAbstractItemModel *KColorSchemeManager::model() const
{
    return d->model.data();
}

QModelIndex KColorSchemeManager::indexForScheme(const QString &name) const
{
    for (int i = 0; i < d->model->rowCount(); ++i) {
        QModelIndex index = d->model->index(i);
        if (index.data().toString() == name) {
            return index;
        }
    }
    return QModelIndex();
}

KActionMenu *KColorSchemeManager::createSchemeSelectionMenu(const QIcon &icon, const QString &name, const QString &selectedSchemeName, QObject *parent)
{
    KActionMenu *menu = new KActionMenu(icon, name, parent);
    QActionGroup *group = new QActionGroup(menu);
    connect(group, &QActionGroup::triggered, [](QAction * action) {
        // hint for the style to synchronize the color scheme with the window manager/compositor
        qApp->setProperty("KDE_COLOR_SCHEME_PATH", action->data());
        qApp->setPalette(KColorScheme::createApplicationPalette(KSharedConfig::openConfig(action->data().toString())));
    });
    for (int i = 0; i < d->model->rowCount(); ++i) {
        QModelIndex index = d->model->index(i);
        QAction *action = new QAction(index.data(Qt::DecorationRole).value<QIcon>(), index.data().toString(), menu);
        action->setData(index.data(Qt::UserRole));
        action->setActionGroup(group);
        action->setCheckable(true);
        if (index.data().toString() == selectedSchemeName) {
            action->setChecked(true);
        }
        menu->addAction(action);
    }
    return menu;
}

KActionMenu *KColorSchemeManager::createSchemeSelectionMenu(const QString &text, const QString &selectedSchemeName, QObject *parent)
{
    return createSchemeSelectionMenu(QIcon(), text, selectedSchemeName, parent);
}

KActionMenu *KColorSchemeManager::createSchemeSelectionMenu(const QString &selectedSchemeName, QObject *parent)
{
    return createSchemeSelectionMenu(QIcon(), QString(), selectedSchemeName, parent);
}

void KColorSchemeManager::activateScheme(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    if (index.model() != d->model.data()) {
        return;
    }
    // hint for the style to synchronize the color scheme with the window manager/compositor
    qApp->setProperty("KDE_COLOR_SCHEME_PATH", index.data(Qt::UserRole));
    qApp->setPalette(KColorScheme::createApplicationPalette(KSharedConfig::openConfig(index.data(Qt::UserRole).toString())));
}
