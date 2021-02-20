/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2001 Simon Hausmann <hausmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef kxmlguifactory_p_h
#define kxmlguifactory_p_h

#include <QStringList>
#include <QMap>
#include <QDomElement>
#include <QStack>
#include <QAction>

class QWidget;
class KXMLGUIClient;
class KXMLGUIBuilder;

namespace KXMLGUI
{

struct BuildState;

class ActionList : public QList<QAction *>
{
public:
    ActionList() {}
    ActionList(const QList<QAction *> &rhs)
        : QList<QAction *>(rhs)
    {}
    ActionList &operator=(const QList<QAction *> &rhs)
    {
        QList<QAction *>::operator=(rhs);
        return *this;
    }

    void plug(QWidget *container, int index) const;
    void unplug(QWidget *container) const;
};

typedef QMap< QString, ActionList > ActionListMap;

/*
 * This structure is used to know to which client certain actions and custom elements
 * (i.e. menu separators) belong.
 * We do not only use a ContainerClient per GUIClient but also per merging group.
 *
 * groupName : Used for grouped merging. Specifies the group name to which these actions/elements
 * belong to.
 * actionLists : maps from action list name to action list.
 * mergingName : The (named) merging point.
 *
 * A ContainerClient always belongs to a ContainerNode.
 */
struct ContainerClient {
    KXMLGUIClient *client;
    ActionList actions;
    QList<QAction *> customElements;
    QString groupName; //is empty if no group client
    ActionListMap actionLists;
    QString mergingName;
};
typedef QList<ContainerClient *> ContainerClientList;

struct ContainerNode;

struct MergingIndex {
    int value; // the actual index value, used as index for plug() or createContainer() calls
    QString mergingName; // the name of the merging index (i.e. the name attribute of the
    // Merge or DefineGroup tag)
    QString clientName; // the name of the client that defined this index
};
typedef QList<MergingIndex> MergingIndexList;

/*
 * Here we store detailed information about a container, its clients (client=a guiclient having actions
 * plugged into the container), child nodes, naming information (tagname and name attribute) and
 * merging index information, to plug/insert new actions/items a the correct position.
 *
 * The builder variable is needed for using the proper GUIBuilder for destruction ( to use the same for
 * con- and destruction ). The builderCustomTags and builderContainerTags variables are cached values
 * of what the corresponding methods of the GUIBuilder which built the container return. The stringlists
 * is shared all over the place, so there's no need to worry about memory consumption for these
 * variables :-)
 *
 * The mergingIndices list contains the merging indices ;-) , as defined by <Merge>, <DefineGroup>
 * or by <ActionList> tags. The order of these index structures within the mergingIndices list
 * is (and has to be) identical with the order in the DOM tree.
 *
 * Beside the merging indices we have the "real" index of the container. It points to the next free
 * position.
 * (used when no merging index is used for a certain action, custom element or sub-container)
 */
struct ContainerNode {
    ContainerNode(QWidget *_container, const QString &_tagName, const QString &_name,
                  ContainerNode *_parent = 0L, KXMLGUIClient *_client = 0L,
                  KXMLGUIBuilder *_builder = 0L, QAction *containerAction = 0,
                  const QString &_mergingName = QString(),
                  const QString &groupName = QString(),
                  const QStringList &customTags = QStringList(),
                  const QStringList &containerTags = QStringList());
    ~ContainerNode();

    ContainerNode *parent;
    KXMLGUIClient *client;
    KXMLGUIBuilder *builder;
    QStringList builderCustomTags;
    QStringList builderContainerTags;
    QWidget *container;
    QAction *containerAction;

    QString tagName;
    QString name;

    QString groupName; //is empty if the container is in no group

    ContainerClientList clients;
    QList<ContainerNode *> children;

    int index;
    MergingIndexList mergingIndices;

    QString mergingName;

    void clearChildren()
    {
        qDeleteAll(children);
        children.clear();
    }
    void removeChild(ContainerNode *child);
    // Removes the child referred to by childIterator.next()
    void removeChild(QMutableListIterator<ContainerNode *> &childIterator);

    MergingIndexList::Iterator findIndex(const QString &name);
    ContainerNode *findContainerNode(QWidget *container);
    ContainerNode *findContainer(const QString &_name, bool tag);
    ContainerNode *findContainer(const QString &name, const QString &tagName,
                                 const QList<QWidget *> *excludeList,
                                 KXMLGUIClient *currClient);

    ContainerClient *findChildContainerClient(KXMLGUIClient *currentGUIClient,
            const QString &groupName,
            const MergingIndexList::Iterator &mergingIdx);

    void plugActionList(BuildState &state);
    void plugActionList(BuildState &state, const MergingIndexList::Iterator &mergingIdxIt);

    void unplugActionList(BuildState &state);
    void unplugActionList(BuildState &state, const MergingIndexList::Iterator &mergingIdxIt);

    void adjustMergingIndices(int offset, const MergingIndexList::Iterator &it);

    bool destruct(QDomElement element, BuildState &state);
    void destructChildren(const QDomElement &element, BuildState &state);
    static QDomElement findElementForChild(const QDomElement &baseElement,
                                           ContainerNode *childNode);
    void unplugActions(BuildState &state);
    void unplugClient(ContainerClient *client);

    void reset();

    int calcMergingIndex(const QString &mergingName,
                         MergingIndexList::Iterator &it,
                         BuildState &state,
                         bool ignoreDefaultMergingIndex);
};

typedef QList<ContainerNode *> ContainerNodeList;

class BuildHelper
{
public:
    BuildHelper(BuildState &state,
                ContainerNode *node);

    void build(const QDomElement &element);

private:
    void processElement(const QDomElement &element);

    void processActionOrCustomElement(const QDomElement &e, bool isActionTag);
    bool processActionElement(const QDomElement &e, int idx);
    bool processCustomElement(const QDomElement &e, int idx);

    void processStateElement(const QDomElement &element);

    void processMergeElement(const QString &tag, const QString &name, const QDomElement &e);

    void processContainerElement(const QDomElement &e, const QString &tag,
                                 const QString &name);

    QWidget *createContainer(QWidget *parent, int index, const QDomElement &element,
                             QAction *&containerAction, KXMLGUIBuilder **builder);

    int calcMergingIndex(const QDomElement &element, MergingIndexList::Iterator &it, QString &group);

    QStringList customTags;
    QStringList containerTags;

    QList<QWidget *> containerList;

    ContainerClient *containerClient;

    bool ignoreDefaultMergingIndex;

    BuildState &m_state;

    ContainerNode *parentNode;
};

struct BuildState {
    BuildState() : guiClient(0), builder(0), clientBuilder(0) {}

    void reset();

    QString clientName;

    QString actionListName;
    ActionList actionList;

    KXMLGUIClient *guiClient;

    MergingIndexList::Iterator currentDefaultMergingIt;
    MergingIndexList::Iterator currentClientMergingIt;

    KXMLGUIBuilder *builder;
    QStringList builderCustomTags;
    QStringList builderContainerTags;

    KXMLGUIBuilder *clientBuilder;
    QStringList clientBuilderCustomTags;
    QStringList clientBuilderContainerTags;
};

typedef QStack<BuildState> BuildStateStack;

}

#endif

