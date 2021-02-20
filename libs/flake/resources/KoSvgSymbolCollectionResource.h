/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>

    SPDX-License-Identifier: LGPL-2.1-or-later

 */
#ifndef KOSVGSYMBOLCOLLECTIONRESOURCE
#define KOSVGSYMBOLCOLLECTIONRESOURCE

#include <QObject>
#include <QColor>
#include <QVector>
#include <QScopedPointer>
#include <QImage>
#include <QPainter>

#include <KoResource.h>

#include <KoShape.h>
#include <KoShapeGroup.h>
#include <KoShapeManager.h>
#include <KoShapePaintingContext.h>

#include "kritaflake_export.h"

struct KRITAFLAKE_EXPORT KoSvgSymbol {
    KoSvgSymbol() {}
    KoSvgSymbol(const QString &_title)
        : title(_title) {}

    KoSvgSymbol(const KoSvgSymbol &rhs)
        : id(rhs.id),
          title(rhs.title),
          shape(rhs.shape->cloneShape())
    {
    }

    ~KoSvgSymbol()
    {
        delete shape;
    }

    QString id;
    QString title;
    KoShape *shape;
    QImage icon();

    bool operator==(const KoSvgSymbol& rhs) const {
        return title == rhs.title;
    }
};

/**
 * Loads an svg file that contains "symbol" objects and creates a collection of those objects.
 */
class KRITAFLAKE_EXPORT KoSvgSymbolCollectionResource : public KoResource
{
public:

    /**
     */
    explicit KoSvgSymbolCollectionResource(const QString &filename);

    /// Create an empty color set
    KoSvgSymbolCollectionResource();
    ~KoSvgSymbolCollectionResource() override;

    KoSvgSymbolCollectionResource(const KoSvgSymbolCollectionResource &rhs);
    KoSvgSymbolCollectionResource &operator=(const KoSvgSymbolCollectionResource &rhs) = delete;
    KoResourceSP clone() const override;

    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;
    bool saveToDevice(QIODevice* dev) const override;

    QString defaultFileExtension() const override;

    QPair<QString, QString> resourceType() const override
    {
        return QPair<QString, QString>(ResourceType::Symbols, "");
    }

    QString title() const;
    QString description() const;
    QString creator() const;
    QString rights() const;
    QString language() const;
    QStringList subjects() const;
    QString license() const;
    QStringList permits() const;

    QVector<KoSvgSymbol *> symbols() const;


private:

    struct Private;
    const QScopedPointer<Private> d;

};
#endif // KoSvgSymbolCollectionResource

