/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KO_COLOR_TRANSFORMATION_FACTORY_H_
#define _KO_COLOR_TRANSFORMATION_FACTORY_H_

#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QString>

class KoColorTransformation;
class KoColorSpace;
class KoID;

#include "pigment_export.h"

/**
 * Allow to extend the number of color transformation of a
 * colorspace.
 */
class PIGMENTCMS_EXPORT KoColorTransformationFactory {
  public:
    KoColorTransformationFactory(QString id, QString name);
    virtual ~KoColorTransformationFactory();
  public:
    QString id() const;
    QString name() const;
  public:
    /**
     * @return an empty list if the factory support all type of colorspaces models.
     */
    virtual QList< QPair< KoID, KoID > > supportedModels() const = 0;
    virtual KoColorTransformation* createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const = 0;
  private:
    struct Private;
    Private* const d;
};

#endif
