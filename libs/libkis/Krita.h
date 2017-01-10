/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef LIBKIS_KRITA_H
#define LIBKIS_KRITA_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

#include "ViewExtension.h"
#include "Document.h"
#include "Window.h"
#include "View.h"
#include "Action.h"
#include "Notifier.h"

class QAction;

/**
 * Krita is a singleton class that offers the root access to the Krita object hierarchy.
 *
 * The Krita.instance() is aliased as two builtins: Scripter and Application.
 */
class KRITALIBKIS_EXPORT Krita : public QObject
{
    Q_OBJECT

public:
    explicit Krita(QObject *parent = 0);
    virtual ~Krita();

public Q_SLOTS:


    /**
     * @return the currently active document, if there is one.
     */
    Document* activeDocument() const;

    /**
     * @brief setActiveDocument activates the first view that shows the given document
     * @param value the document we want to activate
     */
    void setActiveDocument(Document* value);

    /**
     * @brief batchmode determines whether the script is run in batch mode. If batchmode
     * is true, scripts should now show messageboxes or dialog boxes.
     *
     * Note that this separate from Document.setBatchmode(), which determines whether
     * export/save option dialogs are shown.
     *
     * @return true if the script is run in batchmode
     */
    bool batchmode() const;

    /**
     * @brief setBatchmode sets the the batchmode to @param value; if true, scripts should
     * not show dialogs or messageboxes.
     */
    void setBatchmode(bool value);

    /**
     * @return return a list of all actions for the currently active mainWindow.
     */
    QList<Action*> actions() const;
    Action *action(const QString &name) const;

    /**
     * @return a list of all open Documents
     */
    QList<Document*> documents() const;

    /**
     * @brief Filters are identified by an internal name. This function returns a list
     * of all existing registered filters.
     * @return a list of all registered filters
     */
    QStringList filters() const;

    /**
     * @brief filter construct a Filter object with a default configuration.
     * @param name the name of the filter. Use Krita.instance().filters() to get
     * a list of all possible filters.
     * @return the filter or None if there is no such filter.
     */
    Filter *filter(const QString &name) const;

    /**
     * @brief Create a list of all available generator plugins. Generators are identified
     * by an internal name; this name can be used to construct a Generator object. The
     * Generator object can then be used to create a Node object representing a Fill Layer.
     * @return the list of available generators.
     */
    QStringList generators() const;

    /**
     * @brief generator construct a Generator object with a default configuration.
     * @param name the name of the generator. Use Krita.instance().generators() to get
     * a list of all possible filters.
     * @return the generator or None if there is no such generator.
     */
    Generator *generator(const QString &name) const;

    /**
     * @brief profiles creates a list with the names of all color profiles compatible
     * with the given color model and color depth.
     * @param colorModel A string describing the color model of the image:
     * <ul>
     * <li>A: Alpha mask</li>
     * <li>RGBA: RGB with alpha channel (The actual order of channels is most often BGR!)</li>
     * <li>XYZA: XYZ with alpha channel</li>
     * <li>LABA: LAB with alpha channel</li>
     * <li>CMYKA: CMYK with alpha channel</li>
     * <li>GRAYA: Gray with alpha channel</li>
     * <li>YCbCrA: YCbCr with alpha channel</li>
     * </ul>
     * @param colorDepth A string describing the color depth of the image:
     * <ul>
     * <li>U8: unsigned 8 bits integer, the most common type</li>
     * <li>U16: unsigned 16 bits integer</li>
     * <li>F16: half, 16 bits floating point. Only available if Krita was built with OpenEXR</li>
     * <li>F32: 32 bits floating point</li>
     * </ul>
     * @return a list with valid names
     */
    QStringList profiles(const QString &colorModel, const QString &colorDepth) const;

    /**
     * @brief addProfile load the given profile into the profile registry.
     * @param profilePath the path to the profile.
     * @return true if adding the profile succeeded.
     */
    bool addProfile(const QString &profilePath);

    Notifier* notifier() const;

    InfoObject* preferences() const;
    void setPreferences(InfoObject* value);

    /**
     * @brief version Determine the version of Krita
     *
     * Usage: print(Application.version ())
     *
     * @return the version string including git sha1 if Krita was built from git
     */
    QString version() const;

    /**
     * @return a list of all views. A Document can be shown in more than one view.
     */
    QList<View*> views() const;

    /**
     * @return the currently active window or None if there is no window
     */
    Window * activeWindow() const;

    /**
     * @return a list of all windows
     */
    QList<Window *> windows() const;

    QList<Resource*> resources() const;
    void setResources(QList<Resource*> value);


    void addDockWidget(DockWidget *dockWidget);

    bool closeApplication();

    /**
     * @brief createDocument creates a new document and image and registers the document with the Krita application.
     *
     * The document will have one transparent layer.
     *
     * @param width the width in pixels
     * @param height the height in pixels
     * @param name the name of the image (not the filename of the document)
     * @param colorModel A string describing the color model of the image:
     * <ul>
     * <li>A: Alpha mask</li>
     * <li>RGBA: RGB with alpha channel (The actual order of channels is most often BGR!)</li>
     * <li>XYZA: XYZ with alpha channel</li>
     * <li>LABA: LAB with alpha channel</li>
     * <li>CMYKA: CMYK with alpha channel</li>
     * <li>GRAYA: Gray with alpha channel</li>
     * <li>YCbCrA: YCbCr with alpha channel</li>
     * </ul>
     * @param colorDepth A string describing the color depth of the image:
     * <ul>
     * <li>U8: unsigned 8 bits integer, the most common type</li>
     * <li>U16: unsigned 16 bits integer</li>
     * <li>F16: half, 16 bits floating point. Only available if Krita was built with OpenEXR</li>
     * <li>F32: 32 bits floating point</li>
     * </ul>
     * @param profile The name of an icc profile that is known to Krita. If an empty string is passed, the default is
     * taken.
     * @return the created document.
     */
    Document *createDocument(int width, int height, const QString &name, const QString &colorModel, const QString &colorDepth, const QString &profile);

    /**
     * @brief openDocument creates a new Document, registers it with the Krita application and loads the given file.
     * @param filename the file to open in the document
     * @return the document
     */
    Document *openDocument(const QString &filename);

    Window *openWindow();

    /**
     * @brief createAction creates an action with the given text and passes it to Krita. Every newly created
     *     mainwindow will create an instance of this action.
     * @param text the user-visible text
     * @return  the QAction you can connect a slot to.
     */
    Action *createAction(const QString &text);

    void addViewExtension(ViewExtension* viewExtension);
    QList<ViewExtension*> viewExtensions();

    void addDockWidgetFactory(DockWidgetFactoryBase* factory );

    static Krita* instance();

    static QObject *fromVariant(const QVariant& v);

private:
    struct Private;
    Private *const d;
    static Krita* s_instance;

};

Q_DECLARE_METATYPE(Notifier*);

#endif // LIBKIS_KRITA_H
