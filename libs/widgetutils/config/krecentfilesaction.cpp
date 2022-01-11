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
    clearAction = q->menu()->addAction(i18n("Clear List"), q, SLOT(clear()));
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

    q->connect(KisRecentFilesManager::instance(),
            SIGNAL(fileAdded(const QUrl &)),
            SLOT(fileAdded(const QUrl &)));
    q->connect(KisRecentFilesManager::instance(),
            SIGNAL(fileRemoved(const QUrl &)),
            SLOT(fileRemoved(const QUrl &)));
    q->connect(KisRecentFilesManager::instance(),
            SIGNAL(listRenewed()),
            SLOT(listRenewed()));
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

void KRecentFilesAction::addUrl(const QUrl &_url, const QString &name)
{
    KisRecentFilesManager::instance()->add(_url);
}

void KRecentFilesAction::addAction(QAction *action, const QUrl &url, const QString &name)
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

void KRecentFilesAction::removeUrl(const QUrl &url)
{
    KisRecentFilesManager::instance()->remove(url);
}

QList<QUrl> KRecentFilesAction::urls() const
{
    // switch order so last opened file is first
    QList<QUrl> sortedList;
    auto files = KisRecentFilesManager::instance()->recentFiles();
    for (int i = files.length() - 1; i >= 0; i--) {
        sortedList.append(files[i].m_url);
    }

    return sortedList;
}

void KRecentFilesAction::setUrlIcon(const QUrl &url, const QIcon &icon)
{
    Q_D(KRecentFilesAction);
    for (QMap<QAction *, QUrl>::ConstIterator it = d->m_urls.constBegin(); it != d->m_urls.constEnd(); ++it) {
        if (it.value() == url) {
            it.key()->setIcon(icon);
            it.key()->setIconVisibleInMenu(true);
            return;
        }
    }
}

void KRecentFilesAction::clear()
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

void KRecentFilesAction::loadEntries(const KConfigGroup &_config)
{
    KisRecentFilesManager::instance()->loadEntries(_config);
}

void KRecentFilesAction::saveEntries(const KConfigGroup &_cg)
{
    KisRecentFilesManager::instance()->saveEntries(_cg);
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
