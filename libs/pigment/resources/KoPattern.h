/*
    SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KOPATTERN_H
#define KOPATTERN_H

#include <KoResource.h>
#include <kritapigment_export.h>

#include <QMetaType>
#include <QSharedPointer>

class KoPattern;
typedef QSharedPointer<KoPattern> KoPatternSP;


/// Write API docs here
class KRITAPIGMENT_EXPORT KoPattern : public KoResource
{
public:

    /**
     * Creates a new KoPattern object using @p filename.  No file is opened
     * in the constructor, you have to call load.
     *
     * @param filename the file name to save and load from.
     */
    explicit KoPattern(const QString &filename);

    /**
     * Create a new pattern from scratch, without loading it from a file
     *
     * @param image the pattern
     * @param name the name of the pattern
     * @param filename the filename of the pattern (note that this filename does not need to exist)
     */
    KoPattern(const QImage &image, const QString &name, const QString &filename);
    ~KoPattern() override;

    KoPattern(const KoPattern &rhs);
    KoPattern& operator=(const KoPattern& rhs) = delete;
    KoResourceSP clone() const override;


public:

    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;
    bool saveToDevice(QIODevice* dev) const override;

    bool loadPatFromDevice(QIODevice *dev);
    bool savePatToDevice(QIODevice* dev) const;

    qint32 width() const;
    qint32 height() const;

    QString defaultFileExtension() const override;

    QPair<QString, QString> resourceType() const override {
        return QPair<QString, QString>(ResourceType::Patterns, "");
    }

    /**
     * @brief pattern the actual pattern image
     * @return a valid QImage. There are no guarantees to the image format.
     */
    QImage pattern() const;

    bool hasAlpha() const;

    /**
     * Create a copy of this pattern removing all the transparency from
     * it. The fully transparent color becomes 100% black. The name and the
     * filename of the new pattern are kept the same.
     *
     * If hasAlpha() is false, the function just returns a simple clone
     * of this pattern.
     */
    KoPatternSP cloneWithoutAlpha() const;

private:

    void setPatternImage(const QImage& image);
    void checkForAlpha(const QImage& image);

private:
    QImage m_pattern;
    bool m_hasAlpha = false;
};

Q_DECLARE_METATYPE(KoPattern*)

Q_DECLARE_METATYPE(QSharedPointer<KoPattern>)

#endif // KOPATTERN_H

