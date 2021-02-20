/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000 Simon Hausmann <hausmann@kde.org>
   SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kxmlguiclient.h"

#include "kxmlguiversionhandler_p.h"
#include "kxmlguifactory.h"
#include "kxmlguibuilder.h"
#include "kactioncollection.h"

#include <QAction>
#include <QDir>
#include <QFile>
#include <QDomDocument>
#include <QTextStream>
#include <QRegExp>
#include <QPointer>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDebug>

#include <klocalizedstring.h>

#include <assert.h>

#if defined(KCONFIG_BEFORE_5_24)
# define authorizeAction authorizeKAction
#endif

class KXMLGUIClientPrivate
{
public:
    KXMLGUIClientPrivate()
        : m_componentName(QCoreApplication::applicationName()),
          m_actionCollection(0),
          m_parent(0L),
          m_builder(0L)
    {
        m_textTagNames.append(QLatin1String("text"));
        m_textTagNames.append(QLatin1String("Text"));
        m_textTagNames.append(QLatin1String("title"));
    }
    ~KXMLGUIClientPrivate()
    {
    }

    bool mergeXML(QDomElement &base, QDomElement &additive,
                  KActionCollection *actionCollection);
    bool isEmptyContainer(const QDomElement &base,
                          KActionCollection *actionCollection) const;

    QDomElement findMatchingElement(const QDomElement &base,
                                    const QDomElement &additive);

    QString m_componentName;

    QDomDocument m_doc;
    KActionCollection *m_actionCollection;
    QDomDocument m_buildDocument;
    QPointer<KXMLGUIFactory> m_factory;
    KXMLGUIClient *m_parent;
    //QPtrList<KXMLGUIClient> m_supers;
    QList<KXMLGUIClient *> m_children;
    KXMLGUIBuilder *m_builder;
    QString m_xmlFile;
    QString m_localXMLFile;
    QStringList m_textTagNames;

    // Actions to enable/disable on a state change
    QMap<QString, KXMLGUIClient::StateChange> m_actionsStateMap;
};

KXMLGUIClient::KXMLGUIClient()
    : d(new KXMLGUIClientPrivate)
{
}

KXMLGUIClient::KXMLGUIClient(KXMLGUIClient *parent)
    : d(new KXMLGUIClientPrivate)
{
    parent->insertChildClient(this);
}

KXMLGUIClient::~KXMLGUIClient()
{
    if (d->m_parent) {
        d->m_parent->removeChildClient(this);
    }

    if (d->m_factory) {
        qWarning() << this << "deleted without having been removed from the factory first. This will leak standalone popupmenus and could lead to crashes.";
        d->m_factory->forgetClient(this);
    }

    Q_FOREACH (KXMLGUIClient *client, d->m_children) {
        if (d->m_factory) {
            d->m_factory->forgetClient(client);
        }
        assert(client->d->m_parent == this);
        client->d->m_parent = 0;
    }

    delete d->m_actionCollection;
    delete d;
}

QAction *KXMLGUIClient::action(const char *name) const
{
    QAction *act = actionCollection()->action(QLatin1String(name));
    if (!act) {
        Q_FOREACH (KXMLGUIClient *client, d->m_children) {
            act = client->actionCollection()->action(QLatin1String(name));
            if (act) {
                break;
            }
        }
    }
    return act;
}

KActionCollection *KXMLGUIClient::actionCollection() const
{
    if (!d->m_actionCollection) {
        d->m_actionCollection = new KActionCollection(this);
        d->m_actionCollection->setObjectName(QStringLiteral("KXMLGUIClient-KActionCollection"));
    }
    return d->m_actionCollection;
}

QAction *KXMLGUIClient::action(const QDomElement &element) const
{
    return actionCollection()->action(element.attribute(QStringLiteral("name")));
}

QString KXMLGUIClient::componentName() const
{
    return d->m_componentName;
}

QDomDocument KXMLGUIClient::domDocument() const
{
    return d->m_doc;
}

QString KXMLGUIClient::xmlFile() const
{
    return d->m_xmlFile;
}

QString KXMLGUIClient::localXMLFile() const
{
    if (!d->m_localXMLFile.isEmpty()) {
        return d->m_localXMLFile;
    }

    if (!QDir::isRelativePath(d->m_xmlFile)) {
        return QString();    // can't save anything here
    }

    if (d->m_xmlFile.isEmpty()) { // setXMLFile not called at all, can't save. Use case: ToolBarHandler
        return QString();
    }

    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/kxmlgui5/") +
           componentName() + QLatin1Char('/') + d->m_xmlFile;
}

void KXMLGUIClient::reloadXML()
{
    // TODO: this method can't be used for the KXmlGuiWindow, since it doesn't merge in ui_standards.xmlgui!
    //   -> KDE5: load ui_standards_rc in setXMLFile using a flag, and remember that flag?
    //            and then KEditToolBar can use reloadXML.
    QString file(xmlFile());
    if (!file.isEmpty()) {
        setXMLFile(file);
    }
}

void KXMLGUIClient::setComponentName(const QString &componentName, const QString &componentDisplayName)
{
    d->m_componentName = componentName;
    actionCollection()->setComponentName(componentName);
    actionCollection()->setComponentDisplayName(componentDisplayName);
    if (d->m_builder) {
        d->m_builder->setBuilderClient(this);
    }
}

QString KXMLGUIClient::standardsXmlFileLocation()
{
    QString file = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, QStringLiteral("ui/ui_standards.xmlgui"));
    if (file.isEmpty()) {
        // fallback to resource, to allow to use the rc file compiled into this framework, must exist!
        file = QStringLiteral(":/kxmlgui5/ui_standards.xmlgui");
        Q_ASSERT(QFile::exists(file));
    }
    return file;
}

void KXMLGUIClient::loadStandardsXmlFile()
{
    setXML(KXMLGUIFactory::readConfigFile(standardsXmlFileLocation()));
}

void KXMLGUIClient::setXMLFile(const QString &_file, bool merge, bool setXMLDoc)
{
    // store our xml file name
    if (!_file.isNull()) {
        d->m_xmlFile = _file;
    }

    if (!setXMLDoc) {
        return;
    }

    QString file = _file;
    QStringList allFiles;
    if (!QDir::isRelativePath(file)) {
        allFiles.append(file);
    } else {
        const QString filter = componentName() + QLatin1Char('/') + _file;

        // files on filesystem
        allFiles << QStandardPaths::locateAll(QStandardPaths::AppDataLocation, QStringLiteral("kxmlgui5/") + filter); // KF >= 5.1

        // KF >= 5.4 (resource file)
        const QString qrcFile(QStringLiteral(":/kxmlgui5/") + filter);
        if (QFile::exists(qrcFile)) {
            allFiles << qrcFile;
        }

        // then compat locations
        const QStringList compatFiles =
                   QStandardPaths::locateAll(QStandardPaths::AppDataLocation, filter) + // kdelibs4, KF 5.0
                   QStandardPaths::locateAll(QStandardPaths::AppDataLocation, _file); // kdelibs4, KF 5.0, caller passes component name

        if (allFiles.isEmpty() && !compatFiles.isEmpty()) {
            qWarning() << "kxmlguiclient: KXMLGUI file found at deprecated location" << compatFiles << "-- please use ${KXMLGUI_INSTALL_DIR} to install this file instead.";
        }
        allFiles += compatFiles;
    }
    if (allFiles.isEmpty() && !_file.isEmpty()) {
        // if a non-empty file gets passed and we can't find it,
        // inform the developer using some debug output
        qWarning() << "cannot find .xmlgui file" << _file << "for component" << componentName();
    }

    // make sure to merge the settings from any file specified by setLocalXMLFile()
    if (!d->m_localXMLFile.isEmpty() && !file.endsWith(QStringLiteral("ui_standards.xmlgui"))) {
        const bool exists = QDir::isRelativePath(d->m_localXMLFile) || QFile::exists(d->m_localXMLFile);
        if (exists && !allFiles.contains(d->m_localXMLFile)) {
            allFiles.prepend(d->m_localXMLFile);
        }
    }

    QString doc;
    if (!allFiles.isEmpty()) {
        file = findMostRecentXMLFile(allFiles, doc);
    }

    // Always call setXML, even on error, so that we don't keep all ui_standards.xmlgui menus.
    setXML(doc, merge);
}

void KXMLGUIClient::setLocalXMLFile(const QString &file)
{
    d->m_localXMLFile = file;
}

void KXMLGUIClient::replaceXMLFile(const QString &xmlfile, const QString &localxmlfile, bool merge)
{
    if (!QDir::isAbsolutePath(xmlfile)) {
        qWarning() << "xml file" << xmlfile << "is not an absolute path";
    }

    setLocalXMLFile(localxmlfile);
    setXMLFile(xmlfile, merge);
}

// The top document element may have translation domain attribute set,
// or the translation domain may be implicitly the application domain.
// This domain must be used to fetch translations for all text elements
// in the document that do not have their own domain attribute.
// In order to preserve this semantics through document mergings,
// the top or application domain must be propagated to all text elements
// lacking their own domain attribute.
static void propagateTranslationDomain(QDomDocument &doc, const QStringList tagNames)
{
    const QLatin1String attrDomain("translationDomain");
    QDomElement base = doc.documentElement();
    QString domain = base.attribute(attrDomain);
    if (domain.isEmpty()) {
        domain = QString::fromUtf8(KLocalizedString::applicationDomain());
        if (domain.isEmpty()) {
            return;
        }
    }
    foreach (const QString &tagName, tagNames) {
        QDomNodeList textNodes = base.elementsByTagName(tagName);
        for (int i = 0; i < textNodes.length(); ++i) {
            QDomElement e = textNodes.item(i).toElement();
            QString localDomain = e.attribute(attrDomain);
            if (localDomain.isEmpty()) {
                 e.setAttribute(attrDomain, domain);
            }
        }
    }
}

void KXMLGUIClient::setXML(const QString &document, bool merge)
{
    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    // QDomDocument raises a parse error on empty document, but we accept no app-specific document,
    // in which case you only get ui_standards.xmlgui layout.
    bool result = document.isEmpty() || doc.setContent(document, &errorMsg, &errorLine, &errorColumn);
    if (result) {
        propagateTranslationDomain(doc, d->m_textTagNames);
        setDOMDocument(doc, merge);
    } else {
#ifdef NDEBUG
        qCritical() << "Error parsing XML document:" << errorMsg << "at line" << errorLine << "column" << errorColumn;
        setDOMDocument(QDomDocument(), merge); // otherwise empty menus from ui_standards.xmlgui stay around
#else
        qCritical() << "Error parsing XML document:" << errorMsg << "at line" << errorLine << "column" << errorColumn;
        abort();
#endif
    }
}

void KXMLGUIClient::setDOMDocument(const QDomDocument &document, bool merge)
{
    if (merge && !d->m_doc.isNull()) {
        QDomElement base = d->m_doc.documentElement();

        QDomElement e = document.documentElement();

        // merge our original (global) xml with our new one
        d->mergeXML(base, e, actionCollection());

        // reassign our pointer as mergeXML might have done something
        // strange to it
        base = d->m_doc.documentElement();

        //qDebug(260) << "Result of xmlgui merging:" << d->m_doc.toString();

        // we want some sort of failsafe.. just in case
        if (base.isNull()) {
            d->m_doc = document;
        }
    } else {
        d->m_doc = document;
    }

    setXMLGUIBuildDocument(QDomDocument());
}

// if (equals(a,b)) is more readable than if (a.compare(b, Qt::CaseInsensitive)==0)
static inline bool equalstr(const QString &a, const QString &b)
{
    return a.compare(b, Qt::CaseInsensitive) == 0;
}
static inline bool equalstr(const QString &a, const QLatin1String &b)
{
    return a.compare(b, Qt::CaseInsensitive) == 0;
}

bool KXMLGUIClientPrivate::mergeXML(QDomElement &base, QDomElement &additive, KActionCollection *actionCollection)
{
    const QLatin1String tagAction("Action");
    const QLatin1String tagMerge("Merge");
    const QLatin1String tagSeparator("Separator");
    const QLatin1String tagMergeLocal("MergeLocal");
    const QLatin1String tagText("text");
    const QLatin1String attrAppend("append");
    const QString       attrName(QStringLiteral("name"));
    const QString       attrWeakSeparator(QStringLiteral("weakSeparator"));
    const QString       attrAlreadyVisited(QStringLiteral("alreadyVisited"));
    const QString       attrNoMerge(QStringLiteral("noMerge"));
    const QLatin1String attrOne("1");

    // there is a possibility that we don't want to merge in the
    // additive.. rather, we might want to *replace* the base with the
    // additive.  this can be for any container.. either at a file wide
    // level or a simple container level.  we look for the 'noMerge'
    // tag, in any event and just replace the old with the new
    if (additive.attribute(attrNoMerge) == attrOne) { // ### use toInt() instead? (Simon)
        base.parentNode().replaceChild(additive, base);
        return true;
    } else {
        // Merge attributes
        {
            const QDomNamedNodeMap attribs = additive.attributes();
            const uint attribcount = attribs.count();

            for (uint i = 0; i < attribcount; ++i) {
                const QDomNode node = attribs.item(i);
                base.setAttribute(node.nodeName(), node.nodeValue());
            }
        }

        // iterate over all elements in the container (of the global DOM tree)
        QDomNode n = base.firstChild();
        while (!n.isNull()) {
            QDomElement e = n.toElement();
            n = n.nextSibling(); // Advance now so that we can safely delete e
            if (e.isNull()) {
                continue;
            }

            const QString tag = e.tagName();

            // if there's an action tag in the global tree and the action is
            // not implemented, then we remove the element
            if (equalstr(tag, tagAction)) {
                const QString name =  e.attribute(attrName);
                if (!actionCollection->action(name)) {
                    // remove this child as we aren't using it
                    base.removeChild(e);
                    continue;
                }
            }

            // if there's a separator defined in the global tree, then add an
            // attribute, specifying that this is a "weak" separator
            else if (equalstr(tag, tagSeparator)) {
                e.setAttribute(attrWeakSeparator, (uint)1);

                // okay, hack time. if the last item was a weak separator OR
                // this is the first item in a container, then we nuke the
                // current one
                QDomElement prev = e.previousSibling().toElement();
                if (prev.isNull() ||
                        (equalstr(prev.tagName(), tagSeparator) && !prev.attribute(attrWeakSeparator).isNull()) ||
                        (equalstr(prev.tagName(), tagText))) {
                    // the previous element was a weak separator or didn't exist
                    base.removeChild(e);
                    continue;
                }
            }

            // the MergeLocal tag lets us specify where non-standard elements
            // of the local tree shall be merged in.  After inserting the
            // elements we delete this element
            else if (equalstr(tag, tagMergeLocal)) {
                QDomNode it = additive.firstChild();
                while (!it.isNull()) {
                    QDomElement newChild = it.toElement();
                    it = it.nextSibling();
                    if (newChild.isNull()) {
                        continue;
                    }

                    if (equalstr(newChild.tagName(), tagText)) {
                        continue;
                    }

                    if (newChild.attribute(attrAlreadyVisited) == attrOne) {
                        continue;
                    }

                    QString itAppend(newChild.attribute(attrAppend));
                    QString elemName(e.attribute(attrName));

                    if ((itAppend.isNull() && elemName.isEmpty()) ||
                            (itAppend == elemName)) {
                        // first, see if this new element matches a standard one in
                        // the global file.  if it does, then we skip it as it will
                        // be merged in, later
                        QDomElement matchingElement = findMatchingElement(newChild, base);
                        if (matchingElement.isNull() || equalstr(newChild.tagName(), tagSeparator)) {
                            base.insertBefore(newChild, e);
                        }
                    }
                }

                base.removeChild(e);
                continue;
            }

            else if (equalstr(tag, tagText)) {
                continue;
            } else if (equalstr(tag, tagMerge)) {
                continue;
            }

            // in this last case we check for a separator tag and, if not, we
            // can be sure that it is a container --> proceed with child nodes
            // recursively and delete the just proceeded container item in
            // case it is empty (if the recursive call returns true)
            else {
                QDomElement matchingElement = findMatchingElement(e, additive);
                if (!matchingElement.isNull()) {
                    matchingElement.setAttribute(attrAlreadyVisited, (uint)1);

                    if (mergeXML(e, matchingElement, actionCollection)) {
                        base.removeChild(e);
                        additive.removeChild(matchingElement); // make sure we don't append it below
                        continue;
                    }

                    continue;
                } else {
                    // this is an important case here! We reach this point if the
                    // "local" tree does not contain a container definition for
                    // this container. However we have to call mergeXML recursively
                    // and make it check if there are actions implemented for this
                    // container. *If* none, then we can remove this container now
                    QDomElement dummy;
                    if (mergeXML(e, dummy, actionCollection)) {
                        base.removeChild(e);
                    }
                    continue;
                }
            }
        }

        //here we append all child elements which were not inserted
        //previously via the LocalMerge tag
        n = additive.firstChild();
        while (!n.isNull()) {
            QDomElement e = n.toElement();
            n = n.nextSibling(); // Advance now so that we can safely delete e
            if (e.isNull()) {
                continue;
            }

            QDomElement matchingElement = findMatchingElement(e, base);

            if (matchingElement.isNull()) {
                base.appendChild(e);
            }
        }

        // do one quick check to make sure that the last element was not
        // a weak separator
        QDomElement last = base.lastChild().toElement();
        if (equalstr(last.tagName(), tagSeparator) &&
                (!last.attribute(attrWeakSeparator).isNull())) {
            base.removeChild(last);
        }
    }

    return isEmptyContainer(base, actionCollection);
}

bool KXMLGUIClientPrivate::isEmptyContainer(const QDomElement &base, KActionCollection *actionCollection) const
{
    // now we check if we are empty (in which case we return "true", to
    // indicate the caller that it can delete "us" (the base element
    // argument of "this" call)
    QDomNode n = base.firstChild();
    while (!n.isNull()) {
        const QDomElement e = n.toElement();
        n = n.nextSibling(); // Advance now so that we can safely delete e
        if (e.isNull()) {
            continue;
        }

        const QString tag = e.tagName();

        if (equalstr(tag, QLatin1String("Action"))) {
            // if base contains an implemented action, then we must not get
            // deleted (note that the actionCollection contains both,
            // "global" and "local" actions)
            if (actionCollection->action(e.attribute(QStringLiteral("name")))) {
                return false;
            }
        } else if (equalstr(tag, QLatin1String("Separator"))) {
            // if we have a separator which has *not* the weak attribute
            // set, then it must be owned by the "local" tree in which case
            // we must not get deleted either
            const QString weakAttr = e.attribute(QStringLiteral("weakSeparator"));
            if (weakAttr.isEmpty() || weakAttr.toInt() != 1) {
                return false;
            }
        }

        else if (equalstr(tag, QLatin1String("merge"))) {
            continue;
        }

        // a text tag is NOT enough to spare this container
        else if (equalstr(tag, QLatin1String("text"))) {
            continue;
        }

        // what's left are non-empty containers! *don't* delete us in this
        // case (at this position we can be *sure* that the container is
        // *not* empty, as the recursive call for it was in the first loop
        // which deleted the element in case the call returned "true"
        else {
            return false;
        }
    }

    return true; // I'm empty, please delete me.
}

QDomElement KXMLGUIClientPrivate::findMatchingElement(const QDomElement &base, const QDomElement &additive)
{
    QDomNode n = additive.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        n = n.nextSibling(); // Advance now so that we can safely delete e -- TODO we don't, so simplify this
        if (e.isNull()) {
            continue;
        }

        const QString tag = e.tagName();
        // skip all action and merge tags as we will never use them
        if (equalstr(tag, QLatin1String("Action"))
                || equalstr(tag, QLatin1String("MergeLocal"))) {
            continue;
        }

        // now see if our tags are equivalent
        if (equalstr(tag, base.tagName()) &&
                e.attribute(QStringLiteral("name")) == base.attribute(QStringLiteral("name"))) {
            return e;
        }
    }

    // nope, return a (now) null element
    return QDomElement();
}

void KXMLGUIClient::setXMLGUIBuildDocument(const QDomDocument &doc)
{
    d->m_buildDocument = doc;
}

QDomDocument KXMLGUIClient::xmlguiBuildDocument() const
{
    return d->m_buildDocument;
}

void KXMLGUIClient::setFactory(KXMLGUIFactory *factory)
{
    d->m_factory = factory;
}

KXMLGUIFactory *KXMLGUIClient::factory() const
{
    return d->m_factory;
}

KXMLGUIClient *KXMLGUIClient::parentClient() const
{
    return d->m_parent;
}

void KXMLGUIClient::insertChildClient(KXMLGUIClient *child)
{
    if (child->d->m_parent) {
        child->d->m_parent->removeChildClient(child);
    }
    d->m_children.append(child);
    child->d->m_parent = this;
}

void KXMLGUIClient::removeChildClient(KXMLGUIClient *child)
{
    assert(d->m_children.contains(child));
    d->m_children.removeAll(child);
    child->d->m_parent = 0;
}

/*bool KXMLGUIClient::addSuperClient( KXMLGUIClient *super )
{
  if ( d->m_supers.contains( super ) )
    return false;
  d->m_supers.append( super );
  return true;
}*/

QList<KXMLGUIClient *> KXMLGUIClient::childClients()
{
    return d->m_children;
}

void KXMLGUIClient::setClientBuilder(KXMLGUIBuilder *builder)
{
    d->m_builder = builder;
}

KXMLGUIBuilder *KXMLGUIClient::clientBuilder() const
{
    return d->m_builder;
}

void KXMLGUIClient::plugActionList(const QString &name, const QList<QAction *> &actionList)
{
    if (!d->m_factory) {
        return;
    }

    d->m_factory->plugActionList(this, name, actionList);
}

void KXMLGUIClient::unplugActionList(const QString &name)
{
    if (!d->m_factory) {
        return;
    }

    d->m_factory->unplugActionList(this, name);
}

QString KXMLGUIClient::findMostRecentXMLFile(const QStringList &files, QString &doc)
{
    KXmlGuiVersionHandler versionHandler(files);
    doc = versionHandler.finalDocument();
    return versionHandler.finalFile();
}

void KXMLGUIClient::addStateActionEnabled(const QString &state,
        const QString &action)
{
    StateChange stateChange = getActionsToChangeForState(state);

    stateChange.actionsToEnable.append(action);
    //qDebug(260) << "KXMLGUIClient::addStateActionEnabled( " << state << ", " << action << ")";

    d->m_actionsStateMap.insert(state, stateChange);
}

void KXMLGUIClient::addStateActionDisabled(const QString &state,
        const QString &action)
{
    StateChange stateChange = getActionsToChangeForState(state);

    stateChange.actionsToDisable.append(action);
    //qDebug(260) << "KXMLGUIClient::addStateActionDisabled( " << state << ", " << action << ")";

    d->m_actionsStateMap.insert(state, stateChange);
}

KXMLGUIClient::StateChange KXMLGUIClient::getActionsToChangeForState(const QString &state)
{
    return d->m_actionsStateMap[state];
}

void KXMLGUIClient::stateChanged(const QString &newstate, KXMLGUIClient::ReverseStateChange reverse)
{
    StateChange stateChange = getActionsToChangeForState(newstate);

    bool setTrue = (reverse == StateNoReverse);
    bool setFalse = !setTrue;

    // Enable actions which need to be enabled...
    //
    for (QStringList::const_iterator it = stateChange.actionsToEnable.constBegin();
            it != stateChange.actionsToEnable.constEnd(); ++it) {

        QAction *action = actionCollection()->action(*it);
        if (action) {
            action->setEnabled(setTrue);
        }
    }

    // and disable actions which need to be disabled...
    //
    for (QStringList::const_iterator it = stateChange.actionsToDisable.constBegin();
            it != stateChange.actionsToDisable.constEnd(); ++it) {

        QAction *action = actionCollection()->action(*it);
        if (action) {
            action->setEnabled(setFalse);
        }
    }

}

void KXMLGUIClient::beginXMLPlug(QWidget *w)
{
    actionCollection()->addAssociatedWidget(w);
    foreach (KXMLGUIClient *client, d->m_children) {
        client->beginXMLPlug(w);
    }
}

void KXMLGUIClient::endXMLPlug()
{
}

void KXMLGUIClient::prepareXMLUnplug(QWidget *w)
{
    actionCollection()->removeAssociatedWidget(w);
    foreach (KXMLGUIClient *client, d->m_children) {
        client->prepareXMLUnplug(w);
    }
}

void KXMLGUIClient::virtual_hook(int, void *)
{
    /*BASE::virtual_hook( id, data );*/
}
