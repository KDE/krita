/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QFlags>
#include <QMetaType>

#include <kritaglobal_export.h>

namespace KisAlgebra2D
{

enum KisTransformComponent
{
    Translate = 0x1,
    Scale = 0x2,
    Rotate = 0x4,
    Shear = 0x8,
    Project = 0x10
};

Q_DECLARE_FLAGS(KisTransformComponents, KisTransformComponent);


KisTransformComponents KRITAGLOBAL_EXPORT makeFullTransformComponents();
KisTransformComponents KRITAGLOBAL_EXPORT componentsForTransform(const QTransform &t);
KisTransformComponents KRITAGLOBAL_EXPORT compareTransformComponents(const QTransform &lhs, const QTransform &rhs);
}

Q_DECLARE_METATYPE(KisAlgebra2D::KisTransformComponents)
Q_DECLARE_OPERATORS_FOR_FLAGS(KisAlgebra2D::KisTransformComponents)


// we don't use Q_FLAGS's autogeneration of QDebug here because we
// want to avoid adding Q_NAMESPACE to KisAlgebra2D
QDebug KRITAGLOBAL_EXPORT operator<<(QDebug dbg, KisAlgebra2D::KisTransformComponent component);
QDebug KRITAGLOBAL_EXPORT operator<<(QDebug dbg, KisAlgebra2D::KisTransformComponents components);
