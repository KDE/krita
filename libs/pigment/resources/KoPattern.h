/*
    Copyright (c) 2000 Matthias Elter  <elter@kde.org>

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
    KoPattern(const QImage &image, const QString &name, const QString &folderName);
    ~KoPattern() override;

    KoPattern(const KoPattern &rhs);
    KoPattern& operator=(const KoPattern& rhs);
    KoResourceSP clone() const;


public:

    bool load() override;
    bool loadFromDevice(QIODevice *dev) override;
    bool save() override;
    bool saveToDevice(QIODevice* dev) const override;

    bool loadPatFromDevice(QIODevice *dev);
    bool savePatToDevice(QIODevice* dev) const;

    qint32 width() const;
    qint32 height() const;

    QString defaultFileExtension() const override;

    QString resourceType() const override
    {
        return ResourceType::Patterns;
    }

    /**
     * @brief pattern the actual pattern image
     * @return a valid QImage. There are no guarantees to the image format.
     */
    QImage pattern() const;

private:

    bool init(QByteArray& data);
    void setPatternImage(const QImage& image);

private:
    QImage m_pattern;
    mutable QByteArray m_md5;
};

Q_DECLARE_METATYPE(KoPattern*)

Q_DECLARE_METATYPE(QSharedPointer<KoPattern>)

#endif // KOPATTERN_H

