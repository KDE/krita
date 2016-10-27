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
 */
class KRITALIBKIS_EXPORT Krita : public QObject
{
    Q_OBJECT

    /**
     * The actions list returns a map of Action objects that belong to the currently
     * active Window. You can retrieve a certain action by name from the map.
     *
     * This code will retrieve all actions, and first use the Select All action to
     * select everything, and the Deselect action to deselect everything *in the
     * current window, the current view, the current image, on the current layer*.
     *
     * @code
     * actions = Krita.instance().actions()
     * print(actions.keys())
     * actions["select_all"].trigger()
     * actions["deselect"].trigger()
     * @endcode
     */
    Q_PROPERTY(QVariantMap Actions READ actions)
    Q_PROPERTY(Document* ActiveDocument READ activeDocument WRITE setActiveDocument)
    Q_PROPERTY(bool Batchmode READ batchmode WRITE setBatchmode)
    Q_PROPERTY(QList<Document*> Documents READ documents)
    Q_PROPERTY(QList<Exporter*> Exporters READ exporters)
    Q_PROPERTY(QList<Filter*> Filters READ filters)
    Q_PROPERTY(QList<Generator*> Generators READ generators)
    Q_PROPERTY(QList<Importer*> Importers READ importers)
    Q_PROPERTY(Notifier* Notifier READ notifier)
    Q_PROPERTY(InfoObject* Preferences READ preferences WRITE setPreferences)
    Q_PROPERTY(QString Version READ version)
    Q_PROPERTY(QList<View*> Views READ views)
    Q_PROPERTY(QList<Window*> Windows READ windows)
    Q_PROPERTY(QList<Resource*> Resources READ resources)

public:
    explicit Krita(QObject *parent = 0);
    virtual ~Krita();

    Document* activeDocument() const;
    void setActiveDocument(Document* value);

    bool batchmode() const;
    void setBatchmode(bool value);

    QVariantMap actions() const;

    QList<Document*> documents() const;

    QList<Exporter*> exporters() const;

    QList<Filter*> filters() const;

    QList<Generator*> generators() const;

    QList<Importer*> importers() const;

    Notifier* notifier() const;

    InfoObject* preferences() const;
    void setPreferences(InfoObject* value);

    QString version() const;

    QList<View*> views() const;

    QList<Window*> windows() const;

    QList<Resource*> resources() const;
    void setResources(QList<Resource*> value);


public Q_SLOTS:

    void addDockWidget(DockWidget *dockWidget);

    void addAction(Action *action);

    bool closeApplication();

    Document* createDocument();

    Document* openDocument();

    Window* openWindow();

    QAction *createAction(const QString &text);

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
