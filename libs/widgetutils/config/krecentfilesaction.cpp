/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2022 Alvin Wong <alvin@alvinhc.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "krecentfilesaction.h"
#include "krecentfilesaction_p.h"

#include <QFile>
#include <QGuiApplication>
#include <QDir>
#include <QMenu>
#include <QComboBox>
#include <QScreen>
#include <QProxyStyle>
#include <QStandardItemModel>
#include <QStyleFactory>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

#include "KisRecentFilesManager.h"

class KRecentFilesIconProxyStyle : public QProxyStyle
{
public:
    KRecentFilesIconProxyStyle(QStyle *style = nullptr)
        : QProxyStyle(style)
    {
    }

    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override
    {
        if (metric == QStyle::PM_SmallIconSize) {
            return 48;
        }
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
};

KRecentFilesAction::KRecentFilesAction(QObject *parent)
    : KSelectAction(parent),
      d_ptr(new KRecentFilesActionPrivate(this))
{
    Q_D(KRecentFilesAction);
    d->init();
}

KRecentFilesAction::KRecentFilesAction(const QString &text, QObject *parent)
    : KSelectAction(parent),
      d_ptr(new KRecentFilesActionPrivate(this))
{
    Q_D(KRecentFilesAction);
    d->init();

    // Want to keep the ampersands
    setText(text);
}

KRecentFilesAction::KRecentFilesAction(const QIcon &icon, const QString &text, QObject *parent)
    : KSelectAction(parent),
      d_ptr(new KRecentFilesActionPrivate(this))
{
    Q_D(KRecentFilesAction);
    d->init();

    setIcon(icon);
    // Want to keep the ampersands
    setText(text);
}

void KRecentFilesActionPrivate::init()
{
    Q_Q(KRecentFilesAction);
    delete q->menu();
    q->setMenu(new QMenu());
    q->setToolBarMode(KSelectAction::MenuMode);
    m_noEntriesAction = q->menu()->addAction(i18n("No Entries"));
    m_noEntriesAction->setObjectName(QLatin1String("no_entries"));
    m_noEntriesAction->setEnabled(false);
    clearSeparator = q->menu()->addSeparator();
    clearSeparator->setVisible(false);
    clearSeparator->setObjectName(QLatin1String("separator"));
    clearAction = q->menu()->addAction(i18n("Clear List"), q, SLOT(clearActionTriggered()));
    clearAction->setObjectName(QLatin1String("clear_action"));
    clearAction->setVisible(false);
    q->setEnabled(false);
    q->connect(q, SIGNAL(triggered(QAction*)), SLOT(_k_urlSelected(QAction*)));

    QString baseStyleName = q->menu()->style()->objectName();
    if (baseStyleName != QLatin1String("windows")) {
        // Force Fusion theme because other themes like Breeze doesn't
        // work well with QProxyStyle, may result in small icons.
        baseStyleName = QStringLiteral("fusion");
    }
    QStyle *baseStyle = QStyleFactory::create(baseStyleName);
    QStyle *newStyle = new KRecentFilesIconProxyStyle(baseStyle);
    newStyle->setParent(q->menu());
    q->menu()->setStyle(newStyle);

    q->connect(q->menu(),
            SIGNAL(aboutToShow()),
            SLOT(menuAboutToShow()));

    q->connect(KisRecentFilesManager::instance(),
            SIGNAL(fileAdded(const QUrl &)),
            SLOT(fileAdded(const QUrl &)));
    q->connect(KisRecentFilesManager::instance(),
            SIGNAL(fileRemoved(const QUrl &)),
            SLOT(fileRemoved(const QUrl &)));
    q->connect(KisRecentFilesManager::instance(),
            SIGNAL(listRenewed()),
            SLOT(listRenewed()));

    // We have to manually trigger the initial load because
    // KisRecentFilesManager is initialized earlier than this.
    q->rebuildEntries();
}

KRecentFilesAction::~KRecentFilesAction()
{
    delete d_ptr;
}

void KRecentFilesActionPrivate::_k_urlSelected(QAction *action)
{
    Q_Q(KRecentFilesAction);
    emit q->urlSelected(m_urls[action]);
}

void KRecentFilesActionPrivate::updateIcon(const QStandardItem *item)
{
    Q_Q(KRecentFilesAction);
    if (!item) {
        return;
    }
    const QUrl url = item->data().toUrl();
    if (!url.isValid()) {
        return;
    }
    QAction *action = m_urls.key(url);
    if (!action) {
        return;
    }
    const QIcon icon = item->icon();
    if (icon.isNull()) {
        return;
    }
    action->setIcon(icon);
    action->setIconVisibleInMenu(true);
}

static QString titleWithSensibleWidth(const QString &nameValue, const QString &value)
{
    // Calculate 3/4 of screen geometry, we do not want
    // action titles to be bigger than that
    // Since we do not know in which screen we are going to show
    // we choose the min of all the screens
    int maxWidthForTitles = INT_MAX;
    Q_FOREACH(const QScreen *screen, QGuiApplication::screens()) {
        maxWidthForTitles = qMin(maxWidthForTitles, screen->availableGeometry().width() * 3 / 4);
    }
    const QFontMetrics fontMetrics = QFontMetrics(QFont());

    QString title = nameValue + " [" + value + ']';
    if (fontMetrics.boundingRect(title).width() > maxWidthForTitles) {
        // If it does not fit, try to cut only the whole path, though if the
        // name is too long (more than 3/4 of the whole text) we cut it a bit too
        const int nameValueMaxWidth = maxWidthForTitles * 3 / 4;
        const int nameWidth = fontMetrics.boundingRect(nameValue).width();
        QString cutNameValue, cutValue;
        if (nameWidth > nameValueMaxWidth) {
            cutNameValue = fontMetrics.elidedText(nameValue, Qt::ElideMiddle, nameValueMaxWidth);
            cutValue = fontMetrics.elidedText(value, Qt::ElideMiddle, maxWidthForTitles - nameValueMaxWidth);
        } else {
            cutNameValue = nameValue;
            cutValue = fontMetrics.elidedText(value, Qt::ElideMiddle, maxWidthForTitles - nameWidth);
        }
        title = cutNameValue + " [" + cutValue + ']';
    }
    return title;
}

void KRecentFilesAction::addAction(QAction *action, const QUrl &url, const QString &/*name*/)
{
    Q_D(KRecentFilesAction);

    menu()->insertAction(menu()->actions().value(0), action);
    d->m_urls.insert(action, url);
}

QAction *KRecentFilesAction::removeAction(QAction *action)
{
    Q_D(KRecentFilesAction);
    KSelectAction::removeAction(action);

    d->m_urls.remove(action);

    return action;
}

void KRecentFilesAction::setRecentFilesModel(const QStandardItemModel *model)
{
    Q_D(KRecentFilesAction);

    if (d->m_recentFilesModel) {
        disconnect(d->m_recentFilesModel, nullptr, this, nullptr);
    }

    d->m_recentFilesModel = model;
    // Do not connect the signals or populate the icons now, because we want
    // them to be lazy-loaded only when the menu is opened for the first time.
    d->m_fileIconsPopulated = false;
}

void KRecentFilesAction::modelItemChanged(QStandardItem *item)
{
    Q_D(KRecentFilesAction);
    d->updateIcon(item);
}

void KRecentFilesAction::modelRowsInserted(const QModelIndex &/*parent*/, int first, int last)
{
    Q_D(KRecentFilesAction);
    for (int i = first; i <= last; i++) {
        d->updateIcon(d->m_recentFilesModel->item(i));
    }
}

void KRecentFilesAction::menuAboutToShow()
{
    Q_D(KRecentFilesAction);
    if (!d->m_fileIconsPopulated) {
        d->m_fileIconsPopulated = true;
        connect(d->m_recentFilesModel, SIGNAL(itemChanged(QStandardItem *)),
                SLOT(modelItemChanged(QStandardItem *)));
        connect(d->m_recentFilesModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                SLOT(modelRowsInserted(const QModelIndex &, int, int)));
        // Populate the file icons only on first showing the menu, so lazy
        // loading actually works.
        const int count = d->m_recentFilesModel->rowCount();
        for (int i = 0; i < count; i++) {
            d->updateIcon(d->m_recentFilesModel->item(i));
        }
    }
}

void KRecentFilesAction::clearActionTriggered()
{
    KisRecentFilesManager::instance()->clear();
}

void KRecentFilesAction::clearEntries()
{
    Q_D(KRecentFilesAction);
    KSelectAction::clear();
    d->m_urls.clear();
    d->m_noEntriesAction->setVisible(true);
    d->clearSeparator->setVisible(false);
    d->clearAction->setVisible(false);
    setEnabled(false);
}

void KRecentFilesAction::rebuildEntries()
{
    Q_D(KRecentFilesAction);

    clearEntries();

    QVector<KisRecentFilesEntry> items = KisRecentFilesManager::instance()->recentFiles();
    if (items.count() > d->m_visibleItemsCount) {
        items = items.mid(items.count() - d->m_visibleItemsCount);
    }
    bool thereAreEntries = false;
    Q_FOREACH(const auto &item, items) {
        QString value;
        if (item.m_url.isLocalFile()) {
            value = item.m_url.toLocalFile();
#ifdef Q_OS_WIN
            // Convert forward slashes to backslashes
            value = QDir::toNativeSeparators(value);
#endif
        } else {
            value = item.m_url.toDisplayString();
        }
        const QString nameValue = item.m_displayName;
        const QString title = titleWithSensibleWidth(nameValue, value);
        if (!value.isNull()) {
            thereAreEntries = true;
            addAction(new QAction(title, selectableActionGroup()), item.m_url, nameValue);
        }
    }
    if (thereAreEntries) {
        d->m_noEntriesAction->setVisible(false);
        d->clearSeparator->setVisible(true);
        d->clearAction->setVisible(true);
        setEnabled(true);
    }
}

void KRecentFilesAction::fileAdded(const QUrl &url)
{
    Q_D(KRecentFilesAction);
    const QString name; // Dummy

    if (d->m_visibleItemsCount <= 0) {
        return;
    }

    // remove oldest item if already maxitems in list
    if (selectableActionGroup()->actions().count() >= d->m_visibleItemsCount) {
        // remove oldest added item
        delete removeAction(selectableActionGroup()->actions().first());
    }

    const QString tmpName = name.isEmpty() ? url.fileName() : name;
    const QString pathOrUrl(url.toDisplayString(QUrl::PreferLocalFile));

#ifdef Q_OS_WIN
    const QString file = url.isLocalFile() ? QDir::toNativeSeparators(pathOrUrl) : pathOrUrl;
#else
    const QString file = pathOrUrl;
#endif

    d->m_noEntriesAction->setVisible(false);
    d->clearSeparator->setVisible(true);
    d->clearAction->setVisible(true);
    setEnabled(true);
    // add file to list
    const QString title = titleWithSensibleWidth(tmpName, file);
    QAction *action = new QAction(title, selectableActionGroup());
    addAction(action, url, tmpName);
}

void KRecentFilesAction::fileRemoved(const QUrl &url)
{
    Q_D(KRecentFilesAction);
    for (QMap<QAction *, QUrl>::ConstIterator it = d->m_urls.constBegin(); it != d->m_urls.constEnd(); ++it) {
        if (it.value() == url) {
            delete removeAction(it.key());
            return;
        }
    }
}

void KRecentFilesAction::listRenewed()
{
    rebuildEntries();
}

#include "moc_krecentfilesaction.cpp"
