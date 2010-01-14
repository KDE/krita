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
#ifndef KOODFCOLLECTIONLOADER_H
#define KOODFCOLLECTIONLOADER_H

#include <kurl.h>

#include <KoXmlReader.h>

#include <QObject>
#include <QList>
#include <QStringList>

class KoOdfReadStore;
class KoOdfLoadingContext;
class KoShapeLoadingContext;
class QTimer;
class KoShape;
class KoFilterManager;

class OdfCollectionLoader : public QObject
{
    Q_OBJECT
    public:
        explicit OdfCollectionLoader(const QString& path, QObject* parent = 0);
        ~OdfCollectionLoader();

        void load();

        QList<KoShape*> shapeList() const { return m_shapeList; }
        QString collectionPath() const { return m_path; }

    protected:
        void nextFile();
        void loadNativeFile(const QString& path);
        QString findMimeTypeByUrl(const KUrl& url);

    protected slots:
        void loadShape();

    private:
        KoOdfReadStore* m_odfStore;
        QTimer* m_loadingTimer;
        KoOdfLoadingContext* m_loadingContext;
        KoShapeLoadingContext* m_shapeLoadingContext;
        KoXmlElement m_body;
        KoXmlElement m_page;
        KoXmlElement m_shape;
        QList<KoShape*> m_shapeList;
        QString m_path;
        QStringList m_fileList;
        KoFilterManager* m_filterManager;

    signals:
        /**
         * Emitted when the loading failed
         * @param reason Reason the loading failed.
         */
        void loadingFailed(const QString& reason);

        void loadingFinished();
};

#endif //KOODFCOLLECTIONLOADER_H
