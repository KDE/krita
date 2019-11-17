/*  This file is part of the KDE project
    Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>

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
#include <KoViewConverter.h>
#include <KoShapePaintingContext.h>

#include "kritaflake_export.h"

struct KRITAFLAKE_EXPORT KoSvgSymbol {
    KoSvgSymbol() {}
    KoSvgSymbol(const QString &_title)
        : title(_title) {}

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
    KoSvgSymbolCollectionResource &operator=(const KoSvgSymbolCollectionResource &rhs);
    KoResourceSP clone() const override;

    bool load() override;
    bool loadFromDevice(QIODevice *dev) override;
    bool save() override;
    bool saveToDevice(QIODevice* dev) const override;

    QString defaultFileExtension() const override;

    QString resourceType() const override
    {
        return ResourceType::Symbols;
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

