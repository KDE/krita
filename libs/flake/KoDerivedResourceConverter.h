/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KO_DERIVED_RESOURCE_CONVERTER_H
#define __KO_DERIVED_RESOURCE_CONVERTER_H

#include <QScopedPointer>
#include <QSharedPointer>
#include "kritaflake_export.h"

class QVariant;

/**
 * \class KoDerivedResourceConverter
 *
 * Defines the abstraction of a derived resource. It should be
 * uploaded to the KoResourceManager during the loading phase.
 * The manager will use it to convert values to/from the source
 * resource.
 *
 *  "Derived" resources are the resources that do not exist temselves.
 *  Instead they are contained in some other resources and are updated
 *  synchronously with the parent resources as well.
 *
 *  E.g. we store opacity and composite op and opacity in the current
 *  paintop preset, which is also a resource. So composite op and opacity
 *  are "derived" resources.
 *
 *  The main goal of this class is to make our resources comply with
 *  a general Model-View-Controller architecture:
 *
 *  Model: KisPaintOpPreset. It stores opacity, composite op, eraser mode
 *         and other "global" properties.
 *
 *  Controller: KoCanvasResourceManager. It controls access to the resources
 *              and emits notification signals when they are changed.
 *
 *  View: KisPaintOpBox and other classes that show the resources on screen
 *
 *  Please take into account that according to the MVC design all the access
 *  to the model resources should be performed through the controller.
 *
 */
 class KRITAFLAKE_EXPORT KoDerivedResourceConverter
{
public:
    /**
     * \param key the unique id of the resource defined by this
     *            converter
     *
     * \param sourceKey the id of the parent resource, i.e. where the
     *                  values are really loaded/saved.
     */
    KoDerivedResourceConverter(int key, int sourceKey);
    virtual ~KoDerivedResourceConverter();

    int key() const;
    int sourceKey() const;

    QVariant readFromSource(const QVariant &value);
    QVariant writeToSource(const QVariant &value,
                           const QVariant &sourceValue,
                           bool *changed);

    virtual bool notifySourceChanged(const QVariant &sourceValue);

protected:
    /**
     * Converts the \p value of the source resource into the space of
     * the "derived" resource. E.g. preset -> opacity.
     */
    virtual QVariant fromSource(const QVariant &value) = 0;

    /**
     * Converts the value of the "derived" resource into the space of the
     * original ("source") resource. E.g. opacity -> preset.
     */
    virtual QVariant toSource(const QVariant &value, const QVariant &sourceValue) = 0;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

typedef QSharedPointer<KoDerivedResourceConverter> KoDerivedResourceConverterSP;

#endif /* __KO_DERIVED_RESOURCE_CONVERTER_H */
