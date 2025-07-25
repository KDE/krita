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

    /// The actual text properties.
    KoSvgTextProperties properties(int ppi = 72, bool removeKraProps = false) const;
    void setProperties(const KoSvgTextProperties &properties);

    /// The description associated with this style.
    QString description() const;
    void setDescription(QString description);

    /// Set the style type, type is either "paragraph" or "character".
    QString styleType() const;
    void setStyleType(const QString &type);

    /// The sample text that is being styled by this preset.
    QString sampleText() const;

    /// The text displayed before the sample. Only relevant when in Character mode.
    QString beforeText() const;

    /// The text displayed after the sample, only relevant when in character mode.
    QString afterText() const;

    /// Returns the sample svg metadata. Use updateThumbnail to update it.
    QString sampleSvg() const;

    /**
     * The resolution that this style is tied to.
     * if this is above 0, then the properties absolute values are scaled by
     * to fit the document resolution.
     * This allows for pixel-relative styles to be created.
     */
    int storedPPIResolution() const;

    void setStoredPPIResolution(const int ppi);

    /**
     * @brief setSampleText
     * This allows setting the visible sample text. In effect, this reconstructs the internal text
     * with before, styled and after text.
     * @param before -- unstyled before text, this is only set when the styletype is character.
     * @param sample -- text of the styled sample.
     * @param after -- unstyled after text, this is only set when the styletype is character.
     */
    void setSampleText(const QString &sample, const KoSvgTextProperties &properties, const QString &before = "", const QString &after = "");

    /// Determines the prefered sample alignment based on the text properties. It's set up so that
    /// the alignment anchor of the text is shown.
    Qt::Alignment alignSample() const;

    /**
     * @brief primaryFontFamily
     * If a style uses a FontFamily, it may not look as expected when that
     * font family is missing. Typically, we'd use linked resources for this,
     * however, embedding fonts is really complex.
     * @return the primary font family for this style, will return empty if
     * the style does not require a font family.
     */
    QString primaryFontFamily() const;

    void updateAlignSample();
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
