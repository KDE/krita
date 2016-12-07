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

public:
    explicit Krita(QObject *parent = 0);
    virtual ~Krita();

    Document* activeDocument() const;
    void setActiveDocument(Document* value);

    bool batchmode() const;
    void setBatchmode(bool value);

    QList<Action*> actions() const;
    Action *action(const QString &name) const;

    QList<Document*> documents() const;

    QList<Exporter*> exporters() const;

    QList<Filter*> filters() const;

    QList<Generator *> generators() const;

    QList<Importer *> importers() const;

    Notifier* notifier() const;

    InfoObject* preferences() const;
    void setPreferences(InfoObject* value);

    QString version() const;

    QList<View*> views() const;

    QList<Window *> windows() const;

    QList<Resource*> resources() const;
    void setResources(QList<Resource*> value);


public Q_SLOTS:

    void addDockWidget(DockWidget *dockWidget);

    void addAction(Action *action);

    bool closeApplication();

    Document* createDocument();

    Document* openDocument(const QString &filename);

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
