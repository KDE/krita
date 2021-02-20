/*
    SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KOABSTRACTGRADIENT_H
#define KOABSTRACTGRADIENT_H

#include <QGradient>
#include <QMetaType>

#include "KoColorSpace.h"
#include <KoResource.h>
#include <kritapigment_export.h>

class KoAbstractGradient;
typedef QSharedPointer<KoAbstractGradient> KoAbstractGradientSP;

class KoCanvasResourcesInterface;
using KoCanvasResourcesInterfaceSP = QSharedPointer<KoCanvasResourcesInterface>;

class KoColor;

/**
 * KoAbstractGradient is the base class of all gradient resources
 */
class KRITAPIGMENT_EXPORT KoAbstractGradient : public KoResource
{
public:
    explicit KoAbstractGradient(const QString &filename);
    ~KoAbstractGradient() override;

    /**
    * Creates a QGradient from the gradient.
    * The resulting QGradient might differ from original gradient
    */
    virtual QGradient* toQGradient() const {
        return new QGradient();
    }

    /// gets the color at position 0 <= t <= 1
    virtual void colorAt(KoColor&, qreal t) const;

    void setColorSpace(KoColorSpace* colorSpace);
    const KoColorSpace * colorSpace() const;

    void setSpread(QGradient::Spread spreadMethod);
    QGradient::Spread spread() const;

    void setType(QGradient::Type repeatType);
    QGradient::Type type() const;

    void updatePreview();

    QImage generatePreview(int width, int height) const;
    QImage generatePreview(int width, int height, KoCanvasResourcesInterfaceSP canvasResourcesInterface) const;

    KoAbstractGradient(const KoAbstractGradient &rhs);

    KoAbstractGradientSP cloneAndBakeVariableColors(KoCanvasResourcesInterfaceSP canvasResourcesInterface) const;
    virtual void bakeVariableColors(KoCanvasResourcesInterfaceSP canvasResourcesInterface);

    KoAbstractGradientSP cloneAndUpdateVariableColors(KoCanvasResourcesInterfaceSP canvasResourcesInterface) const;
    virtual void updateVariableColors(KoCanvasResourcesInterfaceSP canvasResourcesInterface);

private:
    struct Private;
    Private* const d;
};

Q_DECLARE_METATYPE(KoAbstractGradient*)
Q_DECLARE_METATYPE(QSharedPointer<KoAbstractGradient>)


#endif // KOABSTRACTGRADIENT_H
