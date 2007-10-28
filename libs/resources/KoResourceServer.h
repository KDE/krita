/*  This file is part of the KDE project

    Copyright (c) 1999 Matthias Elter <elter@kde.org>
    Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
    Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef KORESOURCESERVER_H
#define KORESOURCESERVER_H

#include <QString>
#include <QStringList>
#include <QList>

#include "KoGenericRegistry.h"
#include <koresource_export.h>

class KoResource;

class KORESOURCES_EXPORT KoResourceServerBase : public QObject {
    Q_OBJECT
public:
    KoResourceServerBase(const QString & type);
    virtual ~KoResourceServerBase();

    void loadResources(QStringList filenames);
    /// Adds an already loaded resource to the server
    void addResource(KoResource* resource);
    /// Remove a resource from resourceserver and hard disk
    void removeResource(KoResource* resource);
    QList<KoResource*> resources();
    QString type() { return m_type; }
    QString id() const { return QString(); }
    QString name() const { return QString(); }

signals:
    void resourceAdded(KoResource*);

protected:
    virtual KoResource* createResource( const QString & filename ) = 0;

private:
    QList<KoResource*> m_resources;
    QString m_type;

    bool m_loaded;

};

template <class T> class KoResourceServer : public KoResourceServerBase {

public:
    KoResourceServer(const QString & type) : KoResourceServerBase(type) {}
    virtual ~KoResourceServer() {}

private:
    KoResource* createResource(const QString & filename) { return new T(filename); }
};

#endif // KORESOURCESERVER_H
