/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KO_CTL_COLOR_SPACE_INFO_H_
#define _KO_CTL_COLOR_SPACE_INFO_H_

#include <KoChannelInfo.h>

class KoCtlAccumulator;
class KoID;

namespace GTLCore
{
class PixelDescription;
}

class KoCtlColorSpaceInfo
{
public:
    class ChannelInfo
    {
        friend class KoCtlColorSpaceInfo;
        friend class QList<ChannelInfo>;
        ChannelInfo();
        ~ChannelInfo();
    public:
        const QString& name() const;
        const QString& shortName() const;
        qint32 position() const;
        qint32 index() const;
        enum KoChannelInfo::enumChannelType channelType() const;
        enum KoChannelInfo::enumChannelValueType valueType() const;
        qint32 size() const;
        const QColor& color() const;
    private:
        struct Private;
        Private* const d;
    };
public:
    KoCtlColorSpaceInfo(const QString& _xmlfile);
    const QString& fileName() const;
    bool load();
    ~KoCtlColorSpaceInfo();
    qint32 referenceDepth() const;
    const KoID& colorDepthId() const;
    const KoID& colorModelId() const;
    const QString& colorSpaceId() const;
    const QString& name() const;
    const QString& defaultProfile() const;
    const QList<const ChannelInfo*>& channels() const;
    bool isHdr() const;
    quint32 colorChannelCount() const;
    quint32 pixelSize() const;
    const GTLCore::PixelDescription& pixelDescription() const;
    int alphaPos() const;
    QList<KoCtlAccumulator*> accumulators() const;
private:
    struct Private;
    Private* const d;
};

#endif
