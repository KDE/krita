/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KO_COLOR_SPACE_ENGINE_H_
#define _KO_COLOR_SPACE_ENGINE_H_

#include <KoColorConversionTransformationAbstractFactory.h>
#include <KoGenericRegistry.h>

/**
 * A KoColorSpaceEngine is a class use to create color conversion
 * transformation between color spaces, for which all profiles can
 * output to all profiles.
 * 
 * Typically, when you have an ICC color space and color profile, you
 * can convert to any other ICC color space and color profile. While
 * creating a KoColorTransformationFactory for each of this transformation
 * is possible, the number of links will make the Color Conversion explode
 * System. KoColorSpaceEngine provides a virtual node in the Color
 * Conversion System that can convert to any other node supported by the
 * engine.
 */
class PIGMENTCMS_EXPORT KoColorSpaceEngine : public KoColorConversionTransformationAbstractFactory {
    public:
        KoColorSpaceEngine(const QString& id, const QString& name);
        virtual ~KoColorSpaceEngine();
        const QString& id() const;
        const QString& name() const;
    private:
        struct Private;
        Private* const d;
};

class PIGMENTCMS_EXPORT KoColorSpaceEngineRegistry : public KoGenericRegistry< KoColorSpaceEngine* > {
    public:
        KoColorSpaceEngineRegistry();
        ~KoColorSpaceEngineRegistry();
        static KoColorSpaceEngineRegistry* instance();
};

#endif
