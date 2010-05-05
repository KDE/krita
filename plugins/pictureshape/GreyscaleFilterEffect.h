/*
    This file is part of the KDE project
    Copyright (C) 2010 Carlos Licea carlos@kdab.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef GREYSCALEFILTEREFFECT_H
#define GREYSCALEFILTEREFFECT_H

#include <KoFilterEffect.h>

#define GreyscaleFilterEffectId "GreyscaleFilterEffectId"

class GreyscaleFilterEffect : public KoFilterEffect
{
public:
    GreyscaleFilterEffect();
    virtual ~GreyscaleFilterEffect();
    virtual void save(KoXmlWriter& writer);
    virtual bool load(const KoXmlElement& element, const KoFilterEffectLoadingContext& context);
    virtual QImage processImage(const QImage& image, const KoFilterEffectRenderContext& context) const;
};

#endif // GRAYSCALEFILTEREFFECT_H
