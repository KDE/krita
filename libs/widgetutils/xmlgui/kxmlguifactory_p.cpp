/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2001 Simon Hausmann <hausmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kxmlguifactory_p.h"

#include "kxmlguiclient.h"
#include "kxmlguibuilder.h"
#include "ktoolbar.h"

#include <QWidget>
#include <QDebug>

#include <assert.h>

using namespace KXMLGUI;

void ActionList::plug(QWidget *container, int index) const
{
    QAction *before = 0L; // Insert after end of widget's current actions (default).

    if ((index < 0) || (index > container->actions().count())) {
        qWarning() << "Index " << index << " is not within range (0 - " << container->actions().count();
    } else if (index != container->actions().count()) {
        before = container->actions().at(index);    // Insert before indexed action.
    }

    Q_FOREACH (QAction *action, *this) {
        container->insertAction(before, action);
        // before = action; // BUG FIX: do not insert actions in reverse order.
    }
}

void ActionList::unplug(QWidget *container) const
{
    Q_FOREACH (QAction *action, *this) {
        if (container->actions().contains(action)) {
            container->removeAction(action);
        }
    }
}

ContainerNode::ContainerNode(QWidget *_container, const QString &_tagName,
                             const QString &_name, ContainerNode *_parent,
                             KXMLGUIClient *_client, KXMLGUIBuilder *_builder,
                             QAction *_containerAction, const QString &_mergingName,
                             const QString &_groupName, const QStringList &customTags,
                             const QStringList &containerTags)
    : parent(_parent), client(_client), builder(_builder),
      builderCustomTags(customTags), builderContainerTags(containerTags),
      container(_container), containerAction(_containerAction), tagName(_tagName), name(_name),
      groupName(_groupName), index(0), mergingName(_mergingName)
{
    if (parent) {
        parent->children.append(this);
    }
}

ContainerNode::~ContainerNode()
{
    qDeleteAll(children);
    qDeleteAll(clients);
}

void ContainerNode::removeChild(ContainerNode *child)
{
    MergingIndexList::Iterator mergingIt = findIndex(child->mergingName);
    adjustMergingIndices(-1, mergingIt);
    children.removeAll(child);
    delete child;
}

void ContainerNode::removeChild(QMutableListIterator<ContainerNode *> &childIterator)
{
    MergingIndexList::Iterator mergingIt = findIndex(childIterator.peekNext()->mergingName);
    adjustMergingIndices(-1, mergingIt);
    delete childIterator.next();
    childIterator.remove();
}

/*
 * Find a merging index with the given name. Used to find an index defined by <Merge name="blah"/>
 * or by a <DefineGroup name="foo" /> tag.
 */
MergingIndexList::Iterator ContainerNode::findIndex(const QString &name)
{
    MergingIndexList::Iterator it(mergingIndices.begin());
    MergingIndexList::Iterator end(mergingIndices.end());
    for (; it != end; ++it)
        if ((*it).mergingName == name) {
            return it;
        }
    return it;
}

/*
 * Check if the given container widget is a child of this node and return the node structure
 * if fonud.
 */
ContainerNode *ContainerNode::findContainerNode(QWidget *container)
{
    Q_FOREACH (ContainerNode *child, children)
        if (child->container == container) {
            return child;
        }

    return 0L;
}

/*
 * Find a container recursively with the given name. Either compares _name with the
 * container's tag name or the value of the container's name attribute. Specified by
 * the tag bool .
 */
ContainerNode *ContainerNode::findContainer(const QString &_name, bool tag)
{
    if ((tag && tagName == _name) ||
            (!tag && name == _name)) {
        return this;
    }

    Q_FOREACH (ContainerNode *child, children) {
        ContainerNode *res = child->findContainer(_name, tag);
        if (res) {
            return res;
        }
    }

    return 0;
}

/*
 * Finds a child container node (not recursively) with the given name and tagname. Explicitly
 * leaves out container widgets specified in the exludeList . Also ensures that the containers
 * belongs to currClient.
 */
ContainerNode *ContainerNode::findContainer(const QString &name, const QString &tagName,
        const QList<QWidget *> *excludeList,
        KXMLGUIClient * /*currClient*/)
{
    ContainerNode *res = 0L;
    ContainerNodeList::ConstIterator nIt = children.constBegin();

    if (!name.isEmpty()) {
        for (; nIt != children.constEnd(); ++nIt)
            if ((*nIt)->name == name &&
                    !excludeList->contains((*nIt)->container)) {
                res = *nIt;
                break;
            }

        return res;
    }

    if (!tagName.isEmpty())
        for (; nIt != children.constEnd(); ++nIt) {
            if ((*nIt)->tagName == tagName &&
                    !excludeList->contains((*nIt)->container)
                    /*
                     * It is a bad idea to also compare the client, because
                     * we don't want to do so in situations like these:
                     *
                     * <MenuBar>
                     *   <Menu>
                     *     ...
                     *
                     * other client:
                     * <MenuBar>
                     *   <Menu>
                     *    ...
                     *
                    && (*nIt)->client == currClient )
                    */
               ) {
                res = *nIt;
                break;
            }
        }

    return res;
}

ContainerClient *ContainerNode::findChildContainerClient(KXMLGUIClient *currentGUIClient,
        const QString &groupName,
        const MergingIndexList::Iterator &mergingIdx)
{
    if (!clients.isEmpty()) {
        Q_FOREACH (ContainerClient *client, clients)
            if (client->client == currentGUIClient) {
                if (groupName.isEmpty()) {
                    return client;
                }

                if (groupName == client->groupName) {
                    return client;
                }
            }
    }

    ContainerClient *client = new ContainerClient;
    client->client = currentGUIClient;
    client->groupName = groupName;

    if (mergingIdx != mergingIndices.end()) {
        client->mergingName = (*mergingIdx).mergingName;
    }

    clients.append(client);

    return client;
}

void ContainerNode::plugActionList(BuildState &state)
{
    MergingIndexList::Iterator mIt(mergingIndices.begin());
    MergingIndexList::Iterator mEnd(mergingIndices.end());
    for (; mIt != mEnd; ++mIt) {
        plugActionList(state, mIt);
    }

    Q_FOREACH (ContainerNode *child, children) {
        child->plugActionList(state);
    }
}

void ContainerNode::plugActionList(BuildState &state, const MergingIndexList::Iterator &mergingIdxIt)
{
    static const QString &tagActionList = QString::fromLatin1("actionlist");

    MergingIndex mergingIdx = *mergingIdxIt;

    QString k(mergingIdx.mergingName);

    if (k.indexOf(tagActionList) == -1) {
        return;
    }

    k = k.mid(tagActionList.length());

    if (mergingIdx.clientName != state.clientName) {
        return;
    }

    if (k != state.actionListName) {
        return;
    }

    ContainerClient *client = findChildContainerClient(state.guiClient,
                              QString(),
                              mergingIndices.end());

    client->actionLists.insert(k, state.actionList);

    state.actionList.plug(container, mergingIdx.value);

    adjustMergingIndices(state.actionList.count(), mergingIdxIt);
}

void ContainerNode::unplugActionList(BuildState &state)
{
    MergingIndexList::Iterator mIt(mergingIndices.begin());
    MergingIndexList::Iterator mEnd(mergingIndices.end());
    for (; mIt != mEnd; ++mIt) {
        unplugActionList(state, mIt);
    }

    Q_FOREACH (ContainerNode *child, children) {
        child->unplugActionList(state);
    }
}

void ContainerNode::unplugActionList(BuildState &state, const MergingIndexList::Iterator &mergingIdxIt)
{
    static const QString &tagActionList = QString::fromLatin1("actionlist");

    MergingIndex mergingIdx = *mergingIdxIt;

    QString k = mergingIdx.mergingName;

    if (k.indexOf(tagActionList) == -1) {
        return;
    }

    k = k.mid(tagActionList.length());

    if (mergingIdx.clientName != state.clientName) {
        return;
    }

    if (k != state.actionListName) {
        return;
    }

    ContainerClient *client = findChildContainerClient(state.guiClient,
                              QString(),
                              mergingIndices.end());

    ActionListMap::Iterator lIt(client->actionLists.find(k));
    if (lIt == client->actionLists.end()) {
        return;
    }

    lIt.value().unplug(container);

    adjustMergingIndices(-int(lIt.value().count()), mergingIdxIt);

    client->actionLists.erase(lIt);
}

void ContainerNode::adjustMergingIndices(int offset,
        const MergingIndexList::Iterator &it)
{
    MergingIndexList::Iterator mergingIt = it;
    MergingIndexList::Iterator mergingEnd = mergingIndices.end();

    for (; mergingIt != mergingEnd; ++mergingIt) {
        (*mergingIt).value += offset;
    }

    index += offset;
}

bool ContainerNode::destruct(QDomElement element, BuildState &state)   //krazy:exclude=passbyvalue (this is correct QDom usage, and a ref wouldn't allow passing doc.documentElement() as argument)
{
    destructChildren(element, state);

    unplugActions(state);

    // remove all merging indices the client defined
    QMutableListIterator<MergingIndex> cmIt = mergingIndices;
    while (cmIt.hasNext())
        if (cmIt.next().clientName == state.clientName) {
            cmIt.remove();
        }

    // ### check for merging index count, too?
    if (clients.count() == 0 && children.count() == 0 && container &&
            client == state.guiClient) {
        QWidget *parentContainer = 0L;

        if (parent && parent->container) {
            parentContainer = parent->container;
        }

        assert(builder);

        builder->removeContainer(container, parentContainer, element, containerAction);

        client = 0L;

        return true;
    }

    if (client == state.guiClient) {
        client = 0L;
    }

    return false;

}

void ContainerNode::destructChildren(const QDomElement &element, BuildState &state)
{
    QMutableListIterator<ContainerNode *> childIt = children;
    while (childIt.hasNext()) {
        ContainerNode *childNode = childIt.peekNext();

        QDomElement childElement = findElementForChild(element, childNode);

        // destruct returns true in case the container really got deleted
        if (childNode->destruct(childElement, state)) {
            removeChild(childIt);
        } else {
            childIt.next();
        }
    }
}

QDomElement ContainerNode::findElementForChild(const QDomElement &baseElement,
        ContainerNode *childNode)
{
    // ### slow
    for (QDomNode n = baseElement.firstChild(); !n.isNull();
            n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (e.tagName().toLower() == childNode->tagName &&
                e.attribute(QStringLiteral("name")) == childNode->name) {
            return e;
        }
    }

    return QDomElement();
}

void ContainerNode::unplugActions(BuildState &state)
{
    if (!container) {
        return;
    }

    QMutableListIterator<ContainerClient *> clientIt(clients);

    /*
        Disabled because it means in KToolBar::saveState isHidden is always true then,
        which is clearly wrong.

    if ( clients.count() == 1 && clientIt.current()->client == client &&
         client == state.guiClient )
        container->hide(); // this container is going to die, that's for sure.
                           // in this case let's just hide it, which makes the
                           // destruction faster
     */

    while (clientIt.hasNext())
        //only unplug the actions of the client we want to remove, as the container might be owned
        //by a different client
        if (clientIt.peekNext()->client == state.guiClient) {
            unplugClient(clientIt.peekNext());
            delete clientIt.next();
            clientIt.remove();
        } else {
            clientIt.next();
        }
}

void ContainerNode::unplugClient(ContainerClient *client)
{
    assert(builder);

    // now quickly remove all custom elements (i.e. separators) and unplug all actions

    QList<QAction *>::ConstIterator custIt = client->customElements.constBegin();
    QList<QAction *>::ConstIterator custEnd = client->customElements.constEnd();
    for (; custIt != custEnd; ++custIt) {
        builder->removeCustomElement(container, *custIt);
    }

    KToolBar *bar = qobject_cast<KToolBar *>(container);
    if (bar) {
        bar->removeXMLGUIClient(client->client);
    }

    client->actions.unplug(container);

    // now adjust all merging indices

    MergingIndexList::Iterator mergingIt = findIndex(client->mergingName);

    adjustMergingIndices(- int(client->actions.count()
                               + client->customElements.count()),
                         mergingIt);

    // unplug all actionslists

    ActionListMap::ConstIterator alIt = client->actionLists.constBegin();
    ActionListMap::ConstIterator alEnd = client->actionLists.constEnd();
    for (; alIt != alEnd; ++alIt) {
        alIt.value().unplug(container);

        // construct the merging index key (i.e. like named merging) , find the
        // corresponding merging index and adjust all indices
        QString mergingKey = alIt.key();
        mergingKey.prepend(QStringLiteral("actionlist"));

        MergingIndexList::Iterator mIt = findIndex(mergingKey);
        if (mIt == mergingIndices.end()) {
            continue;
        }

        adjustMergingIndices(-int(alIt.value().count()), mIt);

        // remove the actionlists' merging index
        // ### still needed? we clean up below anyway?
        mergingIndices.erase(mIt);
    }
}

void ContainerNode::reset()
{
    Q_FOREACH (ContainerNode *child, children) {
        child->reset();
    }

    if (client) {
        client->setFactory(0L);
    }
}

int ContainerNode::calcMergingIndex(const QString &mergingName,
                                    MergingIndexList::Iterator &it,
                                    BuildState &state,
                                    bool ignoreDefaultMergingIndex)
{
    MergingIndexList::Iterator mergingIt;

    if (mergingName.isEmpty()) {
        mergingIt = findIndex(state.clientName);
    } else {
        mergingIt = findIndex(mergingName);
    }

    MergingIndexList::Iterator mergingEnd = mergingIndices.end();
    it = mergingEnd;

    if ((mergingIt == mergingEnd && state.currentDefaultMergingIt == mergingEnd) ||
            ignoreDefaultMergingIndex) {
        return index;
    }

    if (mergingIt != mergingEnd) {
        it = mergingIt;
    } else {
        it = state.currentDefaultMergingIt;
    }

    return (*it).value;
}

int BuildHelper::calcMergingIndex(const QDomElement &element, MergingIndexList::Iterator &it, QString &group)
{
    const QLatin1String attrGroup("group");

    bool haveGroup = false;
    group = element.attribute(attrGroup);
    if (!group.isEmpty()) {
        group.prepend(attrGroup);
        haveGroup = true;
    }

    int idx;
    if (haveGroup) {
        idx = parentNode->calcMergingIndex(group, it, m_state, ignoreDefaultMergingIndex);
    } else if (m_state.currentClientMergingIt == parentNode->mergingIndices.end()) {
        idx = parentNode->index;
    } else {
        idx = (*m_state.currentClientMergingIt).value;
    }

    return idx;
}

BuildHelper::BuildHelper(BuildState &state, ContainerNode *node)
    : containerClient(0), ignoreDefaultMergingIndex(false), m_state(state),
      parentNode(node)
{
    // create a list of supported container and custom tags
    customTags = m_state.builderCustomTags;
    containerTags = m_state.builderContainerTags;

    if (parentNode->builder != m_state.builder) {
        customTags += parentNode->builderCustomTags;
        containerTags += parentNode->builderContainerTags;
    }

    if (m_state.clientBuilder) {
        customTags = m_state.clientBuilderCustomTags + customTags;
        containerTags = m_state.clientBuilderContainerTags + containerTags;
    }

    m_state.currentDefaultMergingIt = parentNode->findIndex(QStringLiteral("<default>"));
    parentNode->calcMergingIndex(QString(), m_state.currentClientMergingIt,
                                 m_state, /*ignoreDefaultMergingIndex*/ false);
}

void BuildHelper::build(const QDomElement &element)
{
    for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (e.isNull()) {
            continue;
        }
        processElement(e);
    }
}

void BuildHelper::processElement(const QDomElement &e)
{
    QString tag(e.tagName().toLower());
    QString currName(e.attribute(QStringLiteral("name")));

    bool isActionTag = (tag == QStringLiteral("action"));

    if (isActionTag || customTags.indexOf(tag) != -1) {
        processActionOrCustomElement(e, isActionTag);
    } else if (containerTags.indexOf(tag) != -1) {
        processContainerElement(e, tag, currName);
    } else if (tag == QStringLiteral("merge") || tag == QLatin1String("definegroup")
               || tag == QStringLiteral("actionlist")) {
        processMergeElement(tag, currName, e);
    } else if (tag == QStringLiteral("state")) {
        processStateElement(e);
    }
}

void BuildHelper::processActionOrCustomElement(const QDomElement &e, bool isActionTag)
{
    if (!parentNode->container) {
        return;
    }

    MergingIndexList::Iterator it(m_state.currentClientMergingIt);

    QString group;
    int idx = calcMergingIndex(e, it, group);

    containerClient = parentNode->findChildContainerClient(m_state.guiClient, group, it);

    bool guiElementCreated = false;
    if (isActionTag) {
        guiElementCreated = processActionElement(e, idx);
    } else {
        guiElementCreated = processCustomElement(e, idx);
    }

    if (guiElementCreated)
        // adjust any following merging indices and the current running index for the container
    {
        parentNode->adjustMergingIndices(1, it);
    }
}

bool BuildHelper::processActionElement(const QDomElement &e, int idx)
{
    assert(m_state.guiClient);

    // look up the action and plug it in
    QAction *action = m_state.guiClient->action(e);

    //qDebug(260) << "BuildHelper::processActionElement " << e.attribute( "name" ) << " -> " << action << " (in " << m_state.guiClient->actionCollection() << ")";
    if (!action) {
        return false;
    }

    QAction *before = 0L;
    if (idx >= 0 && idx < parentNode->container->actions().count()) {
        before = parentNode->container->actions()[idx];
    }

    parentNode->container->insertAction(before, action);

    // save a reference to the plugged action, in order to properly unplug it afterwards.
    containerClient->actions.append(action);

    return true;
}

bool BuildHelper::processCustomElement(const QDomElement &e, int idx)
{
    assert(parentNode->builder);

    QAction *action = parentNode->builder->createCustomElement(parentNode->container, idx, e);
    if (!action) {
        return false;
    }

    containerClient->customElements.append(action);
    return true;
}

void BuildHelper::processStateElement(const QDomElement &element)
{
    QString stateName = element.attribute(QStringLiteral("name"));

    if (stateName.isNull() || !stateName.length()) {
        return;
    }

    for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (e.isNull()) {
            continue;
        }

        QString tagName = e.tagName().toLower();

        if (tagName != QStringLiteral("enable") && tagName != QLatin1String("disable")) {
            continue;
        }

        bool processingActionsToEnable = (tagName == QStringLiteral("enable"));

        // process action names
        for (QDomNode n2 = n.firstChild(); !n2.isNull(); n2 = n2.nextSibling()) {
            QDomElement actionEl = n2.toElement();
            if (actionEl.tagName().toLower() != QStringLiteral("action")) {
                continue;
            }

            QString actionName = actionEl.attribute(QStringLiteral("name"));
            if (actionName.isEmpty()) {
                return;
            }

            if (processingActionsToEnable) {
                m_state.guiClient->addStateActionEnabled(stateName, actionName);
            } else {
                m_state.guiClient->addStateActionDisabled(stateName, actionName);
            }

        }
    }
}

void BuildHelper::processMergeElement(const QString &tag, const QString &name, const QDomElement &e)
{
    const QLatin1String tagDefineGroup("definegroup");
    const QLatin1String tagActionList("actionlist");
    const QLatin1String defaultMergingName("<default>");
    const QLatin1String attrGroup("group");

    QString mergingName(name);
    if (mergingName.isEmpty()) {
        if (tag == tagDefineGroup) {
            qCritical() << "cannot define group without name!" << endl;
            return;
        }
        if (tag == tagActionList) {
            qCritical() << "cannot define actionlist without name!" << endl;
            return;
        }
        mergingName = defaultMergingName;
    }

    if (tag == tagDefineGroup) {
        mergingName.prepend(attrGroup);    //avoid possible name clashes by prepending
    }
    // "group" to group definitions
    else if (tag == tagActionList) {
        mergingName.prepend(tagActionList);
    }

    if (parentNode->findIndex(mergingName) != parentNode->mergingIndices.end()) {
        return;    //do not allow the redefinition of merging indices!
    }

    MergingIndexList::Iterator mIt(parentNode->mergingIndices.end());

    QString group(e.attribute(attrGroup));
    if (!group.isEmpty()) {
        group.prepend(attrGroup);
    }

    // calculate the index of the new merging index. Usually this does not need any calculation,
    // we just want the last available index (i.e. append) . But in case the <Merge> tag appears
    // "inside" another <Merge> tag from a previously build client, then we have to use the
    // "parent's" index. That's why we call calcMergingIndex here.
    MergingIndex newIdx;
    newIdx.value = parentNode->calcMergingIndex(group, mIt, m_state, ignoreDefaultMergingIndex);
    newIdx.mergingName = mergingName;
    newIdx.clientName = m_state.clientName;

    // if that merging index is "inside" another one, then append it right after the "parent" .
    if (mIt != parentNode->mergingIndices.end()) {
        parentNode->mergingIndices.insert(++mIt, newIdx);
    } else {
        parentNode->mergingIndices.append(newIdx);
    }

    if (mergingName == defaultMergingName)

    {
        ignoreDefaultMergingIndex = true;
    }

    // re-calculate the running default and client merging indices.
    m_state.currentDefaultMergingIt = parentNode->findIndex(defaultMergingName);
    parentNode->calcMergingIndex(QString(), m_state.currentClientMergingIt,
                                 m_state, ignoreDefaultMergingIndex);
}

void BuildHelper::processContainerElement(const QDomElement &e, const QString &tag,
        const QString &name)
{
    ContainerNode *containerNode = parentNode->findContainer(name, tag,
                                   &containerList,
                                   m_state.guiClient);

    if (!containerNode) {
        MergingIndexList::Iterator it(m_state.currentClientMergingIt);
        QString group;

        int idx = calcMergingIndex(e, it, group);

        QAction *containerAction;

        KXMLGUIBuilder *builder;

        QWidget *container = createContainer(parentNode->container, idx, e, containerAction, &builder);

        // no container? (probably some <text> tag or so ;-)
        if (!container) {
            return;
        }

        parentNode->adjustMergingIndices(1, it);

        assert(!parentNode->findContainerNode(container));

        containerList.append(container);

        QString mergingName;
        if (it != parentNode->mergingIndices.end()) {
            mergingName = (*it).mergingName;
        }

        QStringList cusTags = m_state.builderCustomTags;
        QStringList conTags = m_state.builderContainerTags;
        if (builder != m_state.builder) {
            cusTags = m_state.clientBuilderCustomTags;
            conTags = m_state.clientBuilderContainerTags;
        }

        containerNode = new ContainerNode(container, tag, name, parentNode,
                                          m_state.guiClient, builder, containerAction,
                                          mergingName, group, cusTags, conTags);
    } else {
        if (tag == QStringLiteral("toolbar")) {
            KToolBar *bar = qobject_cast<KToolBar *>(containerNode->container);
            if (bar) {
                if (m_state.guiClient && !m_state.guiClient->xmlFile().isEmpty()) {
                    bar->addXMLGUIClient(m_state.guiClient);
                }
            } else {
                qWarning() << "toolbar container is not a KToolBar";
            }
        }
    }

    BuildHelper(m_state, containerNode).build(e);

    // and re-calculate running values, for better performance
    m_state.currentDefaultMergingIt = parentNode->findIndex(QStringLiteral("<default>"));
    parentNode->calcMergingIndex(QString(), m_state.currentClientMergingIt,
                                 m_state, ignoreDefaultMergingIndex);
}

QWidget *BuildHelper::createContainer(QWidget *parent, int index,
                                      const QDomElement &element, QAction *&containerAction,
                                      KXMLGUIBuilder **builder)
{
    QWidget *res = 0L;

    if (m_state.clientBuilder) {
        res = m_state.clientBuilder->createContainer(parent, index, element, containerAction);

        if (res) {
            *builder = m_state.clientBuilder;
            return res;
        }
    }

    KXMLGUIClient *oldClient = m_state.builder->builderClient();

    m_state.builder->setBuilderClient(m_state.guiClient);

    res = m_state.builder->createContainer(parent, index, element, containerAction);

    m_state.builder->setBuilderClient(oldClient);

    if (res) {
        *builder = m_state.builder;
    }

    return res;
}

void BuildState::reset()
{
    clientName.clear();
    actionListName.clear();
    actionList.clear();
    guiClient = 0;
    clientBuilder = 0;

    currentDefaultMergingIt = currentClientMergingIt = MergingIndexList::Iterator();
}

