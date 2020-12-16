/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000 Simon Hausmann <hausmann@kde.org>
   SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef KXMLGUICLIENT_H
#define KXMLGUICLIENT_H

#include <kritawidgetutils_export.h>

#include <QMap>
#include <QStringList>

class QDomDocument;
class QDomElement;
class QWidget;

class QAction;
class KActionCollection;
class KXMLGUIClientPrivate;
class KXMLGUIFactory;
class KXMLGUIBuilder;

namespace KDEPrivate
{
class KEditToolBarWidget;
}

/**
 *
 * A KXMLGUIClient can be used with KXMLGUIFactory to create a
 * GUI from actions and an XML document, and can be dynamically merged
 * with other KXMLGUIClients.
 */
class KRITAWIDGETUTILS_EXPORT KXMLGUIClient
{
    friend class KDEPrivate::KEditToolBarWidget; // for setXMLFile(3 args)
public:
    /**
     * Constructs a KXMLGUIClient which can be used with a
     * KXMLGUIFactory to create a GUI from actions and an XML document, and
     * which can be dynamically merged with other KXMLGUIClients.
     */
    KXMLGUIClient();

    /**
     * Constructs a KXMLGUIClient which can be used with a KXMLGUIFactory
     * to create a GUI from actions and an XML document,
     * and which can be dynamically merged with other KXMLGUIClients.
     *
     * This constructor takes an additional @p parent argument, which makes
     * the client a child client of the parent.
     *
     * Child clients are automatically added to the GUI if the parent is added.
     *
     */
    explicit KXMLGUIClient(KXMLGUIClient *parent);

    /**
     * Destructs the KXMLGUIClient.
     *
     * If the client was in a factory, the factory is NOT informed about the client
     * being removed. This is a feature, it makes window destruction fast (the xmlgui
     * is not updated for every client being deleted), but if you want to simply remove
     * one client and to keep using the window, make sure to call factory->removeClient(client)
     * before deleting the client.
     */
    virtual ~KXMLGUIClient();

    /**
     * Retrieves an action of the client by name.  If not found, it looks in its child clients.
     * This method is provided for convenience, as it uses actionCollection()
     * to get the action object.
     */
    QAction *action(const char *name) const;

    /**
     * Retrieves an action for a given QDomElement. The default
     * implementation uses the "name" attribute to query the action
     * object via the other action() method.
     */
    virtual QAction *action(const QDomElement &element) const;

    /**
     * Retrieves the entire action collection for the GUI client.
     */
    virtual KActionCollection *actionCollection() const;

    /**
     * @return The component name for this GUI client.
     */
    virtual QString componentName() const;

    /**
     * @return The parsed XML in a QDomDocument, set by
     * setXMLFile() or setXML().
     * This document describes the layout of the GUI.
     */
    virtual QDomDocument domDocument() const;

    /**
     * This will return the name of the XML file as set by setXMLFile().
     * If setXML() is used directly, then this will return 0.
     *
     * The filename that this returns is obvious for components as each
     * component has exactly one XML file.  In non-components, however,
     * there are usually two: the global file and the local file.  This
     * function doesn't really care about that, though.  It will always
     * return the last XML file set.  This, in almost all cases, will
     * be the local XML file.
     *
     * @return The name of the XML file or QString()
     */
    virtual QString xmlFile() const;

    virtual QString localXMLFile() const;

    /**
     * @internal
     */
    void setXMLGUIBuildDocument(const QDomDocument &doc);
    /**
     * @internal
     */
    QDomDocument xmlguiBuildDocument() const;

    /**
     * This method is called by the KXMLGUIFactory as soon as the client
     * is added to the KXMLGUIFactory's GUI.
     */
    void setFactory(KXMLGUIFactory *factory);
    /**
     * Retrieves a pointer to the KXMLGUIFactory this client is
     * associated with (will return 0 if the client's GUI has not been built
     * by a KXMLGUIFactory.
     */
    KXMLGUIFactory *factory() const;

    /**
     * KXMLGUIClients can form a simple child/parent object tree. This
     * method returns a pointer to the parent client or 0 if it has no
     * parent client assigned.
     */
    KXMLGUIClient *parentClient() const;

    /**
     * Use this method to make a client a child client of another client.
     * Usually you don't need to call this method, as it is called
     * automatically when using the second constructor, which takes a
     * parent argument.
     */
    void insertChildClient(KXMLGUIClient *child);

    /**
     * Removes the given @p child from the client's children list.
     */
    void removeChildClient(KXMLGUIClient *child);

    /**
     * Retrieves a list of all child clients.
     */
    QList<KXMLGUIClient *> childClients();

    /**
     * A client can have an own KXMLGUIBuilder.
     * Use this method to assign your builder instance to the client (so that the
     * KXMLGUIFactory can use it when building the client's GUI)
     *
     * Client specific guibuilders are useful if you want to create
     * custom container widgets for your GUI.
     */
    void setClientBuilder(KXMLGUIBuilder *builder);

    /**
     * Retrieves the client's GUI builder or 0 if no client specific
     * builder has been assigned via setClientBuilder()
     */
    KXMLGUIBuilder *clientBuilder() const;

    /**
     * Forces this client to re-read its XML resource file.  This is
     * intended to be used when you know that the resource file has
     * changed and you will soon be rebuilding the GUI. This will only have
     * an effect if the client is then removed and re-added to the factory.
     *
     * This method is only for child clients, do not call it for a mainwindow!
     * For a mainwindow, use loadStandardsXmlFile + setXmlFile(xmlFile()) instead.
     */
    void reloadXML();

    /**
     * ActionLists are a way for XMLGUI to support dynamic lists of
     * actions.  E.g. if you are writing a file manager, and there is a
     * menu file whose contents depend on the mimetype of the file that
     * is selected, then you can achieve this using ActionLists. It
     * works as follows:
     * In your xxxui.xmlgui file ( the one that you set in setXMLFile() / pass to setupGUI()
     * ), you put a tag <tt>\<ActionList name="xxx"\></tt>.
     *
     * Example:
     * \code
     * <gui name="xxx_part" version="1">
     * <MenuBar>
     *   <Menu name="file">
     *     ...  <!-- some useful actions-->
     *     <ActionList name="xxx_file_actionlist" />
     *     ...  <!-- even more useful actions-->
     *   </Menu>
     *   ...
     * </MenuBar>
     * </gui>
     * \endcode
     *
     * This tag will get expanded to a list of actions.  In the example
     * above ( a file manager with a dynamic file menu ), you would call
     * \code
     * QList<QAction*> file_actions;
     * for( ... )
     *   if( ... )
     *     file_actions.append( cool_action );
     * unplugActionList( "xxx_file_actionlist" );
     * plugActionList( "xxx_file_actionlist", file_actions );
     * \endcode
     * every time a file is selected, unselected or ...
     *
     * \note You should not call createGUI() after calling this
     *       function.  In fact, that would remove the newly added
     *       actionlists again...
     * \note Forgetting to call unplugActionList() before
     *       plugActionList() would leave the previous actions in the
     *       menu too..
     * \see unplugActionList()
     */
    void plugActionList(const QString &name, const QList<QAction *> &actionList);

    /**
     * Unplugs the action list \p name from the XMLGUI.
     * Calling this removes the specified action list, i.e. this is the
     * complement to plugActionList(). See plugActionList() for a more
     * detailed example.
     * \see plugActionList()
     */
    void unplugActionList(const QString &name);

    static QString findMostRecentXMLFile(const QStringList &files, QString &doc);

    void addStateActionEnabled(const QString &state, const QString &action);

    void addStateActionDisabled(const QString &state, const QString &action);

    enum ReverseStateChange { StateNoReverse, StateReverse };
    struct StateChange {
        QStringList actionsToEnable;
        QStringList actionsToDisable;
    };

    StateChange getActionsToChangeForState(const QString &state);

    void beginXMLPlug(QWidget *);
    void endXMLPlug();
    void prepareXMLUnplug(QWidget *);

    /**
     * Sets a new xmlFile() and localXMLFile(). The purpose of this public
     * method is to allow non-inherited objects to replace the ui definition
     * of an embedded client with a customized version. It corresponds to the
     * usual calls to setXMLFile() and setLocalXMLFile().
     *
     * @param xmlfile The xml file to use. Contrary to setXMLFile(), this
     *                must be an absolute file path.
     * @param localxmlfile The local xml file to set. This should be the full path
     *                    to a writeable file, usually using QStandardPaths::writableLocation.
     *                    You can set this to QString(), but no user changes to shortcuts / toolbars
     *                    will be possible in this case. @see setLocalXMLFile()
     * @param merge Whether to merge with the global document
     *
     * @note If in any doubt whether you need this or not, use setXMLFile()
     *       and setLocalXMLFile(), instead of this function.
     * @note Just like setXMLFile(), this function has to be called before
     *       the client is added to a KXMLGUIFactory in order to have an
     *       effect.
     *
     * @since 4.4
    */
    void replaceXMLFile(const QString &xmlfile, const QString &localxmlfile, bool merge = false);

protected:
    /**
     * Sets the component name for this part.
     *
     * Call this first in the inherited class constructor.
     * (At least before setXMLFile().)
     * @param componentName the name of the directory where the XMLGUI files will be found
     * @param componentDisplayName a user-visible name (e.g. for the toolbar editor)
     */
    virtual void setComponentName(const QString &componentName, const QString &componentDisplayName);

    /**
     * Sets the name of the rc file containing the XML for the part.
     *
     * Call this in the inherited class constructor, for parts and plugins.
     * NOTE: for mainwindows, don't call this, pass the name of the xml file
     * to KXmlGuiWindow::setupGUI() or KXmlGuiWindow::createGUI().
     *
     * @param file Either an absolute path for the file, or simply the
     *             filename. See below for details.
     *             If you pass an absolute path here, make sure to also call
     *             setLocalXMLFile, otherwise toolbar editing won't work.
     * @param merge Whether to merge with the global document.
     * @param setXMLDoc Specify whether to call setXML. Default is true.
     *
     * The preferred way to call this method is with a simple filename for the @p file argument.
     *
     * Since KF 5.1, the file will then be assumed to be installed in DATADIR/kxmlgui5/, under a directory
     * named after the component name.
     * You should use ${KXMLGUI_INSTALL_DIR}/componentname in your CMakeLists.txt file, to install
     * the .xmlgui file(s).
     *
     * Since KF 5.4, the file will then be assumed to be installed in a Qt resource in :/kxmlgui5/,
     * under a directory named after the component name.
     *
     * Compatibility notes:
     * Fallback lookups exist to older locations: DATADIR/componentname/file and DATADIR/file.
     * The latter was there so that setXMLFile("componentname/filename") worked (but this was
     * undocumented). Do not do this anymore after porting to KXMLGUI_INSTALL_DIR, use
     * setComponentName("componentname") and setXMLFile("filename").
     **/
    void setXMLFile(const QString &file, bool merge = false, bool setXMLDoc = true);

    /**
     * Return the full path to the ui_standards.xmlgui, might return a resource path.
     * @return full path to ui_standards.xmlgui, always non-empty.
     * @since 5.16
     */
    static QString standardsXmlFileLocation();

    /**
     * Load the ui_standards.xmlgui file. Usually followed by setXMLFile(xmlFile, true), for merging.
     * @since 4.6
     */
    void loadStandardsXmlFile();

    /**
     * Set the full path to the "local" xml file, the one used for saving
     * toolbar and shortcut changes. You normally don't need to call this,
     * if you pass a simple filename to setXMLFile.
     */
    void setLocalXMLFile(const QString &file);

    /**
     * Sets the XML for the part.
     *
     * Call this in the Part-inherited class constructor if you
     *  don't call setXMLFile().
     **/
    void setXML(const QString &document, bool merge = false);

    /**
     * Sets the Document for the part, describing the layout of the GUI.
     *
     * Call this in the Part-inherited class constructor if you don't call
     * setXMLFile or setXML.
     *
     * WARNING: using this method is not recommended. Many code paths
     * lead to reloading from the XML file on disk. And editing toolbars requires
     * that the result is written to disk anyway, and loaded from there the next time.
     *
     * For application-specific changes to a client's XML, it is a better idea to
     * save the modified dom document to an app/default-client.xml and define a local-xml-file
     * to something specific like app/local-client.xml, using replaceXMLFile.
     * See kdepimlibs/kontactinterface/plugin.cpp for an example.
     */
    void setDOMDocument(const QDomDocument &document, bool merge = false);

    /**
     * Actions can collectively be assigned a "State". To accomplish this
     * the respective actions are tagged as \<enable\> or \<disable\> in
     * a \<State\> \</State\> group of the XMLfile. During program execution the
     * programmer can call stateChanged() to set actions to a defined state.
     *
     * @param newstate Name of a State in the XMLfile.
     * @param reverse If the flag reverse is set to StateReverse, the State is reversed.
     * (actions to be enabled will be disabled and action to be disabled will be enabled)
     * Default is reverse=false.
     */
    virtual void stateChanged(const QString &newstate, ReverseStateChange reverse = StateNoReverse);

    // KDE5 TODO: virtual void loadActionLists() {}, called when the guiclient is added to the xmlgui factory

protected:
    virtual void virtual_hook(int id, void *data);

private:
    KXMLGUIClientPrivate *const d;
};

#endif
