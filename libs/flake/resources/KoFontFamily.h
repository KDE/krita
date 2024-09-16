/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOFONTFAMILY_H
#define KOFONTFAMILY_H

#include <KoResource.h>
#include <KoFFWWSConverter.h>

class KoFontFamily;
typedef QSharedPointer<KoFontFamily> KoFontFamilySP;

/**
 * @brief The KoFontFamily class
 * Abstract representation of a Weight/Width/Slant font family, as determined by KoFFWWSConverter.
 */
class KRITAFLAKE_EXPORT KoFontFamily : public KoResource
{
public:
    explicit KoFontFamily(KoFontFamilyWWSRepresentation representation);

    KoFontFamily(const QString &filename);
    ~KoFontFamily();

    KoFontFamily(const KoFontFamily &rhs);
    KoFontFamily &operator=(const KoFontFamily &rhs) = delete;
    KoResourceSP clone() const override;

    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;

    bool isSerializable() const override;
    QPair<QString, QString> resourceType() const override;

    void updateThumbnail() override;

    /// Returns the typographic family name, if any.
    QString typographicFamily() const;

    /// Return the translated name for a given locale...
    QString translatedFontName(QStringList locales) const;

    /// Font is variable
    bool isVariable() const;
    /// Font has color bitmaps.
    bool colorBitmap() const;
    /// Font has colrv0 layers
    bool colorClrV0() const;
    /// Font has colrv1 layers -- doesn't yet work.
    bool colorClrV1() const;
    /// Font is SVG.
    bool colorSVG() const;

    QList<KoSvgText::FontFamilyAxis> axes() const;
    QList<KoSvgText::FontFamilyStyleInfo> styles() const;

    QDateTime lastModified() const;

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KOFONTFAMILY_H
