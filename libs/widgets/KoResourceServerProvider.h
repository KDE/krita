/*  This file is part of the KDE project

    Copyright (c) 1999 Matthias Elter <elter@kde.org>
    Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
    Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef KORESOURCESERVERPROVIDER_H
#define KORESOURCESERVERPROVIDER_H

#include <kritawidgets_export.h>

#include <QThread>

#include <WidgetsDebug.h>

#include "KoResourceServer.h"
#include <resources/KoPattern.h>
#include <resources/KoAbstractGradient.h>
#include <resources/KoColorSet.h>
#include <resources/KoAbstractGradient.h>
#include <resources/KoSvgSymbolCollectionResource.h>

/**
 * KoResourceLoaderThread allows threaded loading of the resources of a resource server
 */
class KRITAWIDGETS_EXPORT KoResourceLoaderThread : public QThread {

    Q_OBJECT
public:

    /**
     * Constructs a KoResourceLoaderThread for a server
     * @param server the server the resources will be loaded for
     */
    explicit KoResourceLoaderThread(KoResourceServerBase *server);
    ~KoResourceLoaderThread() override;
    void loadSynchronously();
public Q_SLOTS:
    /**
     * Checks whether the thread has finished loading and waits
     * until it is finished if necessary
     */
    void barrier();

protected:
    /**
     * Overridden from QThread
     */
    void run() override;

private:

    KoResourceServerBase * m_server;
    QStringList m_fileNames;
};


/**
 * Provides default resource servers for gradients, patterns and palettes
 */
class KRITAWIDGETS_EXPORT KoResourceServerProvider : public QObject
{
    Q_OBJECT

public:
    KoResourceServerProvider();
    ~KoResourceServerProvider() override;

    static KoResourceServerProvider* instance();

    KoResourceServer<KoPattern>* patternServer(bool block = true);
    KoResourceServer<KoAbstractGradient>* gradientServer(bool block = true);
    KoResourceServer<KoColorSet>* paletteServer(bool block = true);
    KoResourceServer<KoSvgSymbolCollectionResource>* svgSymbolCollectionServer(bool block = true);

private:
    KoResourceServerProvider(const KoResourceServerProvider&);
    KoResourceServerProvider operator=(const KoResourceServerProvider&);

private:
    struct Private;
    Private* const d;
};

#endif // KORESOURCESERVERPROVIDER_H
