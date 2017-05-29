/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KOSVGTEXTPROPERTIES_H
#define KOSVGTEXTPROPERTIES_H

#include "kritaflake_export.h"

#include <QScopedPointer>
#include <QVariant>
#include <QList>

class SvgLoadingContext;


class KRITAFLAKE_EXPORT KoSvgTextProperties
{
public:
    enum PropertyId {
        WritingModeId,
        DirectionId,
        UnicodeBidiId,
        TextAnchorId,
        DominantBaselineId,
        AlignmentBaselineId,
        BaselineShiftModeId,
        BaselineShiftValueId,
        KerningId,
        GlyphOrientationVerticalId,
        GlyphOrientationHorizontalId,
        LetterSpacingId,
        WordSpacingId,

        FontFamiliesId,
        FontStyleId,
        FontIsSmallCapsId,
        FontStretchId,
        FontWeightId,
        FontSizeId,
        FontSizeAdjustId,
        TextDecorationId,
    };

public:

    KoSvgTextProperties();
    ~KoSvgTextProperties();

    KoSvgTextProperties(const KoSvgTextProperties &rhs);
    KoSvgTextProperties& operator=(const KoSvgTextProperties &rhs);

    void setProperty(PropertyId id, const QVariant &value);
    bool hasProperty(PropertyId id) const;
    QVariant property(PropertyId id, const QVariant &defaultValue = QVariant()) const;
    void removeProperty(PropertyId id);

    QVariant propertyOrDefault(PropertyId id) const;

    QList<PropertyId> properties() const;

    bool isEmpty() const;

    void resetNonInheritableToDefault();
    void inheritFrom(const KoSvgTextProperties &parentProperties);

    bool inheritsProperty(PropertyId id, const KoSvgTextProperties &parentProperties) const;
    KoSvgTextProperties ownProperties(const KoSvgTextProperties &parentProperties) const;

    void parseSvgTextAttribute(const SvgLoadingContext &context, const QString &command, const QString &value);
    QMap<QString,QString> convertToSvgTextAttributes() const;
    static QStringList supportedXmlAttributes();

    static const KoSvgTextProperties& defaultProperties();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KOSVGTEXTPROPERTIES_H
