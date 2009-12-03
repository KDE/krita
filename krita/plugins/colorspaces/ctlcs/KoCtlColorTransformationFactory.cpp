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

#include "KoCtlColorTransformationFactory.h"

#include "KoColorTransformation.h"
#include "KoColorSpace.h"

#include <QHash>

#include <KoID.h>

#include <KoCtlBuffer.h>
#include <KoCtlUtils.h>

#include <OpenCTL/Module.h>
#include <OpenCTL/Program.h>
#include <OpenCTL/Template.h>
#include <GTLCore/Value.h>

class KoCtlColorTransformation : public KoColorTransformation
{
public:
    KoCtlColorTransformation(OpenCTL::Program* program, const KoColorSpace* colorSpace) : m_program(program), m_colorSpace(colorSpace) {
    }
    ~KoCtlColorTransformation()
    {
      delete m_program;
    }

public:
    void transform(const quint8 *srcU8, quint8 *dstU8, qint32 numColumns) const {
        KoCtlBuffer src(reinterpret_cast<char*>(const_cast<quint8*>(srcU8)), numColumns * m_colorSpace->pixelSize());
        KoCtlBuffer dst(reinterpret_cast<char*>(dstU8), numColumns * m_colorSpace->pixelSize());
        std::list< GTLCore::Buffer* > ops;
        ops.push_back(&src);
        m_program->apply(ops, dst);
    }

    virtual void setParameter(const QString& name, const QVariant& variant) {
        const char* ascii = name.toAscii().data();
        switch ( m_program->varying( ascii ).type()->dataType()) {
        case GTLCore::Type::BOOLEAN:
            m_program->setVarying( ascii, GTLCore::Value(variant.toBool()));
            break;
        case GTLCore::Type::FLOAT16:
        case GTLCore::Type::FLOAT32:
        case GTLCore::Type::FLOAT64:
            m_program->setVarying( ascii, GTLCore::Value((float)variant.toDouble()));
            break;
        case GTLCore::Type::INTEGER8:
        case GTLCore::Type::INTEGER16:
        case GTLCore::Type::INTEGER32:
            m_program->setVarying( ascii, GTLCore::Value(variant.toInt()));
            break;
        case GTLCore::Type::UNSIGNED_INTEGER8:
        case GTLCore::Type::UNSIGNED_INTEGER16:
        case GTLCore::Type::UNSIGNED_INTEGER32:
            m_program->setVarying( ascii, GTLCore::Value(variant.toUInt()));
            break;
        case GTLCore::Type::ARRAY:
        case GTLCore::Type::VECTOR:
        default:
        case GTLCore::Type::UNDEFINED: {
            qFatal("Unsupported type: %i", variant.type());
        }
        }
    }

private:
    OpenCTL::Program* m_program;
    const KoColorSpace* m_colorSpace;
};


KoCtlColorTransformationFactory::KoCtlColorTransformationFactory(OpenCTL::Template* _template) : KoColorTransformationFactory(_template->name().c_str(), _template->name().c_str()), m_template(_template)
{
}

KoCtlColorTransformationFactory::~KoCtlColorTransformationFactory()
{
}

QList< QPair< KoID, KoID > > KoCtlColorTransformationFactory::supportedModels() const
{
  return QList< QPair< KoID, KoID > >();
}

KoColorTransformation* KoCtlColorTransformationFactory::createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const
{
  GTLCore::PixelDescription pixelDescription = createPixelDescription(colorSpace);
  
  OpenCTL::Module* module = m_template->generateModule(pixelDescription);
  
  OpenCTL::Program* program = new OpenCTL::Program("process", module, pixelDescription);
  
  return new KoCtlColorTransformation(program, colorSpace);
}
