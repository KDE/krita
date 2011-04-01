/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>

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

#ifndef KOSHAPESAVINGCONTEXT_H
#define KOSHAPESAVINGCONTEXT_H

#include "flake_export.h"

#include <QImage>
#include <QTransform>
#include <QTextBlockUserData>

class KoShape;
class KoXmlWriter;
class KoGenStyles;
class KoDataCenterBase;
class KoEmbeddedDocumentSaver;
class KoImageData;
class KoShapeLayer;
class KoStore;
class KoSharedSavingData;
class KoShapeSavingContextPrivate;

/**
 * The set of data for the ODF file format used during saving of a shape.
 */
class FLAKE_EXPORT KoShapeSavingContext
{
public:
    /// The Style used for saving the shape
    enum ShapeSavingOption {
        /**
         * If set the style of family presentation is used, when not set the
         * family graphic is used.
         * See OpenDocument 9.2.15 Common Drawing Shape Attributes / Style
         */
        PresentationShape = 1,
        /**
         * Save the draw:id used for referencing the shape.
         * See OpenDocument 9.2.15 Common Drawing Shape Attributes / ID
         */
        DrawId = 2,
        /**
         * If set the automatic style will be marked as being needed in styles.xml
         */
        AutoStyleInStyleXml = 4,
        /**
         * If set duplicate master pages will be merged to one
         */
        UniqueMasterPages = 8
    };
    Q_DECLARE_FLAGS(ShapeSavingOptions, ShapeSavingOption)

    /**
     * @brief Constructor
     * @param xmlWriter used for writing the xml
     * @param mainStyles for saving the styles
     * @param embeddedSaver for saving embedded documents
     */
    KoShapeSavingContext(KoXmlWriter &xmlWriter, KoGenStyles &mainStyles,
                         KoEmbeddedDocumentSaver &embeddedSaver);
    virtual ~KoShapeSavingContext();

    /**
     * @brief Get the xml writer
     *
     * @return xmlWriter
     */
    KoXmlWriter &xmlWriter();

    /**
     * @brief Set the xml writer
     *
     * Change the xmlWriter that is used in the Context e.g. for saving to styles.xml
     * instead of content.xml
     *
     * @param xmlWriter to use
     */
    void setXmlWriter(KoXmlWriter &xmlWriter);

    /**
     * @brief Get the main styles
     *
     * @return main styles
     */
    KoGenStyles &mainStyles();

    /**
     * @brief Get the embedded document saver
     *
     * @return embedded document saver
     */
    KoEmbeddedDocumentSaver &embeddedSaver();

    /**
     * @brief Check if an option is set
     *
     * @return ture if the option is set, false otherwise
     */
    bool isSet(ShapeSavingOption option) const;

    /**
     * @brief Set the options to use
     *
     * @param options to use
     */
    void setOptions(ShapeSavingOptions options);

    /// add an option to the set of options stored on this context, will leave the other options intact.
    void addOption(ShapeSavingOption option);

    /// remove an option, will leave the other options intact.
    void removeOption(ShapeSavingOption option);

    /**
     * @brief Get the options used
     *
     * @return options used
     */
    ShapeSavingOptions options() const;

    /**
     * @brief Get the draw id for a shape
     *
     * The draw:id is unique for all shapes.
     *
     * @param shape for which the draw id should be returned
     * @param insert if true a new draw id will be generated if there is non yet
     *
     * @return the draw id for the shape or and empty string if it was not found
     */
    QString drawId(const KoShape *shape, bool insert = true);

    /**
     * @brief Clear out all given draw ids
     *
     * This is needed for checking if master pages are the same. In normal saving
     * this should not be called.
     *
     * @see KoPAPastePage::process
     */
    void clearDrawIds();

    /**
     * @brief Get the text id for a sub-item
     *
     * The text:id is unique for all sub-item.
     *
     * @param subitem for which the sub-item id should be returned
     * @param insert if true a new sub-item id will be generated if there is non yet
     *
     * @return the sub-item id for the sub-item or and empty string if it was not found
     */
    QString subId(const QTextBlockUserData *subItem, bool insert = true);

    /**
     * Adds a layer to save into a layer-set in styles.xml according to 9.1.2/9.1.3 odf spec
     * @param layer the layer to save
     */
    void addLayerForSaving(const KoShapeLayer *layer);

    /**
     * Saves the layers added with addLayerForSaving to the xml writer
     */
    void saveLayerSet(KoXmlWriter &xmlWriter) const;

    /**
     * remove all layers
     *
     * This can be used for saving different layer sets per page.
     */
    void clearLayers();

    /**
     * Get the image href under which the image will be saved in the store
     */
    QString imageHref(KoImageData *image);

    /**
     * Get the image href under which the image will be save in the store
     *
     * This should only be used for temporary images that are onle there during
     * saving, e.g. a pixmap representation of a draw:frame
     */
    QString imageHref(QImage &image);

    /**
     * Get the images that needs to be saved to the store
     */
    QMap<qint64, QString> imagesToSave();

    /**
     * Add data center
     */
    void addDataCenter(KoDataCenterBase *dataCenter);

    /**
     * Save the data centers
     *
     * This calls KoDataCenterBase::completeSaving()
     * @returns false if an error occurred, which typically cancels the save.
     */
    bool saveDataCenter(KoStore *store, KoXmlWriter *manifestWriter);

    /**
     * Add shared data
     *
     * This can be use to pass data between shapes on saving. E.g. The presentation page layout
     * styles. With that e.g. the styles only need to be saved once and can be used everywhere
     * without creating them again.
     *
     * The ownership of the added data is passed to the context. The KoShapeSavingContext will
     * delete the added data when it is destroyed.
     *
     * Data inserted for a specific id will not be overwritten by calling addSharedData with
     * the same id again.
     *
     * You get an assertion when the id is already existing.
     *
     * @see KoSharedSavingData
     */
    void addSharedData(const QString &id, KoSharedSavingData *data);

    /**
     * Get the shared data.
     *
     * @see KoSharedLoadingData
     *
     * @param id The id used to identify the shared data.
     * @return The shared data for the id or 0 if there is no shared data for the id.
     */
    KoSharedSavingData *sharedData(const QString &id) const;

    /*
     * Add an offset that will be applied to the shape position when saved
     *
     * This is needed e.g. for shapes anchored to a text shape as the position is
     * saved as offset to the anchor.
     *
     * @param shape The shape for which the offset should be added.
     * @param matrix The offset which should be applied on saving the position.
     */
    void addShapeOffset(const KoShape *shape, const QTransform &matrix);

    /**
     * Remove an offset from the saved offset list
     *
     * @param shape The shape for which the offset should be removed.
     */
    void removeShapeOffset(const KoShape *shape);

    /**
     * Get the offest that will be applied to the shape position when saved.
     *
     * @param shape The shape for which the offset should be get.
     * @return the saved offset or QTransform() when offset is not set.
     */
    QTransform shapeOffset(const KoShape *shape) const;

private:
    KoShapeSavingContextPrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KoShapeSavingContext::ShapeSavingOptions)

#endif // KOSHAPESAVINGCONTEXT_H
