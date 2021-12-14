/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 1999, 2000 Simon Hausmann <hausmann@kde.org>
   SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kxmlguifactory.h"

#include "config-xmlgui.h"

#include "kxmlguifactory_p.h"
#include "kxmlguiclient.h"
#include "kxmlguibuilder.h"
#include "KisShortcutsDialog.h"
#include "kactioncollection.h"

#include <QAction>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>
#include <QWidget>
#include <QDate>
#include <QVariant>
#include <QTextCodec>
#include <QStandardPaths>
#include <QDebug>

#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include <kis_icon_utils.h>
#include <WidgetUtilsDebug.h>

Q_DECLARE_METATYPE(QList<QKeySequence>)

using namespace KXMLGUI;

class KXMLGUIFactoryPrivate : public BuildState
{
public:
    enum ShortcutOption { SetActiveShortcut = 1, SetDefaultShortcut = 2};

    KXMLGUIFactoryPrivate()
    {
        m_rootNode = new ContainerNode(0L, QString(), QString());
        m_defaultMergingName = QStringLiteral("<default>");
        tagActionList = QStringLiteral("actionlist");
        attrName = QStringLiteral("name");
    }
    ~KXMLGUIFactoryPrivate()
    {
        delete m_rootNode;
    }

    void pushState()
    {
        m_stateStack.push(*this);
    }

    void popState()
    {
        BuildState::operator=(m_stateStack.pop());
    }

    bool emptyState() const
    {
        return m_stateStack.isEmpty();
    }

    QWidget *findRecursive(KXMLGUI::ContainerNode *node, bool tag);
    QList<QWidget *> findRecursive(KXMLGUI::ContainerNode *node, const QString &tagName);
    void applyActionProperties(const QDomElement &element,
                               ShortcutOption shortcutOption = KXMLGUIFactoryPrivate::SetActiveShortcut);
    void configureAction(QAction *action, const QDomNamedNodeMap &attributes,
                         ShortcutOption shortcutOption = KXMLGUIFactoryPrivate::SetActiveShortcut);
    void configureAction(QAction *action, const QDomAttr &attribute,
                         ShortcutOption shortcutOption = KXMLGUIFactoryPrivate::SetActiveShortcut);


    void refreshActionProperties(KXMLGUIClient *client, const QList<QAction *> &actions, const QDomDocument &doc);
    void saveDefaultActionProperties(const QList<QAction *> &actions);

    ContainerNode *m_rootNode;

    QString m_defaultMergingName;

    /*
     * Contains the container which is searched for in ::container .
     */
    QString m_containerName;

    /*
     * List of all clients
     */
    QList<KXMLGUIClient *> m_clients;

    QString tagActionList;

    QString attrName;

    BuildStateStack m_stateStack;
};

QString KXMLGUIFactory::readConfigFile(const QString &filename, const QString &_componentName)
{
    QString componentName = _componentName.isEmpty() ? QCoreApplication::applicationName() : _componentName;
    QString xml_file;

    if (!QDir::isRelativePath(filename)) {
        xml_file = filename;
    } else {
        // KF >= 5.1 (KXMLGUI_INSTALL_DIR)
        xml_file = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("kxmlgui5/") + componentName + QLatin1Char('/') + filename);
        if (!QFile::exists(xml_file)) {
            // KF >= 5.4 (resource file)
            xml_file = QStringLiteral(":/kxmlgui5/") + componentName + QLatin1Char('/') + filename;
        }

        bool warn = false;
        if (!QFile::exists(xml_file)) {
            // kdelibs4 / KF 5.0 solution
            xml_file = QStandardPaths::locate(QStandardPaths::AppDataLocation, componentName + QLatin1Char('/') + filename);
            warn = true;
        }

        if (!QFile::exists(xml_file)) {
            // kdelibs4 / KF 5.0 solution, and the caller includes the component name
            // This was broken (lead to component/component/ in kdehome) and unnecessary
            // (they can specify it with setComponentName instead)
            xml_file = QStandardPaths::locate(QStandardPaths::AppDataLocation, filename);
            warn = true;
        }

        if (warn) {
            qWarning() << "kxmlguifactory: KXMLGUI file found at deprecated location" << xml_file << "-- please use ${KXMLGUI_INSTALL_DIR} to install these files instead.";
        }
    }

    QFile file(xml_file);
    if (xml_file.isEmpty() || !file.open(QIODevice::ReadOnly)) {
        qCritical() << "No such XML file" << filename;
        return QString();
    }

    QByteArray buffer(file.readAll());
    return QString::fromUtf8(buffer.constData(), buffer.size());
}

bool KXMLGUIFactory::saveConfigFile(const QDomDocument &doc,
                                    const QString &filename, const QString &_componentName)
{
    QString componentName = _componentName.isEmpty() ? QCoreApplication::applicationName() : _componentName;
    QString xml_file(filename);

    if (QDir::isRelativePath(xml_file))
        xml_file = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
            QStringLiteral("/kxmlgui5/") + componentName + QLatin1Char('/') + filename;

    QFileInfo fileInfo(xml_file);
    QDir().mkpath(fileInfo.absolutePath());
    QFile file(xml_file);
    if (xml_file.isEmpty() || !file.open(QIODevice::WriteOnly)) {
        qCritical() << "Could not write to" << filename;
        return false;
    }

    // write out our document
    QTextStream ts(&file);
    ts.setCodec(QTextCodec::codecForName("UTF-8"));
    ts << doc;

    file.close();
    return true;
}

/**
 * Removes all QDomComment objects from the specified node and all its children.
 */
/*
static void removeDOMComments(QDomNode &node)
{
    QDomNode n = node.firstChild();
    while (!n.isNull()) {
        if (n.nodeType() == QDomNode::CommentNode) {
            QDomNode tmp = n;
            n = n.nextSibling();
            node.removeChild(tmp);
        } else {
            QDomNode tmp = n;
            n = n.nextSibling();
            removeDOMComments(tmp);
        }
    }
}*/

KXMLGUIFactory::KXMLGUIFactory(KXMLGUIBuilder *builder, QObject *parent)
    : QObject(parent), d(new KXMLGUIFactoryPrivate)
{
    d->builder = builder;
    d->guiClient = 0;
    if (d->builder) {
        d->builderContainerTags = d->builder->containerTags();
        d->builderCustomTags = d->builder->customTags();
    }
}

KXMLGUIFactory::~KXMLGUIFactory()
{
    Q_FOREACH (KXMLGUIClient *client, d->m_clients) {
        client->setFactory(0L);
    }
    delete d;
}

void KXMLGUIFactory::addClient(KXMLGUIClient *client)
{
    debugWidgetUtils << client;
    if (client->factory()) {
        if (client->factory() == this) {
            return;
        } else {
            client->factory()->removeClient(client);    //just in case someone does stupid things ;-)
        }
    }

    if (d->emptyState()) {
        emit makingChanges(true);
    }
    d->pushState();

//    QTime dt; dt.start();

    d->guiClient = client;

    // add this client to our client list
    if (!d->m_clients.contains(client)) {
        d->m_clients.append(client);
    }
    else {
        debugWidgetUtils << "XMLGUI client already added " << client;
    }
    // Tell the client that plugging in is process and
    //  let it know what builder widget its mainwindow shortcuts
    //  should be attached to.
    client->beginXMLPlug(d->builder->widget());

    // try to use the build document for building the client's GUI, as the build document
    // contains the correct container state information (like toolbar positions, sizes, etc.) .
    // if there is non available, then use the "real" document.
    QDomDocument doc = client->xmlguiBuildDocument();
    if (doc.documentElement().isNull()) {
        doc = client->domDocument();
    }

    QDomElement docElement = doc.documentElement();

    d->m_rootNode->index = -1;

    // cache some variables

    d->clientName = docElement.attribute(d->attrName);
    d->clientBuilder = client->clientBuilder();

    if (d->clientBuilder) {
        d->clientBuilderContainerTags = d->clientBuilder->containerTags();
        d->clientBuilderCustomTags = d->clientBuilder->customTags();
    } else {
        d->clientBuilderContainerTags.clear();
        d->clientBuilderCustomTags.clear();
    }

    // load user-defined shortcuts and other action properties
    d->saveDefaultActionProperties(client->actionCollection()->actions());
    if (!doc.isNull()) {
        d->refreshActionProperties(client, client->actionCollection()->actions(), doc);
    }

    BuildHelper(*d, d->m_rootNode).build(docElement);

    // let the client know that we built its GUI.
    client->setFactory(this);

    // call the finalizeGUI method, to fix up the positions of toolbars for example.
    // ### FIXME : obey client builder
    // --- Well, toolbars have a bool "positioned", so it doesn't really matter,
    // if we call positionYourself on all of them each time. (David)
    d->builder->finalizeGUI(d->guiClient);

    // reset some variables, for safety
    d->BuildState::reset();

    client->endXMLPlug();

    d->popState();

    emit clientAdded(client);

    // build child clients
    Q_FOREACH (KXMLGUIClient *child, client->childClients()) {
        addClient(child);
    }

    if (d->emptyState()) {
        emit makingChanges(false);
    }
    /*
        QString unaddedActions;
        Q_FOREACH (KActionCollection* ac, KActionCollection::allCollections())
          Q_FOREACH (QAction* action, ac->actions())
            if (action->associatedWidgets().isEmpty())
              unaddedActions += action->objectName() + ' ';

        if (!unaddedActions.isEmpty())
          qWarning() << "The following actions are not plugged into the gui (shortcuts will not work): " << unaddedActions;
    */

//    qDebug() << "addClient took " << dt.elapsed();
}

// Find the right ActionProperties element, otherwise return null element
static QDomElement findActionPropertiesElement(const QDomDocument &doc)
{
    const QLatin1String tagActionProp("ActionProperties");
    QDomElement e = doc.documentElement().firstChildElement();
    for (; !e.isNull(); e = e.nextSiblingElement()) {
        if (QString::compare(e.tagName(), tagActionProp, Qt::CaseInsensitive) == 0) {
            return e;
        }
    }
    return QDomElement();
}

void KXMLGUIFactoryPrivate::refreshActionProperties(KXMLGUIClient *client, const QList<QAction *> &actions, const QDomDocument &doc)
{
    // These were used for applyShortcutScheme() but not for applyActionProperties()??
    Q_UNUSED(client);
    Q_UNUSED(actions);


    // try to find and apply user-defined shortcuts
    const QDomElement actionPropElement = findActionPropertiesElement(doc);
    if (!actionPropElement.isNull()) {
        applyActionProperties(actionPropElement);
    }
}

void KXMLGUIFactoryPrivate::saveDefaultActionProperties(const QList<QAction *> &actions)
{
    // This method is called every time the user activated a new
    // kxmlguiclient. We only want to execute the following code only once in
    // the lifetime of an action.
    Q_FOREACH (QAction *action, actions) {
        // Skip 0 actions or those we have seen already.
        if (!action || action->property("_k_DefaultShortcut").isValid()) {
            continue;
        }

        // Check if the default shortcut is set
        QList<QKeySequence> defaultShortcut = action->property("defaultShortcuts").value<QList<QKeySequence> >();
        QList<QKeySequence> activeShortcut = action->shortcuts();
        //qDebug() << action->objectName() << "default=" << defaultShortcut.toString() << "active=" << activeShortcut.toString();

        // Check if we have an empty default shortcut and an non empty
        // custom shortcut. Print out a warning and correct the mistake.
        if ((!activeShortcut.isEmpty()) && defaultShortcut.isEmpty()) {
            action->setProperty("_k_DefaultShortcut", QVariant::fromValue(activeShortcut));
        } else {
            action->setProperty("_k_DefaultShortcut", QVariant::fromValue(defaultShortcut));
        }
    }
}

void KXMLGUIFactory::forgetClient(KXMLGUIClient *client)
{
    d->m_clients.removeAll(client);
}

void KXMLGUIFactory::removeClient(KXMLGUIClient *client)
{
    //qDebug(260) << client;

    // don't try to remove the client's GUI if we didn't build it
    if (!client || client->factory() != this) {
        return;
    }

    if (d->emptyState()) {
        emit makingChanges(true);
    }

    // remove this client from our client list
    d->m_clients.removeAll(client);

    // remove child clients first (create a copy of the list just in case the
    // original list is modified directly or indirectly in removeClient())
    const QList<KXMLGUIClient *> childClients(client->childClients());
    Q_FOREACH (KXMLGUIClient *child, childClients) {
        removeClient(child);
    }

    //qDebug(260) << "calling removeRecursive";

    d->pushState();

    // cache some variables

    d->guiClient = client;
    d->clientName = client->domDocument().documentElement().attribute(d->attrName);
    d->clientBuilder = client->clientBuilder();

    client->setFactory(0L);

    // if we don't have a build document for that client, yet, then create one by
    // cloning the original document, so that saving container information in the
    // DOM tree does not touch the original document.
    QDomDocument doc = client->xmlguiBuildDocument();
    if (doc.documentElement().isNull()) {
        doc = client->domDocument().cloneNode(true).toDocument();
        client->setXMLGUIBuildDocument(doc);
    }

    d->m_rootNode->destruct(doc.documentElement(), *d);

    // reset some variables
    d->BuildState::reset();

    // This will destruct the KAccel object built around the given widget.
    client->prepareXMLUnplug(d->builder->widget());

    d->popState();

    if (d->emptyState()) {
        emit makingChanges(false);
    }

    emit clientRemoved(client);
}

QList<KXMLGUIClient *> KXMLGUIFactory::clients() const
{
    return d->m_clients;
}

QWidget *KXMLGUIFactory::container(const QString &containerName, KXMLGUIClient *client,
                                   bool useTagName)
{
    d->pushState();
    d->m_containerName = containerName;
    d->guiClient = client;

    QWidget *result = d->findRecursive(d->m_rootNode, useTagName);

    d->guiClient = 0L;
    d->m_containerName.clear();

    d->popState();

    return result;
}

QList<QWidget *> KXMLGUIFactory::containers(const QString &tagName)
{
    return d->findRecursive(d->m_rootNode, tagName);
}

void KXMLGUIFactory::reset()
{
    d->m_rootNode->reset();

    d->m_rootNode->clearChildren();
}

void KXMLGUIFactory::resetContainer(const QString &containerName, bool useTagName)
{
    if (containerName.isEmpty()) {
        return;
    }

    ContainerNode *container = d->m_rootNode->findContainer(containerName, useTagName);

    if (!container) {
        return;
    }

    ContainerNode *parent = container->parent;
    if (!parent) {
        return;
    }

    //  resetInternal( container );

    parent->removeChild(container);
}

QWidget *KXMLGUIFactoryPrivate::findRecursive(KXMLGUI::ContainerNode *node, bool tag)
{
    if (((!tag && node->name == m_containerName) ||
            (tag && node->tagName == m_containerName)) &&
            (!guiClient || node->client == guiClient)) {
        return node->container;
    }

    Q_FOREACH (ContainerNode *child, node->children) {
        QWidget *cont = findRecursive(child, tag);
        if (cont) {
            return cont;
        }
    }

    return 0L;
}

// Case insensitive equality without calling toLower which allocates a new string
static inline bool equals(const QString &str1, const char *str2)
{
    return str1.compare(QLatin1String(str2), Qt::CaseInsensitive) == 0;
}
static inline bool equals(const QString &str1, const QString &str2)
{
    return str1.compare(str2, Qt::CaseInsensitive) == 0;
}

QList<QWidget *> KXMLGUIFactoryPrivate::findRecursive(KXMLGUI::ContainerNode *node,
        const QString &tagName)
{
    QList<QWidget *> res;

    if (equals(node->tagName, tagName)) {
        res.append(node->container);
    }

    Q_FOREACH (KXMLGUI::ContainerNode *child, node->children) {
        res << findRecursive(child, tagName);
    }

    return res;
}

void KXMLGUIFactory::plugActionList(KXMLGUIClient *client, const QString &name,
                                    const QList<QAction *> &actionList)
{
    d->pushState();
    d->guiClient = client;
    d->actionListName = name;
    d->actionList = actionList;
    d->clientName = client->domDocument().documentElement().attribute(d->attrName);

    d->m_rootNode->plugActionList(*d);

    // Load shortcuts for these new actions
    d->saveDefaultActionProperties(actionList);
    d->refreshActionProperties(client, actionList, client->domDocument());

    d->BuildState::reset();
    d->popState();
}

void KXMLGUIFactory::unplugActionList(KXMLGUIClient *client, const QString &name)
{
    d->pushState();
    d->guiClient = client;
    d->actionListName = name;
    d->clientName = client->domDocument().documentElement().attribute(d->attrName);

    d->m_rootNode->unplugActionList(*d);

    d->BuildState::reset();
    d->popState();
}

void KXMLGUIFactoryPrivate::applyActionProperties(const QDomElement &actionPropElement,
        ShortcutOption shortcutOption)
{
    for (QDomElement e = actionPropElement.firstChildElement();
            !e.isNull(); e = e.nextSiblingElement()) {
        if (!equals(e.tagName(), "action")) {
            continue;
        }

        QAction *action = guiClient->action(e);
        if (!action) {
            continue;
        }

        configureAction(action, e.attributes(), shortcutOption);
    }
}

void KXMLGUIFactoryPrivate::configureAction(QAction *action, const QDomNamedNodeMap &attributes,
        ShortcutOption shortcutOption)
{
    for (int i = 0; i < attributes.length(); i++) {
        QDomAttr attr = attributes.item(i).toAttr();
        if (attr.isNull()) {
            continue;
        }

        configureAction(action, attr, shortcutOption);
    }
}

void KXMLGUIFactoryPrivate::configureAction(QAction *action, const QDomAttr &attribute,
        ShortcutOption shortcutOption)
{
    QString attrName = attribute.name();
    // If the attribute is a deprecated "accel", change to "shortcut".
    if (equals(attrName, "accel")) {
        attrName = QStringLiteral("shortcut");
    }

    // No need to re-set name, particularly since it's "objectName" in Qt4
    if (equals(attrName, "name")) {
        return;
    }

    if (equals(attrName, "icon")) {
        action->setIcon(KisIconUtils::loadIcon(attribute.value()));
        return;
    }

    QVariant propertyValue;

    QVariant::Type propertyType = action->property(attrName.toLatin1().constData()).type();
    bool isShortcut = (propertyType == QVariant::KeySequence);

    if (propertyType == QVariant::Int) {
        propertyValue = QVariant(attribute.value().toInt());
    } else if (propertyType == QVariant::UInt) {
        propertyValue = QVariant(attribute.value().toUInt());
    } else if (isShortcut) {
        // Setting the shortcut by property also sets the default shortcut
        // (which is incorrect), so we have to do it directly
        action->setShortcuts(QKeySequence::listFromString(attribute.value()));
        if (shortcutOption & KXMLGUIFactoryPrivate::SetDefaultShortcut) {
            action->setProperty("defaultShortcuts",
                                QVariant::fromValue(QKeySequence::listFromString(attribute.value())));
        }
    } else {
        propertyValue = QVariant(attribute.value());
    }
    if (!isShortcut && !action->setProperty(attrName.toLatin1().constData(), propertyValue)) {
        qWarning() << "Error: Unknown action property " << attrName << " will be ignored!";
    }
}

// Find or create
QDomElement KXMLGUIFactory::actionPropertiesElement(QDomDocument &doc)
{
    // first, lets see if we have existing properties
    QDomElement elem = findActionPropertiesElement(doc);

    // if there was none, create one
    if (elem.isNull()) {
        elem = doc.createElement(QStringLiteral("ActionProperties"));
        doc.documentElement().appendChild(elem);
    }
    return elem;
}

QDomElement KXMLGUIFactory::findActionByName(QDomElement &elem, const QString &sName, bool create)
{
    const QLatin1String attrName("name");
    for (QDomNode it = elem.firstChild(); !it.isNull(); it = it.nextSibling()) {
        QDomElement e = it.toElement();
        if (e.attribute(attrName) == sName) {
            return e;
        }
    }

    if (create) {
        QDomElement act_elem = elem.ownerDocument().createElement(QStringLiteral("Action"));
        act_elem.setAttribute(attrName, sName);
        elem.appendChild(act_elem);
        return act_elem;
    }
    return QDomElement();
}

