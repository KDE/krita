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

int KRecentFilesAction::maxItems() const
{
    Q_D(const KRecentFilesAction);
    return d->m_maxItems;
}

void KRecentFilesAction::setMaxItems(int maxItems)
{
    Q_D(KRecentFilesAction);
    // set new maxItems
    d->m_maxItems = maxItems;

    // remove all excess items
    while (selectableActionGroup()->actions().count() > maxItems) {
        delete removeAction(selectableActionGroup()->actions().last());
    }
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
    Q_D(KRecentFilesAction);

    if (d->m_maxItems <= 0) {
        return;
    }

    /**
     * Create a deep copy here, because if _url is the parameter from
     * urlSelected() signal, we will delete it in the removeAction() call below.
     * but access it again in the addAction call... => crash
     */
    const QUrl url(_url);

    if (url.isLocalFile() && url.toLocalFile().startsWith(QDir::tempPath())) {
        return;
    }
    const QString tmpName = name.isEmpty() ? url.fileName() : name;
    const QString pathOrUrl(url.toDisplayString(QUrl::PreferLocalFile));

#ifdef Q_OS_WIN
    const QString file = url.isLocalFile() ? QDir::toNativeSeparators(pathOrUrl) : pathOrUrl;
#else
    const QString file = pathOrUrl;
#endif

    // remove file if already in list
    foreach (QAction *action, selectableActionGroup()->actions()) {
        const QString urlStr = d->m_urls[action].toDisplayString(QUrl::PreferLocalFile);
#ifdef Q_OS_WIN
        const QString tmpFileName = url.isLocalFile() ? QDir::toNativeSeparators(urlStr) : urlStr;
        if (tmpFileName.endsWith(file, Qt::CaseInsensitive))
#else
        if (urlStr.endsWith(file))
#endif
        {
            removeAction(action)->deleteLater();
            break;
        }
    }

    // remove oldest item if already maxitems in list
    if (selectableActionGroup()->actions().count() > d->m_maxItems) {
        // remove oldest added item
        delete removeAction(selectableActionGroup()->actions().first());
    }

    d->m_noEntriesAction->setVisible(false);
    d->clearSeparator->setVisible(true);
    d->clearAction->setVisible(true);
    setEnabled(true);
    // add file to list
    const QString title = titleWithSensibleWidth(tmpName, file);
    QAction *action = new QAction(title, selectableActionGroup());
    addAction(action, url, tmpName);

    // This is needed to load thumbnail for the recents menu.
    d_urls.append(QUrl(url));
}

void KRecentFilesAction::addAction(QAction *action, const QUrl &url, const QString &name)
{
    Q_D(KRecentFilesAction);

    menu()->insertAction(menu()->actions().value(0), action);
    d->m_shortNames.insert(action, name);
    d->m_urls.insert(action, url);
}

QAction *KRecentFilesAction::removeAction(QAction *action)
{
    Q_D(KRecentFilesAction);
    KSelectAction::removeAction(action);

    d->m_shortNames.remove(action);
    d->m_urls.remove(action);

    return action;
}

void KRecentFilesAction::removeUrl(const QUrl &url)
{
    Q_D(KRecentFilesAction);
    for (QMap<QAction *, QUrl>::ConstIterator it = d->m_urls.constBegin(); it != d->m_urls.constEnd(); ++it)
        if (it.value() == url) {
            delete removeAction(it.key());
            return;
        }
}

QList<QUrl> KRecentFilesAction::urls() const
{
    // switch order so last opened file is first
    QList<QUrl> sortedList;
    for (int i=(d_urls.length()-1); i >= 0; i--) {
            sortedList.append(d_urls[i]);
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
    clearEntries();
    emit recentListCleared();
}

void KRecentFilesAction::clearEntries()
{
    Q_D(KRecentFilesAction);
    KSelectAction::clear();
    d->m_shortNames.clear();
    d->m_urls.clear();
    d->m_noEntriesAction->setVisible(true);
    d->clearSeparator->setVisible(false);
    d->clearAction->setVisible(false);
    setEnabled(false);

    d_urls.clear();
}

void KRecentFilesAction::loadEntries(const KConfigGroup &_config)
{
    Q_D(KRecentFilesAction);
    clearEntries();

    QString key;
    QString value;
    QString nameKey;
    QString nameValue;
    QString title;
    QUrl    url;

    KConfigGroup cg = _config;
    if (cg.name().isEmpty()) {
        cg = KConfigGroup(cg.config(), "RecentFiles");
    }

    d->m_maxItems = cg.readEntry("maxRecentFileItems", 100);

    bool thereAreEntries = false;
    // read file list
    for (int i = 0; i < d->m_maxItems; ++i) {
        key = QString("File%1").arg(i+1);
#ifdef Q_OS_ANDROID
        value = cg.readEntry(key, QString());
#else
        value = cg.readPathEntry(key, QString());
#endif
        if (value.isEmpty()) {
            continue;
        }
        url = QUrl::fromUserInput(value);
        d_urls.append(QUrl(url)); // will be used to retrieve on the welcome screen

        // Don't restore if file doesn't exist anymore
        if (url.isLocalFile() && !QFile::exists(url.toLocalFile())) {
            continue;
        }

        // Don't restore where the url is already known (eg. broken config)
        if (d->m_urls.values().contains(url)) {
            continue;
        }

#ifdef Q_OS_WIN
        // convert to backslashes
        if (url.isLocalFile()) {
            value = QDir::toNativeSeparators(value);
        }
#endif

        nameKey = QString("Name%1").arg(i+1);
        nameValue = cg.readPathEntry(nameKey, url.fileName());
        title = titleWithSensibleWidth(nameValue, value);
        if (!value.isNull()) {
            thereAreEntries = true;
            addAction(new QAction(title, selectableActionGroup()), url, nameValue);
        }
    }
    if (thereAreEntries) {
        d->m_noEntriesAction->setVisible(false);
        d->clearSeparator->setVisible(true);
        d->clearAction->setVisible(true);
        setEnabled(true);
    }
}

void KRecentFilesAction::saveEntries(const KConfigGroup &_cg)
{
    Q_D(KRecentFilesAction);
    QString     key;
    QString     value;
    QStringList lst = items();

    KConfigGroup cg = _cg;
    if (cg.name().isEmpty()) {
        cg = KConfigGroup(cg.config(), "RecentFiles");
    }

    cg.deleteGroup();

    cg.writeEntry("maxRecentFileItems", d->m_maxItems);

    // write file list
    for (int i = 0; i < selectableActionGroup()->actions().count(); ++i) {
        key = QString("File%1").arg(i+1);
#ifdef Q_OS_ANDROID
        value = d->m_urls[selectableActionGroup()->actions()[i]].toDisplayString();
        cg.writeEntry(key, value);
#else
        value = d->m_urls[selectableActionGroup()->actions()[i]].toDisplayString(QUrl::PreferLocalFile);
        cg.writePathEntry(key, value);
#endif
        key = QString("Name%1").arg(i+1);
        value = d->m_shortNames[selectableActionGroup()->actions()[i]];
#ifdef Q_OS_ANDROID
        cg.writeEntry(key, value);
#else
        cg.writePathEntry(key, value);
#endif
    }

}

#include "moc_krecentfilesaction.cpp"
