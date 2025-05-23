/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOCSSSTYLEPRESET_H
#define KOCSSSTYLEPRESET_H

#include <KoResource.h>
#include <KoSvgTextProperties.h>
#include <kritaflake_export.h>

class KoCssStylePreset;
typedef QSharedPointer<KoCssStylePreset> KoCssStylePresetSP;

/**
 * @brief The KoCssStylePreset class
 *
 * This is a Resource that represents style data.
 * Internally, the style data is stored inside a text shape,
 * allowing us to showcase the style data in context.
 */
class KRITAFLAKE_EXPORT KoCssStylePreset : public KoResource
{
public:
    KoCssStylePreset(const QString &filename);
    KoCssStylePreset(const KoCssStylePreset &rhs);
    KoCssStylePreset &operator=(const KoCssStylePreset &rhs) = delete;
    ~KoCssStylePreset();

    KoSvgTextProperties properties();
    void setProperties(const KoSvgTextProperties &properties);

    /// The description associated with this style.
    QString description() const;
    void setDescription(QString description);

    QString styleType() const;
    void setStyleType(const QString &type);

    // KoResource interface
public:
    KoResourceSP clone() const override;
    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;
    bool saveToDevice(QIODevice *dev) const override;
    QString defaultFileExtension() const override;
    void updateThumbnail() override;
    QPair<QString, QString> resourceType() const override;
private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KOCSSSTYLEPRESET_H
