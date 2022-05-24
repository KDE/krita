/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
   SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef kxmlguifactory_h
#define kxmlguifactory_h

#include <kritawidgetutils_export.h>

#include <QObject>

class QAction;
class KisKXMLGUIFactoryPrivate;
class KisKXMLGUIClient;
class KisKXMLGUIBuilder;

class QDomAttr;
class QDomDocument;
class QDomElement;
class QDomNode;
class QDomNamedNodeMap;

namespace KisKXMLGUI
{
struct MergingIndex;
struct ContainerNode;
struct ContainerClient;
class BuildHelper;
}

/**
 * KisKXMLGUIFactory, together with KisKXMLGUIClient objects, can be used to create
 * a GUI of container widgets (like menus, toolbars, etc.) and container items
 * (menu items, toolbar buttons, etc.) from an XML document and action objects.
 *
 * Each KisKXMLGUIClient represents a part of the GUI, composed from containers and
 * actions. KisKXMLGUIFactory takes care of building (with the help of a KisKXMLGUIBuilder)
 * and merging the GUI from an unlimited number of clients.
 *
 * Each client provides XML through a QDomDocument and actions through a
 * KisKActionCollection . The XML document contains the rules for how to merge the
 * GUI.
 *
 * KisKXMLGUIFactory processes the DOM tree provided by a client and plugs in the client's actions,
 * according to the XML and the merging rules of previously inserted clients. Container widgets
 * are built via a KisKXMLGUIBuilder , which has to be provided with the KisKXMLGUIFactory constructor.
 */
class KRITAWIDGETUTILS_EXPORT KisKXMLGUIFactory : public QObject
{
    friend class KisKXMLGUI::BuildHelper;
    Q_OBJECT
public:
    /**
     * Constructs a KisKXMLGUIFactory. The provided @p builder KisKXMLGUIBuilder will be called
     * for creating and removing container widgets, when clients are added/removed from the GUI.
     *
     * Note that the ownership of the given KisKXMLGUIBuilder object won't be transferred to this
     * KisKXMLGUIFactory, so you have to take care of deleting it properly.
     */
    explicit KisKXMLGUIFactory(KisKXMLGUIBuilder *builder, QObject *parent = 0);

    /**
     * Destructor
     */
    ~KisKXMLGUIFactory() override;

    // XXX move to somewhere else? (Simon)
    /// @internal
    static QString readConfigFile(const QString &filename,
                                  const QString &componentName = QString());
    /// @internal
    static bool saveConfigFile(const QDomDocument &doc, const QString &filename,
                               const QString &componentName = QString());

    /**
     * @internal
     * Find or create the ActionProperties element, used when saving custom action properties
     */
    static QDomElement actionPropertiesElement(QDomDocument &doc);

    /**
     * @internal
     * Find or create the element for a given action, by name.
     * Used when saving custom action properties
     */
    static QDomElement findActionByName(QDomElement &elem, const QString &sName, bool create);

    /**
     * Creates the GUI described by the QDomDocument of the client,
     * using the client's actions, and merges it with the previously
     * created GUI.
     * This also means that the order in which clients are added to the factory
     * is relevant; assuming that your application supports plugins, you should
     * first add your application to the factory and then the plugin, so that the
     * plugin's UI is merged into the UI of your application, and not the other
     * way round.
     */
    void addClient(KisKXMLGUIClient *client);

    /**
     * Removes the GUI described by the client, by unplugging all
     * provided actions and removing all owned containers (and storing
     * container state information in the given client)
     */
    void removeClient(KisKXMLGUIClient *client);

    void plugActionList(KisKXMLGUIClient *client, const QString &name, const QList<QAction *> &actionList);
    void unplugActionList(KisKXMLGUIClient *client, const QString &name);

    /**
     * Returns a list of all clients currently added to this factory
     */
    QList<KisKXMLGUIClient *> clients() const;

    /**
     * Use this method to get access to a container widget with the name specified with @p containerName
     * and which is owned by the @p client. The container name is specified with a "name" attribute in the
     * XML document.
     *
     * This function is particularly useful for getting hold of a popupmenu defined in an XMLUI file.
     * For instance:
     * \code
     * QMenu *popup = static_cast<QMenu*>(guiFactory()->container("my_popup",this));
     * \endcode
     * where @p "my_popup" is the name of the menu in the XMLUI file, and
     * @p "this" is XMLGUIClient which owns the popupmenu (e.g. the mainwindow, or the part, or the plugin...)
     *
     * @param containerName Name of the container widget
     * @param client Owner of the container widget
     * @param useTagName Specifies whether to compare the specified name with the name attribute or
     *        the tag name.
     *
     * This method may return 0 if no container with the given name exists or is not owned by the client.
     */
    QWidget *container(const QString &containerName, KisKXMLGUIClient *client, bool useTagName = false);

    QList<QWidget *> containers(const QString &tagName);

    /**
     * Use this method to free all memory allocated by the KisKXMLGUIFactory. This deletes the internal node
     * tree and therefore resets the internal state of the class. Please note that the actual GUI is
     * NOT touched at all, meaning no containers are deleted nor any actions unplugged. That is
     * something you have to do on your own. So use this method only if you know what you are doing :-)
     *
     * (also note that this will call KisKXMLGUIClient::setFactory( 0 ) for all inserted clients)
     */
    void reset();

    /**
     * Use this method to free all memory allocated by the KisKXMLGUIFactory for a specific container,
     * including all child containers and actions. This deletes the internal node subtree for the
     * specified container. The actual GUI is not touched, no containers are deleted or any actions
     * unplugged. Use this method only if you know what you are doing :-)
     *
     * (also note that this will call KisKXMLGUIClient::setFactory( 0 ) for all clients of the
     * container)
     */
    void resetContainer(const QString &containerName, bool useTagName = false);

Q_SIGNALS:
    void clientAdded(KisKXMLGUIClient *client);
    void clientRemoved(KisKXMLGUIClient *client);

    /**
     * Emitted when the factory is currently making changes to the GUI,
     * i.e. adding or removing clients.
     * makingChanges(true) is emitted before any change happens, and
     * makingChanges(false) is emitted after the change is done.
     * This allows e.g. KisKMainWindow to know that the GUI is
     * being changed programmatically and not by the user (so there is no reason to
     * save toolbar settings afterwards).
     * @since 4.1.3
     */
    void makingChanges(bool);

private:
    friend class KisKXMLGUIClient;
    /// Internal, called by KisKXMLGUIClient destructor
    void forgetClient(KisKXMLGUIClient *client);

    KisKXMLGUIFactoryPrivate *const d;
};

#endif

