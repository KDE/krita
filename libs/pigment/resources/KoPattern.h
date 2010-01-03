/*
    Copyright (c) 2000 Matthias Elter  <elter@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

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

#include "KoResource.h"
#include <pigment_export.h>

/// Write API docs here
class PIGMENTCMS_EXPORT KoPattern : public KoResource
{

public:

    /**
     * Creates a new KoPattern object using @p filename.  No file is opened
     * in the constructor, you have to call load.
     *
     * @param filename the file name to save and load from.
     */
    KoPattern(const QString& filename);
    virtual ~KoPattern();

public:

    virtual bool load();
    virtual bool save();

    virtual QImage image() const;

    qint32 width() const;
    qint32 height() const;

    void setImage(const QImage& image);

    QString defaultFileExtension() const;

    KoPattern& operator=(const KoPattern& pattern);
private:
    bool init(QByteArray& data);

private:
    QImage m_image;
};

#endif // KOPATTERN_H

