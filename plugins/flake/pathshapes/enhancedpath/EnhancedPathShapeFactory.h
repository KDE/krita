/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOENHANCEDPATHSHAPEFACTORY_H
#define KOENHANCEDPATHSHAPEFACTORY_H

#include <KoShapeFactoryBase.h>

class KoShape;

/// Factory for path shapes
class EnhancedPathShapeFactory : public KoShapeFactoryBase
{
public:
    /// constructor
    EnhancedPathShapeFactory();
    ~EnhancedPathShapeFactory() override {}
    KoShape *createShape(const KoProperties *params, KoDocumentResourceManager *documentResources = 0) const override;
    KoShape *createDefaultShape(KoDocumentResourceManager *documentResources = 0) const override;
    bool supports(const KoXmlElement &e, KoShapeLoadingContext &context) const override;
private:
    void addCross();
    void addArrow();
    void addCallout();
    void addSmiley();
    void addCircularArrow();
    void addGearhead();

    typedef QMap<QString, QVariant > ComplexType;
    typedef QList<QVariant> ListType;
    KoProperties *dataToProperties(const QString &modifiers, const QStringList &commands,
                                   const ListType &handles, const ComplexType &formulae) const;

};

#endif // KOENHANCEDPATHSHAPEFACTORY_H
