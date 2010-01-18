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

#ifndef _KO_CTL_COLOR_TRANSFORMATION_FACTORY_H_
#define _KO_CTL_COLOR_TRANSFORMATION_FACTORY_H_

#include <QMap>
#include <QMutex>

#include "KoColorTransformationFactory.h"
#include <GTLCore/PixelDescription.h>

namespace OpenCTL
{
class Program;
class Template;
}

class KoCtlColorTransformationFactory : public KoColorTransformationFactory
{
public:
    KoCtlColorTransformationFactory(OpenCTL::Template* _template);
    ~KoCtlColorTransformationFactory();

    virtual QList< QPair< KoID, KoID > > supportedModels() const;

    virtual KoColorTransformation* createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const;

    void putBackProgram(const GTLCore::PixelDescription& pixelDescription, OpenCTL::Program* program) const;
private:
    OpenCTL::Template* m_template;
    mutable QMap<GTLCore::PixelDescription, QList<OpenCTL::Program*> > m_programs;
    mutable QMutex m_mutex;
};

#endif
