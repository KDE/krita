/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2014-2015 Denis Kuplyakov <dener.kup@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOSHAPELOADINGCONTEXT_H
#define KOSHAPELOADINGCONTEXT_H

#include <QSet>
#include <QString>
#include <QPair>

#include "kritaflake_export.h"

class KoOdfLoadingContext;
class KoShapeLayer;
class KoShape;
class KoShapeControllerBase;
class KoLoadingShapeUpdater;
class KoImageCollection;
class KoSharedLoadingData;
class KoDocumentResourceManager;
class KoSectionModel;
class QVariant;
class QObject;

/**
 * Context passed to shapes during loading.
 * This class holds various variables as well as a context full of variables which all together
 * form the context of a loading operation.
 */
class KRITAFLAKE_EXPORT KoShapeLoadingContext
{
public:
    /**
     * Struct to store data about additional attributes that should be loaded during
     * the shape loading.
     *
     * Make sure all parameters point to const char * that stay around. e.g. The a KoXmlNS or
     * a "tag" defined string e.g.
     * AdditionalAttributeData( KoXmlNS::presentation, "placeholder", presentation:placeholder" )
     */
    struct AdditionalAttributeData {
        AdditionalAttributeData(const QString &ns, const QString &tag, const QString &name)
                : ns(ns)
                , tag(tag)
                , name(name) {
        }

        const QString ns;
        const QString tag;
        const QString name;

        bool operator==(const AdditionalAttributeData &other) const {
            return name == other.name;
        }
    };

    /**
     * constructor
     * @param context the context created for generic ODF loading.
     * @param documentResources the data of the shape controller.
     */
    KoShapeLoadingContext(KoOdfLoadingContext &context, KoDocumentResourceManager *documentResources);

    /// destructor
    ~KoShapeLoadingContext();

    /// return the embedded loading context
    KoOdfLoadingContext &odfLoadingContext();

    /// Returns layer referenced by given name
    KoShapeLayer *layer(const QString &layerName);
    /// Adds a new layer to be referenced by the given name later
    void addLayer(KoShapeLayer *layer, const QString &layerName);

    /**
     * remove all layers
     *
     * This can be used for loading different layer sets per page.
     */
    void clearLayers();

    /// register the id for a specific shape
    void addShapeId(KoShape *shape, const QString &id);
    /// return the shape formerly registered using addShapeId()
    KoShape *shapeById(const QString &id);

    /// register the id for a specific shape sub item
    void addShapeSubItemId(KoShape *shape, const QVariant &subItem, const QString &id);
    /// return the shape and subitem formerly registered using addShapeSubItemId()
    QPair<KoShape *, QVariant> shapeSubItemById(const QString &id);

    /**
     * call function on the shapeUpdater when the shape with the id shapeid is inserted
     * After that destroy the updater.
     */
    void updateShape(const QString &id, KoLoadingShapeUpdater *shapeUpdater);

    /**
     * this checks if there is an updater for this shape if yes it calls it
     * this needs to be done via the shape id and
     */
    void shapeLoaded(KoShape *shape);

    /// Returns the image collection for loading images
    KoImageCollection *imageCollection();

    /// Get current z-index
    int zIndex();

    /// Set z-index
    void setZIndex(int index);

    /**
     * Add shared data
     *
     * This can be use to pass data between shapes on loading. E.g. The decoded text styles
     * of the TextShape. With that the styles only have to be read once and can be used in
     * all shapes that also need them.
     *
     * The ownership of the added data is passed to the context. The KoShapeLoadingContext will
     * delete the added data when it is destroyed.
     *
     * Data inserted for a specific id will not be overwritten by calling addSharedData with
     * the same id again.
     *
     * You get an assertion when the id is already existing.
     *
     * @see KoSharedLoadingData
     */
    void addSharedData(const QString &id, KoSharedLoadingData *data);

    /**
     * Get the shared data.
     *
     * @see KoSharedLoadingData
     *
     * @param id The id used to identify the shared data.
     * @return The shared data for the id or 0 if there is no shared data for the id.
     */
    KoSharedLoadingData *sharedData(const QString &id) const;

    /**
     * @brief Add an additional attribute that should be loaded during shape loading
     *
     * An application can use that to set the data for additional attributes that should be
     * loaded during shape loading.
     * If attribute is set it will not change if set again. The tag is used to differentiate
     * the attributes
     *
     * @param attributeData The data describing the additional attribute data
     */
    static void addAdditionalAttributeData(const AdditionalAttributeData &attributeData);

    /**
     * @brief Get the additional attribute data for loading of a shape
     *
     * This is used by KoShape::loadOdfAttributes to load all additional attributes defined
     * in the returned set.
     */
    static QSet<AdditionalAttributeData> additionalAttributeData();

    KoDocumentResourceManager *documentResourceManager() const;

    /**
     * @brief get the rdf document
     * @return the rdf document, or 0 if there is none set/
     */
    QObject *documentRdf() const;

    /**
     * @brief setDocumentRdf sets the rdf document for the loading context
     * @param documentRdf the rdf document -- it needs to have been loaded already
     */
    void setDocumentRdf(QObject *documentRdf);

    /**
     * @brief returns the current section model
     * @return the pointer to KoSectionModel
     */
    KoSectionModel *sectionModel();

    /**
     * @brief sets the section model for the loading context
     * @param sectionModel the section model to set
     */
    void setSectionModel(KoSectionModel *sectionModel);

private:
    // to allow only the KoShapeRegistry access to the KoShapeControllerBase
    class Private;
    Private * const d;
};

#endif /* KOSHAPELOADINGCONTEXT_H */
