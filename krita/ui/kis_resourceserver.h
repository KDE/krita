/*
 *  kis_resourceserver.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Sven Langkamp <longamp@reallygood.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_RESOURCESERVER_H_
#define KIS_RESOURCESERVER_H_

#include <QString>
#include <QStringList>
#include <QList>

#include "KoGenericRegistry.h"
#include <krita_export.h>

class KisResource;

class KRITAUI_EXPORT KisResourceServerBase : public QObject {
    Q_OBJECT
public:
    KisResourceServerBase(QString type);
    virtual ~KisResourceServerBase();

    void loadResources(QStringList filenames);
    /// Adds an already loaded resource to the server
    void addResource(KisResource* resource);
    QList<KisResource*> resources();
    QString type() { return m_type; }

signals:
    void resourceAdded(KisResource*);

protected:
    virtual KisResource* createResource( QString filename ) = 0;

private:
    QList<KisResource*> m_resources;
    QString m_type;

    bool m_loaded;

};

template <class T> class KisResourceServer : public KisResourceServerBase {
    typedef KisResourceServerBase super;

public:
    KisResourceServer(QString type) :super(type) {}
    virtual ~KisResourceServer() {}

private:
    KisResource* createResource(QString filename) { return new T(filename); }
};

class KRITAUI_EXPORT KisResourceServerRegistry : public KoGenericRegistry<KisResourceServerBase*>
{
public:
    virtual ~KisResourceServerRegistry();

    static KisResourceServerRegistry* instance();

private:
    KisResourceServerRegistry();
    KisResourceServerRegistry(const KisResourceServerRegistry&);
    KisResourceServerRegistry operator=(const KisResourceServerRegistry&);

    static KisResourceServerRegistry *m_singleton;
};

#endif // KIS_RESOURCESERVER_H_
