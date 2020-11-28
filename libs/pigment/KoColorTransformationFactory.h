/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KO_COLOR_TRANSFORMATION_FACTORY_H_
#define _KO_COLOR_TRANSFORMATION_FACTORY_H_

#include <QHash>
#include <QVariant>
#include <QList>
#include <QPair>
#include <QString>

class KoColorTransformation;
class KoColorSpace;
class KoID;

#include "kritapigment_export.h"

/**
 * Allow to extend the number of color transformation of a
 * colorspace.
 */
class KRITAPIGMENT_EXPORT KoColorTransformationFactory
{
public:
    explicit KoColorTransformationFactory(const QString &id);
    virtual ~KoColorTransformationFactory();
public:
    QString id() const;
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
