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
    //bool isEphemeral() const override;
    QPair<QString, QString> resourceType() const override;

    QString translatedFontName(QList<QLocale> locales);
    QString translatedTypographicName(QList<QLocale> locales);

    QList<KoSvgText::FontFamilyAxis> axes() const;
    QList<KoSvgText::FontFamilyStyleInfo> styles() const;

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KOFONTFAMILY_H
