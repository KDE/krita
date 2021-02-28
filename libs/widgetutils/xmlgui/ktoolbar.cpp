/* This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2000 Reginald Stadlbauer (reggie@kde.org)
    SPDX-FileCopyrightText: 1997, 1998 Stephan Kulow (coolo@kde.org)
    SPDX-FileCopyrightText: 1997, 1998 Mark Donohoe (donohoe@kde.org)
    SPDX-FileCopyrightText: 1997, 1998 Sven Radej (radej@kde.org)
    SPDX-FileCopyrightText: 1997, 1998 Matthias Ettrich (ettrich@kde.org)
    SPDX-FileCopyrightText: 1999 Chris Schlaeger (cs@kde.org)
    SPDX-FileCopyrightText: 1999 Kurt Granroth (granroth@kde.org)
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda (rodda@kde.org)

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ktoolbar.h"
#include "config-xmlgui.h"
#include <QPointer>
#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QFrame>
#include <QLayout>
#include <QMenu>
#include <QMimeData>
#include <QDrag>
#include <QMouseEvent>
#include <QToolButton>
#include <QDomElement>
#ifdef HAVE_DBUS
#include <QDBusConnection>
#include <QDBusMessage>
#endif
#include <QDebug>

#include <kconfig.h>
#include <ksharedconfig.h>
#ifdef HAVE_ICONTHEMES
#include <kicontheme.h>
#endif
#include <klocalizedstring.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <kconfiggroup.h>

#include "kactioncollection.h"
#include "kedittoolbar.h"
#include "kxmlguifactory.h"
#include "kxmlguiwindow.h"

#include <kis_icon_utils.h>

/*
 Toolbar settings (e.g. icon size or toolButtonStyle)
 =====================================================

 We have the following stack of settings (in order of priority) :
   - user-specified settings (loaded/saved in KConfig)
   - developer-specified settings in the XMLGUI file (if using xmlgui) (cannot change at runtime)
   - KDE-global default (user-configurable; can change at runtime)
 and when switching between kparts, they are saved as xml in memory,
 which, in the unlikely case of no-kmainwindow-autosaving, could be
 different from the user-specified settings saved in KConfig and would have
 priority over it.

 So, in summary, without XML:
   Global config / User settings (loaded/saved in kconfig)
 and with XML:
   Global config / App-XML attributes / User settings (loaded/saved in kconfig)

 And all those settings (except the KDE-global defaults) have to be stored in memory
 since we cannot retrieve them at random points in time, not knowing the xml document
 nor config file that holds these settings. Hence the iconSizeSettings and toolButtonStyleSettings arrays.

 For instance, if you change the KDE-global default, whether this makes a change
 on a given toolbar depends on whether there are settings at Level_AppXML or Level_UserSettings.
 Only if there are no settings at those levels, should the change of KDEDefault make a difference.
*/
enum SettingLevel { Level_KDEDefault, Level_AppXML, Level_UserSettings,
                    NSettingLevels
                  };
enum { Unset = -1 };

class KToolBar::Private
{
public:
    Private(KToolBar *qq)
        : q(qq),
          isMainToolBar(false),
          unlockedMovable(true),
          contextOrient(0),
          contextMode(0),
          contextSize(0),
          contextButtonTitle(0),
          contextShowText(0),
          contextButtonAction(0),
          contextTop(0),
          contextLeft(0),
          contextRight(0),
          contextBottom(0),
          contextIcons(0),
          contextTextRight(0),
          contextText(0),
          contextTextUnder(0),
          contextLockAction(0),
          dropIndicatorAction(0),
          context(0),
          dragAction(0)
    {
    }

    void slotAppearanceChanged();
    void slotContextAboutToShow();
    void slotContextAboutToHide();
    void slotContextLeft();
    void slotContextRight();
    void slotContextShowText();
    void slotContextTop();
    void slotContextBottom();
    void slotContextIcons();
    void slotContextText();
    void slotContextTextRight();
    void slotContextTextUnder();
    void slotContextIconSize();
    void slotLockToolBars(bool lock);

    void init(bool readConfig = true, bool isMainToolBar = false);
    QString getPositionAsString() const;
    QMenu *contextMenu(const QPoint &globalPos);
    void setLocked(bool locked);
    void adjustSeparatorVisibility();
    void loadKDESettings();
    void applyCurrentSettings();

    QAction *findAction(const QString &actionName, KXMLGUIClient **client = 0) const;

    static Qt::ToolButtonStyle toolButtonStyleFromString(const QString &style);
    static QString toolButtonStyleToString(Qt::ToolButtonStyle);
    static Qt::ToolBarArea positionFromString(const QString &position);
    static Qt::ToolButtonStyle toolButtonStyleSetting();

    KToolBar *q;
    bool isMainToolBar : 1;
    bool unlockedMovable : 1;
    static bool s_editable;
    static bool s_locked;

    QSet<KXMLGUIClient *> xmlguiClients;

    QMenu *contextOrient;
    QMenu *contextMode;
    QMenu *contextSize;

    QAction *contextButtonTitle;
    QAction *contextShowText;
    QAction *contextButtonAction;
    QAction *contextTop;
    QAction *contextLeft;
    QAction *contextRight;
    QAction *contextBottom;
    QAction *contextIcons;
    QAction *contextTextRight;
    QAction *contextText;
    QAction *contextTextUnder;
    KToggleAction *contextLockAction;
    QMap<QAction *, int> contextIconSizes;

    class IntSetting
    {
    public:
        IntSetting()
        {
            for (int level = 0; level < NSettingLevels; ++level) {
                values[level] = Unset;
            }
        }
        int currentValue() const
        {
            int val = Unset;
            for (int level = 0; level < NSettingLevels; ++level) {
                if (values[level] != Unset) {
                    val = values[level];
                }
            }
            return val;
        }
        // Default value as far as the user is concerned is kde-global + app-xml.
        // If currentValue()==defaultValue() then nothing to write into kconfig.
        int defaultValue() const
        {
            int val = Unset;
            for (int level = 0; level < Level_UserSettings; ++level) {
                if (values[level] != Unset) {
                    val = values[level];
                }
            }
            return val;
        }
        QString toString() const
        {
            QString str;
            for (int level = 0; level < NSettingLevels; ++level) {
                str += QString::number(values[level]) + QLatin1Char(' ');
            }
            return str;
        }
        int &operator[](int index)
        {
            return values[index];
        }
    private:
        int values[NSettingLevels];
    };
    IntSetting iconSizeSettings;
    IntSetting toolButtonStyleSettings; // either Qt::ToolButtonStyle or -1, hence "int".

    QList<QAction *> actionsBeingDragged;
    QAction *dropIndicatorAction;

    QMenu *context;
    QAction *dragAction;
    QPoint dragStartPosition;
};

bool KToolBar::Private::s_editable = false;
bool KToolBar::Private::s_locked = true;

void KToolBar::Private::init(bool readConfig, bool _isMainToolBar)
{
    isMainToolBar = _isMainToolBar;
    loadKDESettings();

    // also read in our configurable settings (for non-xmlgui toolbars)
    if (readConfig) {
        KConfigGroup cg(KSharedConfig::openConfig(), QString());
        q->applySettings(cg);
    }

    if (q->mainWindow()) {
        // Get notified when settings change
        connect(q, SIGNAL(allowedAreasChanged(Qt::ToolBarAreas)),
                q->mainWindow(), SLOT(setSettingsDirty()));
        connect(q, SIGNAL(iconSizeChanged(QSize)),
                q->mainWindow(), SLOT(setSettingsDirty()));
        connect(q, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                q->mainWindow(), SLOT(setSettingsDirty()));
        connect(q, SIGNAL(movableChanged(bool)),
                q->mainWindow(), SLOT(setSettingsDirty()));
        connect(q, SIGNAL(orientationChanged(Qt::Orientation)),
                q->mainWindow(), SLOT(setSettingsDirty()));
    }

    q->setMovable(!KToolBar::toolBarsLocked());

    connect(q, SIGNAL(movableChanged(bool)),
            q, SLOT(slotMovableChanged(bool)));

    q->setAcceptDrops(true);

#ifdef HAVE_DBUS
    QDBusConnection::sessionBus().connect(QString(), QStringLiteral("/KToolBar"), QStringLiteral("org.kde.KToolBar"),
                                          QStringLiteral("styleChanged"), q, SLOT(slotAppearanceChanged()));
#endif
}

QString KToolBar::Private::getPositionAsString() const
{
    // get all of the stuff to save
    switch (q->mainWindow()->toolBarArea(const_cast<KToolBar *>(q))) {
    case Qt::BottomToolBarArea:
        return QStringLiteral("Bottom");
    case Qt::LeftToolBarArea:
        return QStringLiteral("Left");
    case Qt::RightToolBarArea:
        return QStringLiteral("Right");
    case Qt::TopToolBarArea:
    default:
        return QStringLiteral("Top");
    }
}

QMenu *KToolBar::Private::contextMenu(const QPoint &globalPos)
{
    if (!context) {
        context = new QMenu(q);

        contextButtonTitle = context->addSection(i18nc("@title:menu", "Show Text"));
        contextShowText = context->addAction(QString(), q, SLOT(slotContextShowText()));

        context->addSection(i18nc("@title:menu", "Toolbar Settings"));

        contextOrient = new QMenu(i18nc("Toolbar orientation", "Orientation"), context);

        contextTop = contextOrient->addAction(i18nc("toolbar position string", "Top"), q, SLOT(slotContextTop()));
        contextTop->setChecked(true);
        contextLeft = contextOrient->addAction(i18nc("toolbar position string", "Left"), q, SLOT(slotContextLeft()));
        contextRight = contextOrient->addAction(i18nc("toolbar position string", "Right"), q, SLOT(slotContextRight()));
        contextBottom = contextOrient->addAction(i18nc("toolbar position string", "Bottom"), q, SLOT(slotContextBottom()));

        QActionGroup *positionGroup = new QActionGroup(contextOrient);
        Q_FOREACH (QAction *action, contextOrient->actions()) {
            action->setActionGroup(positionGroup);
            action->setCheckable(true);
        }

        contextMode = new QMenu(i18n("Text Position"), context);

        contextIcons = contextMode->addAction(i18n("Icons Only"), q, SLOT(slotContextIcons()));
        contextText = contextMode->addAction(i18n("Text Only"), q, SLOT(slotContextText()));
        contextTextRight = contextMode->addAction(i18n("Text Alongside Icons"), q, SLOT(slotContextTextRight()));
        contextTextUnder = contextMode->addAction(i18n("Text Under Icons"), q, SLOT(slotContextTextUnder()));

        QActionGroup *textGroup = new QActionGroup(contextMode);
        Q_FOREACH (QAction *action, contextMode->actions()) {
            action->setActionGroup(textGroup);
            action->setCheckable(true);
        }

        contextSize = new QMenu(i18n("Icon Size"), context);

        contextIconSizes.insert(contextSize->addAction(i18nc("@item:inmenu Icon size", "Default"), q, SLOT(slotContextIconSize())),
                                iconSizeSettings.defaultValue());

        QList<int> avSizes;
        avSizes << 16 << 22 << 24 << 32 << 48 << 64 << 128 << 256;

        std::sort(avSizes.begin(), avSizes.end());

        if (avSizes.count() < 10) {
            // Fixed or threshold type icons
            Q_FOREACH (int it, avSizes) {
                QString text;
                if (it < 19) {
                    text = i18n("Small (%1x%2)", it, it);
                } else if (it < 25) {
                    text = i18n("Medium (%1x%2)", it, it);
                } else if (it < 35) {
                    text = i18n("Large (%1x%2)", it, it);
                } else {
                    text = i18n("Huge (%1x%2)", it, it);
                }

                // save the size in the contextIconSizes map
                contextIconSizes.insert(contextSize->addAction(text, q, SLOT(slotContextIconSize())), it);
            }
        } else {
            // Scalable icons.
            const int progression[] = { 16, 22, 32, 48, 64, 96, 128, 192, 256 };

            for (uint i = 0; i < 9; i++) {
                Q_FOREACH (int it, avSizes) {
                    if (it >= progression[ i ]) {
                        QString text;
                        if (it < 19) {
                            text = i18n("Small (%1x%2)", it, it);
                        } else if (it < 25) {
                            text = i18n("Medium (%1x%2)", it, it);
                        } else if (it < 35) {
                            text = i18n("Large (%1x%2)", it, it);
                        } else {
                            text = i18n("Huge (%1x%2)", it, it);
                        }

                        // save the size in the contextIconSizes map
                        contextIconSizes.insert(contextSize->addAction(text, q, SLOT(slotContextIconSize())), it);
                        break;
                    }
                }
            }
        }

        QActionGroup *sizeGroup = new QActionGroup(contextSize);
        Q_FOREACH (QAction *action, contextSize->actions()) {
            action->setActionGroup(sizeGroup);
            action->setCheckable(true);
        }

        if (!q->toolBarsLocked() && !q->isMovable()) {
            unlockedMovable = false;
        }

        delete contextLockAction;
        contextLockAction = new KToggleAction(KisIconUtils::loadIcon(QStringLiteral("system-lock-screen")), i18n("Lock Toolbar Positions"), q);
        contextLockAction->setChecked(q->toolBarsLocked());
        connect(contextLockAction, SIGNAL(toggled(bool)), q, SLOT(slotLockToolBars(bool)));

        // Now add the actions to the menu
        context->addMenu(contextMode);
        context->addMenu(contextSize);
        context->addMenu(contextOrient);
        context->addSeparator();

        connect(context, SIGNAL(aboutToShow()), q, SLOT(slotContextAboutToShow()));
    }

    contextButtonAction = q->actionAt(q->mapFromGlobal(globalPos));
    if (contextButtonAction) {
        contextShowText->setText(contextButtonAction->text());
        contextShowText->setIcon(contextButtonAction->icon());
        contextShowText->setCheckable(true);
    }

    contextOrient->menuAction()->setVisible(!q->toolBarsLocked());
    // Unplugging a submenu from abouttohide leads to the popupmenu floating around
    // So better simply call that code from after exec() returns (DF)
    //connect(context, SIGNAL(aboutToHide()), this, SLOT(slotContextAboutToHide()));

    return context;
}

void KToolBar::Private::setLocked(bool locked)
{
    if (unlockedMovable) {
        q->setMovable(!locked);
    }
}

void KToolBar::Private::adjustSeparatorVisibility()
{
    bool visibleNonSeparator = false;
    int separatorToShow = -1;

    for (int index = 0; index < q->actions().count(); ++index) {
        QAction *action = q->actions()[ index ];
        if (action->isSeparator()) {
            if (visibleNonSeparator) {
                separatorToShow = index;
                visibleNonSeparator = false;
            } else {
                action->setVisible(false);
            }
        } else if (!visibleNonSeparator) {
            if (action->isVisible()) {
                visibleNonSeparator = true;
                if (separatorToShow != -1) {
                    q->actions()[ separatorToShow ]->setVisible(true);
                    separatorToShow = -1;
                }
            }
        }
    }

    if (separatorToShow != -1) {
        q->actions()[ separatorToShow ]->setVisible(false);
    }
}

Qt::ToolButtonStyle KToolBar::Private::toolButtonStyleFromString(const QString &_style)
{
    QString style = _style.toLower();
    if (style == QStringLiteral("textbesideicon") || style == QLatin1String("icontextright")) {
        return Qt::ToolButtonTextBesideIcon;
    } else if (style == QStringLiteral("textundericon") || style == QLatin1String("icontextbottom")) {
        return Qt::ToolButtonTextUnderIcon;
    } else if (style == QStringLiteral("textonly")) {
        return Qt::ToolButtonTextOnly;
    } else {
        return Qt::ToolButtonIconOnly;
    }
}

QString KToolBar::Private::toolButtonStyleToString(Qt::ToolButtonStyle style)
{
    switch (style) {
    case Qt::ToolButtonIconOnly:
    default:
        return QStringLiteral("IconOnly");
    case Qt::ToolButtonTextBesideIcon:
        return QStringLiteral("TextBesideIcon");
    case Qt::ToolButtonTextOnly:
        return QStringLiteral("TextOnly");
    case Qt::ToolButtonTextUnderIcon:
        return QStringLiteral("TextUnderIcon");
    }
}

Qt::ToolBarArea KToolBar::Private::positionFromString(const QString &position)
{
    Qt::ToolBarArea newposition = Qt::TopToolBarArea;
    if (position == QStringLiteral("left")) {
        newposition = Qt::LeftToolBarArea;
    } else if (position == QStringLiteral("bottom")) {
        newposition = Qt::BottomToolBarArea;
    } else if (position == QStringLiteral("right")) {
        newposition = Qt::RightToolBarArea;
    }
    return newposition;
}

// Global setting was changed
void KToolBar::Private::slotAppearanceChanged()
{
    loadKDESettings();
    applyCurrentSettings();
}

Qt::ToolButtonStyle KToolBar::Private::toolButtonStyleSetting()
{
    KConfigGroup group(KSharedConfig::openConfig(), "Toolbar style");
    const QString fallback = KToolBar::Private::toolButtonStyleToString(Qt::ToolButtonTextBesideIcon);
    return KToolBar::Private::toolButtonStyleFromString(group.readEntry("ToolButtonStyle", fallback));
}

void KToolBar::Private::loadKDESettings()
{
    iconSizeSettings[Level_KDEDefault] = q->iconSizeDefault();

    if (isMainToolBar) {
        toolButtonStyleSettings[Level_KDEDefault] = toolButtonStyleSetting();
    } else {
        const QString fallBack = toolButtonStyleToString(Qt::ToolButtonTextBesideIcon);
        KConfigGroup group(KSharedConfig::openConfig(), "Toolbar style");
        const QString value = group.readEntry("ToolButtonStyleOtherToolbars", fallBack);
        toolButtonStyleSettings[Level_KDEDefault] = KToolBar::Private::toolButtonStyleFromString(value);
    }
}

// Call this after changing something in d->iconSizeSettings or d->toolButtonStyleSettings
void KToolBar::Private::applyCurrentSettings()
{
    //qDebug() << q->objectName() << "iconSizeSettings:" << iconSizeSettings.toString() << "->" << iconSizeSettings.currentValue();
    const int currentIconSize = iconSizeSettings.currentValue();
    q->setIconSize(QSize(currentIconSize, currentIconSize));
    //qDebug() << q->objectName() << "toolButtonStyleSettings:" << toolButtonStyleSettings.toString() << "->" << toolButtonStyleSettings.currentValue();
    q->setToolButtonStyle(static_cast<Qt::ToolButtonStyle>(toolButtonStyleSettings.currentValue()));

    // And remember to save the new look later
    KMainWindow *kmw = q->mainWindow();
    if (kmw) {
        kmw->setSettingsDirty();
    }
}

QAction *KToolBar::Private::findAction(const QString &actionName, KXMLGUIClient **clientOut) const
{
    Q_FOREACH (KXMLGUIClient *client, xmlguiClients) {
        QAction *action = client->actionCollection()->action(actionName);
        if (action) {
            if (clientOut) {
                *clientOut = client;
            }
            return action;
        }
    }
    return 0;
}

void KToolBar::Private::slotContextAboutToShow()
{
    /**
     * The idea here is to reuse the "static" part of the menu to save time.
     * But the "Toolbars" action is dynamic (can be a single action or a submenu)
     * and ToolBarHandler::setupActions() deletes it, so better not keep it around.
     * So we currently plug/unplug the last two actions of the menu.
     * Another way would be to keep around the actions and plug them all into a (new each time) popupmenu.
     */

    KXmlGuiWindow *kmw = qobject_cast<KXmlGuiWindow *>(q->mainWindow());

    // try to find "configure toolbars" action
    QAction *configureAction = 0;
    const char *actionName;
    actionName = KStandardAction::name(KStandardAction::ConfigureToolbars);
    configureAction = findAction(QLatin1String(actionName));

    if (!configureAction && kmw) {
        configureAction = kmw->actionCollection()->action(QLatin1String(actionName));
    }

    if (configureAction) {
        context->addAction(configureAction);
    }

    context->addAction(contextLockAction);

    if (kmw) {
        kmw->setupToolbarMenuActions();
        // Only allow hiding a toolbar if the action is also plugged somewhere else (e.g. menubar)
        QAction *tbAction = kmw->toolBarMenuAction();
        if (!q->toolBarsLocked() && tbAction && tbAction->associatedWidgets().count() > 0) {
            context->addAction(tbAction);
        }
    }

    KEditToolBar::setGlobalDefaultToolBar(q->QObject::objectName().toLatin1().constData());

    // Check the actions that should be checked
    switch (q->toolButtonStyle()) {
    case Qt::ToolButtonIconOnly:
    default:
        contextIcons->setChecked(true);
        break;
    case Qt::ToolButtonTextBesideIcon:
        contextTextRight->setChecked(true);
        break;
    case Qt::ToolButtonTextOnly:
        contextText->setChecked(true);
        break;
    case Qt::ToolButtonTextUnderIcon:
        contextTextUnder->setChecked(true);
        break;
    }

    QMapIterator< QAction *, int > it = contextIconSizes;
    while (it.hasNext()) {
        it.next();
        if (it.value() == q->iconSize().width()) {
            it.key()->setChecked(true);
            break;
        }
    }

    switch (q->mainWindow()->toolBarArea(q)) {
    case Qt::BottomToolBarArea:
        contextBottom->setChecked(true);
        break;
    case Qt::LeftToolBarArea:
        contextLeft->setChecked(true);
        break;
    case Qt::RightToolBarArea:
        contextRight->setChecked(true);
        break;
    default:
    case Qt::TopToolBarArea:
        contextTop->setChecked(true);
        break;
    }

    const bool showButtonSettings = contextButtonAction
                                    && !contextShowText->text().isEmpty()
                                    && contextTextRight->isChecked();
    contextButtonTitle->setVisible(showButtonSettings);
    contextShowText->setVisible(showButtonSettings);
    if (showButtonSettings) {
        contextShowText->setChecked(contextButtonAction->priority() >= QAction::NormalPriority);
    }
}

void KToolBar::Private::slotContextAboutToHide()
{
    // We have to unplug whatever slotContextAboutToShow plugged into the menu.
    // Unplug the toolbar menu action
    KXmlGuiWindow *kmw = qobject_cast<KXmlGuiWindow *>(q->mainWindow());
    if (kmw && kmw->toolBarMenuAction()) {
        if (kmw->toolBarMenuAction()->associatedWidgets().count() > 1) {
            context->removeAction(kmw->toolBarMenuAction());
        }
    }

    // Unplug the configure toolbars action too, since it's afterwards anyway
    QAction *configureAction = 0;
    const char *actionName;
    actionName = KStandardAction::name(KStandardAction::ConfigureToolbars);
    configureAction = findAction(QLatin1String(actionName));

    if (!configureAction && kmw) {
        configureAction = kmw->actionCollection()->action(QLatin1String(actionName));
    }

    if (configureAction) {
        context->removeAction(configureAction);
    }

    context->removeAction(contextLockAction);
}

void KToolBar::Private::slotContextLeft()
{
    q->mainWindow()->addToolBar(Qt::LeftToolBarArea, q);
}

void KToolBar::Private::slotContextRight()
{
    q->mainWindow()->addToolBar(Qt::RightToolBarArea, q);
}

void KToolBar::Private::slotContextShowText()
{
    Q_ASSERT(contextButtonAction);
    const QAction::Priority priority = contextShowText->isChecked()
                                       ? QAction::NormalPriority : QAction::LowPriority;
    contextButtonAction->setPriority(priority);

    // Find to which xml file and componentData the action belongs to
    QString componentName;
    QString filename;
    KXMLGUIClient *client;
    if (findAction(contextButtonAction->objectName(), &client)) {
        componentName = client->componentName();
        filename = client->xmlFile();
    }
    if (filename.isEmpty()) {
        componentName = QCoreApplication::applicationName();
        filename = componentName + QStringLiteral("ui.xmlgui");
    }

    // Save the priority state of the action
    const QString configFile = KXMLGUIFactory::readConfigFile(filename, componentName);

    QDomDocument document;
    document.setContent(configFile);
    QDomElement elem = KXMLGUIFactory::actionPropertiesElement(document);
    QDomElement actionElem = KXMLGUIFactory::findActionByName(elem, contextButtonAction->objectName(), true);
    actionElem.setAttribute(QStringLiteral("priority"), priority);
    KXMLGUIFactory::saveConfigFile(document, filename, componentName);
}

void KToolBar::Private::slotContextTop()
{
    q->mainWindow()->addToolBar(Qt::TopToolBarArea, q);
}

void KToolBar::Private::slotContextBottom()
{
    q->mainWindow()->addToolBar(Qt::BottomToolBarArea, q);
}

void KToolBar::Private::slotContextIcons()
{
    q->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolButtonStyleSettings[Level_UserSettings] = q->toolButtonStyle();
}

void KToolBar::Private::slotContextText()
{
    q->setToolButtonStyle(Qt::ToolButtonTextOnly);
    toolButtonStyleSettings[Level_UserSettings] = q->toolButtonStyle();
}

void KToolBar::Private::slotContextTextUnder()
{
    q->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolButtonStyleSettings[Level_UserSettings] = q->toolButtonStyle();
}

void KToolBar::Private::slotContextTextRight()
{
    q->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolButtonStyleSettings[Level_UserSettings] = q->toolButtonStyle();
}

void KToolBar::Private::slotContextIconSize()
{
    QAction *action = qobject_cast<QAction *>(q->sender());
    if (action && contextIconSizes.contains(action)) {
        const int iconSize = contextIconSizes.value(action);
        q->setIconDimensions(iconSize);
    }
}

void KToolBar::Private::slotLockToolBars(bool lock)
{
    q->setToolBarsLocked(lock);
}

KToolBar::KToolBar(const QString &objectName, QWidget *parent, bool readConfig)
    : QToolBar(parent),
      d(new Private(this))
{
    setObjectName(objectName);
    // mainToolBar -> isMainToolBar = true  -> buttonStyle is configurable
    // others      -> isMainToolBar = false -> ### hardcoded default for buttonStyle !!! should be configurable? -> hidden key added
    d->init(readConfig, objectName == QStringLiteral("mainToolBar"));

    // KToolBar is auto-added to the top area of the main window if parent is a QMainWindow
    if (QMainWindow *mw = qobject_cast<QMainWindow *>(parent)) {
        mw->addToolBar(this);
    }
}

KToolBar::~KToolBar()
{
    delete d->contextLockAction;
    delete d;
}

void KToolBar::saveSettings(KConfigGroup &cg)
{
    Q_ASSERT(!cg.name().isEmpty());

    const int currentIconSize = iconSize().width();
    //qDebug() << objectName() << currentIconSize << d->iconSizeSettings.toString() << "defaultValue=" << d->iconSizeSettings.defaultValue();
    if (!cg.hasDefault("IconSize") && currentIconSize == d->iconSizeSettings.defaultValue()) {
        cg.revertToDefault("IconSize");
        d->iconSizeSettings[Level_UserSettings] = Unset;
    } else {
        cg.writeEntry("IconSize", currentIconSize);
        d->iconSizeSettings[Level_UserSettings] = currentIconSize;
    }

    const Qt::ToolButtonStyle currentToolButtonStyle = toolButtonStyle();
    if (!cg.hasDefault("ToolButtonStyle") && currentToolButtonStyle == d->toolButtonStyleSettings.defaultValue()) {
        cg.revertToDefault("ToolButtonStyle");
        d->toolButtonStyleSettings[Level_UserSettings] = Unset;
    } else {
        cg.writeEntry("ToolButtonStyle", d->toolButtonStyleToString(currentToolButtonStyle));
        d->toolButtonStyleSettings[Level_UserSettings] = currentToolButtonStyle;
    }
}


void KToolBar::addXMLGUIClient(KXMLGUIClient *client)
{
    d->xmlguiClients << client;
}

void KToolBar::removeXMLGUIClient(KXMLGUIClient *client)
{
    d->xmlguiClients.remove(client);
}

void KToolBar::contextMenuEvent(QContextMenuEvent *event)
{
    QToolBar::contextMenuEvent(event);
}

void KToolBar::loadState(const QDomElement &element)
{
    QMainWindow *mw = mainWindow();
    if (!mw) {
        return;
    }

    {
        QDomNode textNode = element.namedItem(QStringLiteral("text"));
        QByteArray domain;
        QByteArray text;
        QByteArray context;
        if (textNode.isElement()) {
            QDomElement textElement = textNode.toElement();
            domain = textElement.attribute(QStringLiteral("translationDomain")).toUtf8();
            text = textElement.text().toUtf8();
            context = textElement.attribute(QStringLiteral("context")).toUtf8();
        } else {
            textNode = element.namedItem(QStringLiteral("Text"));
            if (textNode.isElement()) {
                QDomElement textElement = textNode.toElement();
                domain = textElement.attribute(QStringLiteral("translationDomain")).toUtf8();
                text = textElement.text().toUtf8();
                context = textElement.attribute(QStringLiteral("context")).toUtf8();
            }
        }

        if (domain.isEmpty()) {
            domain = element.ownerDocument().documentElement().attribute(QStringLiteral("translationDomain")).toUtf8();
            if (domain.isEmpty()) {
                domain = KLocalizedString::applicationDomain();
            }
        }
        QString i18nText;
        if (!text.isEmpty() && !context.isEmpty()) {
            i18nText = i18ndc(domain.constData(), context.constData(), text.constData());
        } else if (!text.isEmpty()) {
            i18nText = i18nd(domain.constData(), text.constData());
        }

        if (!i18nText.isEmpty()) {
            setWindowTitle(i18nText);
        }
    }

    /*
      This method is called in order to load toolbar settings from XML.
      However this can be used in two rather different cases:
      - for the initial loading of the app's XML. In that case the settings
      are only the defaults (Level_AppXML), the user's KConfig settings will override them

      - for later re-loading when switching between parts in KXMLGUIFactory.
      In that case the XML contains the final settings, not the defaults.
      We do need the defaults, and the toolbar might have been completely
      deleted and recreated meanwhile. So we store the app-default settings
      into the XML.
    */
    bool loadingAppDefaults = true;
    if (element.hasAttribute(QStringLiteral("tempXml"))) {
        // this isn't the first time, so the app-xml defaults have been saved into the (in-memory) XML
        loadingAppDefaults = false;
        const QString iconSizeDefault = element.attribute(QStringLiteral("iconSizeDefault"));
        if (!iconSizeDefault.isEmpty()) {
            d->iconSizeSettings[Level_AppXML] = iconSizeDefault.toInt();
        }
        const QString toolButtonStyleDefault = element.attribute(QStringLiteral("toolButtonStyleDefault"));
        if (!toolButtonStyleDefault.isEmpty()) {
            d->toolButtonStyleSettings[Level_AppXML] = d->toolButtonStyleFromString(toolButtonStyleDefault);
        }
    } else {
        // loading app defaults
        bool newLine = false;
        QString attrNewLine = element.attribute(QStringLiteral("newline")).toLower();
        if (!attrNewLine.isEmpty()) {
            newLine = attrNewLine == QStringLiteral("true");
        }
        if (newLine && mw) {
            mw->insertToolBarBreak(this);
        }

    }

    int newIconSize = -1;
    if (element.hasAttribute(QStringLiteral("iconSize"))) {
        bool ok;
        newIconSize = element.attribute(QStringLiteral("iconSize")).trimmed().toInt(&ok);
        if (!ok) {
            newIconSize = -1;
        }
    }
    if (newIconSize != -1) {
        d->iconSizeSettings[loadingAppDefaults ? Level_AppXML : Level_UserSettings] = newIconSize;
    }

    const QString newToolButtonStyle = element.attribute(QStringLiteral("iconText"));
    if (!newToolButtonStyle.isEmpty()) {
        d->toolButtonStyleSettings[loadingAppDefaults ? Level_AppXML : Level_UserSettings] = d->toolButtonStyleFromString(newToolButtonStyle);
    }

    bool hidden = false;
    {
        QString attrHidden = element.attribute(QStringLiteral("hidden")).toLower();
        if (!attrHidden.isEmpty()) {
            hidden = attrHidden  == QStringLiteral("true");
        }
    }

    Qt::ToolBarArea pos = Qt::NoToolBarArea;
    {
        QString attrPosition = element.attribute(QStringLiteral("position")).toLower();
        if (!attrPosition.isEmpty()) {
            pos = KToolBar::Private::positionFromString(attrPosition);
        }
    }
    if (pos != Qt::NoToolBarArea) {
        mw->addToolBar(pos, this);
    }

    setVisible(!hidden);

    d->applyCurrentSettings();
}

// Called when switching between xmlgui clients, in order to find any unsaved settings
// again when switching back to the current xmlgui client.
void KToolBar::saveState(QDomElement &current) const
{
    Q_ASSERT(!current.isNull());

    current.setAttribute(QStringLiteral("tempXml"), QLatin1String("true"));

    current.setAttribute(QStringLiteral("noMerge"), QLatin1String("1"));
    current.setAttribute(QStringLiteral("position"), d->getPositionAsString().toLower());
    current.setAttribute(QStringLiteral("hidden"), isHidden() ? QLatin1String("true") : QLatin1String("false"));

    const int currentIconSize = iconSize().width();
    if (currentIconSize == d->iconSizeSettings.defaultValue()) {
        current.removeAttribute(QStringLiteral("iconSize"));
    } else {
        current.setAttribute(QStringLiteral("iconSize"), iconSize().width());
    }

    if (toolButtonStyle() == d->toolButtonStyleSettings.defaultValue()) {
        current.removeAttribute(QStringLiteral("iconText"));
    } else {
        current.setAttribute(QStringLiteral("iconText"), d->toolButtonStyleToString(toolButtonStyle()));
    }

    // Note: if this method is used by more than KXMLGUIBuilder, e.g. to save XML settings to *disk*,
    // then the stuff below shouldn't always be done. This is not the case currently though.
    if (d->iconSizeSettings[Level_AppXML] != Unset) {
        current.setAttribute(QStringLiteral("iconSizeDefault"), d->iconSizeSettings[Level_AppXML]);
    }
    if (d->toolButtonStyleSettings[Level_AppXML] != Unset) {
        const Qt::ToolButtonStyle bs = static_cast<Qt::ToolButtonStyle>(d->toolButtonStyleSettings[Level_AppXML]);
        current.setAttribute(QStringLiteral("toolButtonStyleDefault"), d->toolButtonStyleToString(bs));
    }
}

// called by KMainWindow::applyMainWindowSettings to read from the user settings
void KToolBar::applySettings(const KConfigGroup &cg)
{
    Q_ASSERT(!cg.name().isEmpty());

    if (cg.hasKey("IconSize")) {
        d->iconSizeSettings[Level_UserSettings] = cg.readEntry("IconSize", 0);
    }
    if (cg.hasKey("ToolButtonStyle")) {
        d->toolButtonStyleSettings[Level_UserSettings] = d->toolButtonStyleFromString(cg.readEntry("ToolButtonStyle", QString()));
    }

    d->applyCurrentSettings();
}

KMainWindow *KToolBar::mainWindow() const
{
    return qobject_cast<KMainWindow *>(const_cast<QObject *>(parent()));
}

void KToolBar::setIconDimensions(int size)
{
    QToolBar::setIconSize(QSize(size, size));
    d->iconSizeSettings[Level_UserSettings] = size;
}

int KToolBar::iconSizeDefault() const
{
    return 22;
}

void KToolBar::slotMovableChanged(bool movable)
{
    setMovable(movable);
}

void KToolBar::dragEnterEvent(QDragEnterEvent *event)
{
    if (toolBarsEditable() && event->proposedAction() & (Qt::CopyAction | Qt::MoveAction) &&
            event->mimeData()->hasFormat(QStringLiteral("application/x-kde-action-list"))) {
        QByteArray data = event->mimeData()->data(QStringLiteral("application/x-kde-action-list"));

        QDataStream stream(data);

        QStringList actionNames;

        stream >> actionNames;

        Q_FOREACH (const QString &actionName, actionNames) {
            Q_FOREACH (KActionCollection *ac, KActionCollection::allCollections()) {
                QAction *newAction = ac->action(actionName);
                if (newAction) {
                    d->actionsBeingDragged.append(newAction);
                    break;
                }
            }
        }

        if (d->actionsBeingDragged.count()) {
            QAction *overAction = actionAt(event->pos());

            QFrame *dropIndicatorWidget = new QFrame(this);
            dropIndicatorWidget->resize(8, height() - 4);
            dropIndicatorWidget->setFrameShape(QFrame::VLine);
            dropIndicatorWidget->setLineWidth(3);

            d->dropIndicatorAction = insertWidget(overAction, dropIndicatorWidget);

            insertAction(overAction, d->dropIndicatorAction);

            event->acceptProposedAction();
            return;
        }
    }

    QToolBar::dragEnterEvent(event);
}

void KToolBar::dragMoveEvent(QDragMoveEvent *event)
{
    if (toolBarsEditable())
        Q_FOREVER {
        if (d->dropIndicatorAction)
            {
                QAction *overAction = 0L;
                Q_FOREACH (QAction *action, actions()) {
                    // want to make it feel that half way across an action you're dropping on the other side of it
                    QWidget *widget = widgetForAction(action);
                    if (event->pos().x() < widget->pos().x() + (widget->width() / 2)) {
                        overAction = action;
                        break;
                    }
                }

                if (overAction != d->dropIndicatorAction) {
                    // Check to see if the indicator is already in the right spot
                    int dropIndicatorIndex = actions().indexOf(d->dropIndicatorAction);
                    if (dropIndicatorIndex + 1 < actions().count()) {
                        if (actions()[ dropIndicatorIndex + 1 ] == overAction) {
                            break;
                        }
                    } else if (!overAction) {
                        break;
                    }

                    insertAction(overAction, d->dropIndicatorAction);
                }

                event->accept();
                return;
            }
            break;
        }

    QToolBar::dragMoveEvent(event);
}

void KToolBar::dragLeaveEvent(QDragLeaveEvent *event)
{
    // Want to clear this even if toolBarsEditable was changed mid-drag (unlikely)
    delete d->dropIndicatorAction;
    d->dropIndicatorAction = 0L;
    d->actionsBeingDragged.clear();

    if (toolBarsEditable()) {
        event->accept();
        return;
    }

    QToolBar::dragLeaveEvent(event);
}

void KToolBar::dropEvent(QDropEvent *event)
{
    if (toolBarsEditable()) {
        Q_FOREACH (QAction *action, d->actionsBeingDragged) {
            if (actions().contains(action)) {
                removeAction(action);
            }
            insertAction(d->dropIndicatorAction, action);
        }
    }

    // Want to clear this even if toolBarsEditable was changed mid-drag (unlikely)
    delete d->dropIndicatorAction;
    d->dropIndicatorAction = 0L;
    d->actionsBeingDragged.clear();

    if (toolBarsEditable()) {
        event->accept();
        return;
    }

    QToolBar::dropEvent(event);
}

void KToolBar::mousePressEvent(QMouseEvent *event)
{
    if (toolBarsEditable() && event->button() == Qt::LeftButton) {
        if (QAction *action = actionAt(event->pos())) {
            d->dragAction = action;
            d->dragStartPosition = event->pos();
            event->accept();
            return;
        }
    }

    QToolBar::mousePressEvent(event);
}

void KToolBar::mouseMoveEvent(QMouseEvent *event)
{
    if (!toolBarsEditable() || !d->dragAction) {
        return QToolBar::mouseMoveEvent(event);
    }

    if ((event->pos() - d->dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
        event->accept();
        return;
    }

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    QByteArray data;
    {
        QDataStream stream(&data, QIODevice::WriteOnly);

        QStringList actionNames;
        actionNames << d->dragAction->objectName();

        stream << actionNames;
    }

    mimeData->setData(QStringLiteral("application/x-kde-action-list"), data);

    drag->setMimeData(mimeData);

    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);

    if (dropAction == Qt::MoveAction)
        // Only remove from this toolbar if it was moved to another toolbar
        // Otherwise the receiver moves it.
        if (drag->target() != this) {
            removeAction(d->dragAction);
        }

    d->dragAction = 0L;
    event->accept();
}

void KToolBar::mouseReleaseEvent(QMouseEvent *event)
{
    // Want to clear this even if toolBarsEditable was changed mid-drag (unlikely)
    if (d->dragAction) {
        d->dragAction = 0L;
        event->accept();
        return;
    }

    QToolBar::mouseReleaseEvent(event);
}

bool KToolBar::eventFilter(QObject *watched, QEvent *event)
{
    // Generate context menu events for disabled buttons too...
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->buttons() & Qt::RightButton)
            if (QWidget *ww = qobject_cast<QWidget *>(watched))
                if (ww->parent() == this && !ww->isEnabled()) {
                    QCoreApplication::postEvent(this, new QContextMenuEvent(QContextMenuEvent::Mouse, me->pos(), me->globalPos()));
                }

    } else if (event->type() == QEvent::ParentChange) {
        // Make sure we're not leaving stale event filters around,
        // when a child is reparented somewhere else
        if (QWidget *ww = qobject_cast<QWidget *>(watched)) {
            if (!this->isAncestorOf(ww)) {
                // New parent is not a subwidget - remove event filter
                ww->removeEventFilter(this);
                Q_FOREACH (QWidget *child, ww->findChildren<QWidget *>()) {
                    child->removeEventFilter(this);
                }
            }
        }
    }

    QToolButton *tb;
    if ((tb = qobject_cast<QToolButton *>(watched))) {
        const QList<QAction *> tbActions = tb->actions();
        if (!tbActions.isEmpty()) {
            // Handle MMB on toolbar buttons
            if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
                QMouseEvent *me = static_cast<QMouseEvent *>(event);
                if (me->button() == Qt::MidButton /*&&
                                                 act->receivers(SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)))*/) {
                    QAction *act = tbActions.first();
                    if (me->type() == QEvent::MouseButtonPress) {
                        tb->setDown(act->isEnabled());
                    } else {
                        tb->setDown(false);
                        if (act->isEnabled()) {
                            QMetaObject::invokeMethod(act, "triggered", Qt::DirectConnection,
                                                      Q_ARG(Qt::MouseButtons, me->button()),
                                                      Q_ARG(Qt::KeyboardModifiers, QApplication::keyboardModifiers()));
                        }
                    }
                }
                // Set highlight color on checked buttons, same as in KisHighlightedButton
                QPalette p = tb->palette();
                QColor color = p.color(tb->isChecked() ? QPalette::Highlight : QPalette::Button);
                p.setColor(QPalette::Button, color);
                tb->setPalette(p);
            }

            // CJK languages use more verbose accelerator marker: they add a Latin
            // letter in parenthesis, and put accelerator on that. Hence, the default
            // removal of ampersand only may not be enough there, instead the whole
            // parenthesis construct should be removed. Use KLocalizedString's method to do this.
            if (event->type() == QEvent::Show || event->type() == QEvent::Paint || event->type() == QEvent::EnabledChange) {
                QAction *act = tb->defaultAction();
                if (act) {
                    const QString text = KLocalizedString::removeAcceleratorMarker(act->iconText().isEmpty() ? act->text() : act->iconText());
                    const QString toolTip = KLocalizedString::removeAcceleratorMarker(act->toolTip());
                    // Filtering messages requested by translators (scripting).
                    tb->setText(i18nc("@action:intoolbar Text label of toolbar button", "%1", text));
                    tb->setToolTip(i18nc("@info:tooltip Tooltip of toolbar button", "%1", toolTip));
                }
            }
        }
    }

    // Redirect mouse events to the toolbar when drag + drop editing is enabled
    if (toolBarsEditable()) {
        if (QWidget *ww = qobject_cast<QWidget *>(watched)) {
            switch (event->type()) {
            case QEvent::MouseButtonPress: {
                QMouseEvent *me = static_cast<QMouseEvent *>(event);
                QMouseEvent newEvent(me->type(), mapFromGlobal(ww->mapToGlobal(me->pos())), me->globalPos(),
                                     me->button(), me->buttons(), me->modifiers());
                mousePressEvent(&newEvent);
                return true;
            }
            case QEvent::MouseMove: {
                QMouseEvent *me = static_cast<QMouseEvent *>(event);
                QMouseEvent newEvent(me->type(), mapFromGlobal(ww->mapToGlobal(me->pos())), me->globalPos(),
                                     me->button(), me->buttons(), me->modifiers());
                mouseMoveEvent(&newEvent);
                return true;
            }
            case QEvent::MouseButtonRelease: {
                QMouseEvent *me = static_cast<QMouseEvent *>(event);
                QMouseEvent newEvent(me->type(), mapFromGlobal(ww->mapToGlobal(me->pos())), me->globalPos(),
                                     me->button(), me->buttons(), me->modifiers());
                mouseReleaseEvent(&newEvent);
                return true;
            }
            default:
                break;
            }
        }
    }

    return QToolBar::eventFilter(watched, event);
}

void KToolBar::actionEvent(QActionEvent *event)
{
    if (event->type() == QEvent::ActionRemoved) {
        QWidget *widget = widgetForAction(event->action());
        if (widget) {
            widget->removeEventFilter(this);

            Q_FOREACH (QWidget *child, widget->findChildren<QWidget *>()) {
                child->removeEventFilter(this);
            }
        }
    }

    QToolBar::actionEvent(event);

    if (event->type() == QEvent::ActionAdded) {
        QWidget *widget = widgetForAction(event->action());
        if (widget) {
            widget->installEventFilter(this);

            Q_FOREACH (QWidget *child, widget->findChildren<QWidget *>()) {
                child->installEventFilter(this);
            }
            // Center widgets that do not have any use for more space. See bug 165274
            if (!(widget->sizePolicy().horizontalPolicy() & QSizePolicy::GrowFlag)
                    // ... but do not center when using text besides icon in vertical toolbar. See bug 243196
                    && !(orientation() == Qt::Vertical && toolButtonStyle() == Qt::ToolButtonTextBesideIcon)) {
                const int index = layout()->indexOf(widget);
                if (index != -1) {
                    layout()->itemAt(index)->setAlignment(Qt::AlignJustify);
                }
            }
        }
    }

    d->adjustSeparatorVisibility();
}

bool KToolBar::toolBarsEditable()
{
    return KToolBar::Private::s_editable;
}

void KToolBar::setToolBarsEditable(bool editable)
{
    if (KToolBar::Private::s_editable != editable) {
        KToolBar::Private::s_editable = editable;
    }
}

void KToolBar::setToolBarsLocked(bool locked)
{
    if (KToolBar::Private::s_locked != locked) {
        KToolBar::Private::s_locked = locked;

        Q_FOREACH (KMainWindow *mw, KMainWindow::memberList()) {
            Q_FOREACH (KToolBar *toolbar, mw->findChildren<KToolBar *>()) {
                toolbar->d->setLocked(locked);
            }
        }
    }
}

bool KToolBar::toolBarsLocked()
{
    return KToolBar::Private::s_locked;
}

void KToolBar::emitToolbarStyleChanged()
{
#ifdef HAVE_DBUS
    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/KToolBar"), QStringLiteral("org.kde.KToolBar"), QStringLiteral("styleChanged"));
    QDBusConnection::sessionBus().send(message);
#endif
}

#include "moc_ktoolbar.cpp"
