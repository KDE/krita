/* This file is part of the KDE project
 * Copyright (C) 2008 Peter Simonsson <peter.simonsson@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoOdfCollectionLoader.h"

#include <KoStore.h>
#include <KoOdfReadStore.h>
#include <KoOasisLoadingContext.h>
#include <KoXmlNS.h>
#include <KoShape.h>
#include <KoShapeRegistry.h>
#include <KoShapeLoadingContext.h>

#include <klocale.h>
#include <kdebug.h>

#include <QTimer>
#include <QDir>

KoOdfCollectionLoader::KoOdfCollectionLoader(const QString& path, QObject* parent)
    : QObject(parent)
{
    m_path = path;
    m_odfStore = 0;
    m_shapeLoadingContext = 0;
    m_loadingContext = 0;

    m_loadingTimer = new QTimer(this);
    m_loadingTimer->setInterval(0);
    connect(m_loadingTimer, SIGNAL(timeout()),
            this, SLOT(loadShape()));
}

KoOdfCollectionLoader::~KoOdfCollectionLoader()
{
    delete m_shapeLoadingContext;
    delete m_loadingContext;
    m_shapeLoadingContext = 0;
    m_loadingContext = 0;

    if(m_odfStore)
    {
        delete m_odfStore->store();
        delete m_odfStore;
        m_odfStore = 0;
    }
}

void KoOdfCollectionLoader::load()
{
    QDir dir(m_path);
    m_fileList = dir.entryList(QStringList() << "*.odg", QDir::Files);

    if(m_fileList.isEmpty())
    {
        kError() << "Found no shapes in the collection!" << m_path;
        emit loadingFailed(i18n("Found no shapes in the collection! %1", m_path));
        return;
    }

    nextFile();
}

void KoOdfCollectionLoader::loadShape()
{
    kDebug() << m_shape.tagName();
    KoShape * shape = KoShapeRegistry::instance()->createShapeFromOdf(m_shape, *m_shapeLoadingContext);

    if (shape) {
        if(!shape->parent()) {
            m_shapeList.append(shape);
        }
    }

    m_shape = m_shape.nextSibling().toElement();

    if(m_shape.isNull())
    {
        m_page = m_page.nextSibling().toElement();

        if(m_page.isNull())
        {
            m_loadingTimer->stop();

            if(m_fileList.isEmpty())
            {
                emit loadingFinished();
            }
            else
            {
                nextFile();
            }
        }
        else
        {
            m_shape = m_page.firstChild().toElement();
        }
    }
}

void KoOdfCollectionLoader::nextFile()
{
    delete m_shapeLoadingContext;
    delete m_loadingContext;
    m_shapeLoadingContext = 0;
    m_loadingContext = 0;

    if(m_odfStore)
    {
        delete m_odfStore->store();
        delete m_odfStore;
        m_odfStore = 0;
    }

    QString file = m_fileList.takeFirst();
    KoStore* store = KoStore::createStore(m_path + file, KoStore::Read);

    if(store->bad())
    {
        emit loadingFailed(i18n("Not a valid KOffice file: %1", m_path));
        delete store;
        return;
    }

    store->disallowNameExpansion();
    m_odfStore = new KoOdfReadStore(store);
    QString errorMessage;

    if(!m_odfStore->loadAndParse(errorMessage))
    {
        emit loadingFailed(errorMessage);
        return;
    }

    KoOasisLoadingContext* m_loadingContext = new KoOasisLoadingContext(0, m_odfStore->styles(), m_odfStore->store());
    m_shapeLoadingContext = new KoShapeLoadingContext(*m_loadingContext);

    KoXmlElement content = m_odfStore->contentDoc().documentElement();
    KoXmlElement realBody ( KoDom::namedItemNS( content, KoXmlNS::office, "body" ) );

    if (realBody.isNull()) {
        kError() << "No body tag found!" << endl;
        emit loadingFailed(i18n("No body tag found in file: %1", file));
        return;
    }

    m_body = KoDom::namedItemNS(realBody, KoXmlNS::office, "drawing");

    if (m_body.isNull()) {
        kError() << "No office:drawing tag found!" << endl;
        emit loadingFailed(i18n("No office:drawing tag found in file: %1", file));
        return;
    }

    m_page = m_body.firstChild().toElement();

    if (m_page.isNull()) {
        kError() << "No shapes found!" << endl;
        emit loadingFailed(i18n("No shapes found in file: %1", file));
        return;
    }

    m_shape = m_page.firstChild().toElement();

    if (m_shape.isNull()) {
        kError() << "No shapes found!" << endl;
        emit loadingFailed(i18n("No shapes found in file: %1", file));
        return;
    }

    m_loadingTimer->start();
}

#include "KoOdfCollectionLoader.moc"
