/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QDebug>
#include <kis_algebra_2d.h>
#include <KisTransformComponents.h>

namespace KisAlgebra2D {

KisTransformComponents makeFullTransformComponents()
{
    return KisTransformComponent::Translate | KisTransformComponent::Scale | KisTransformComponent::Rotate
        | KisTransformComponent::Shear | KisTransformComponent::Project;
}

KisTransformComponents componentsForTransform(const QTransform &t)
{
    KisTransformComponents result;

    KisAlgebra2D::DecomposedMatrix m(t);

    result.setFlag(KisTransformComponent::Translate,
        !qFuzzyIsNull(m.dx) || !qFuzzyIsNull(m.dy));

    result.setFlag(KisTransformComponent::Scale,
        !qFuzzyCompare(m.scaleX, 1.0) || !qFuzzyCompare(m.scaleY, 1.0));

    result.setFlag(KisTransformComponent::Shear,
        !qFuzzyIsNull(m.shearXY));

    result.setFlag(KisTransformComponent::Rotate,
        !qFuzzyIsNull(m.angle));

    result.setFlag(KisTransformComponent::Project,
        !qFuzzyIsNull(m.proj[0]) || !qFuzzyIsNull(m.proj[1]) || !qFuzzyCompare(m.proj[2], 1.0));

    return result;
}

KisTransformComponents compareTransformComponents(const QTransform &lhs, const QTransform &rhs)
{
    KisAlgebra2D::DecomposedMatrix m1(lhs);
    KisAlgebra2D::DecomposedMatrix m2(rhs);

    KisTransformComponents result;

    if (qFuzzyCompare(m1.dx, m2.dx) && qFuzzyCompare(m1.dy, m2.dy)) {
        result.setFlag(KisTransformComponent::Translate);
    }

    if (qFuzzyCompare(m1.scaleX, m2.scaleX) && qFuzzyCompare(m1.scaleX, m2.scaleY)) {
        result.setFlag(KisTransformComponent::Scale);
    }

    if (qFuzzyCompare(m1.shearXY, m2.shearXY)) {
        result.setFlag(KisTransformComponent::Shear);
    }

    if (qFuzzyCompare(m1.angle, m2.angle)) {
        result.setFlag(KisTransformComponent::Rotate);
    }

    if (qFuzzyCompare(m1.proj[0], m2.proj[0]) &&
        qFuzzyCompare(m1.proj[1], m2.proj[1]) &&
        qFuzzyCompare(m1.proj[2], m2.proj[2])) {

        result.setFlag(KisTransformComponent::Project);
    }

    return result;
}

}

QDebug operator<<(QDebug dbg, KisAlgebra2D::KisTransformComponent component) {
    switch (component) {
        case KisAlgebra2D::KisTransformComponent::Translate:
            dbg << "KisTransformComponent::Translate";
            break;
        case KisAlgebra2D::KisTransformComponent::Scale:
            dbg << "KisTransformComponent::Scale";
            break;
        case KisAlgebra2D::KisTransformComponent::Rotate:
            dbg << "KisTransformComponent::Rotate";
            break;
        case KisAlgebra2D::KisTransformComponent::Shear:
            dbg << "KisTransformComponent::Shear";
            break;
        case KisAlgebra2D::KisTransformComponent::Project:
            dbg << "KisTransformComponent::Project";
            break;
        default:
            dbg << "<unknown>";
            break;
    }
    return dbg;
}

QDebug operator<<(QDebug dbg, KisAlgebra2D::KisTransformComponents components) {
    dbg.nospace() << "KisTransformComponents(";

    bool first = true;
    if (components.testFlag(KisAlgebra2D::KisTransformComponent::Translate)) {
        dbg.nospace() << (first ? "" : " | ") << "Translate";
        first = false;
    }
    if (components.testFlag(KisAlgebra2D::KisTransformComponent::Scale)) {
        dbg.nospace() << (first ? "" : " | ") << "Scale";
        first = false;
    }
    if (components.testFlag(KisAlgebra2D::KisTransformComponent::Rotate)) {
        dbg.nospace() << (first ? "" : " | ") << "Rotate";
        first = false;
    }
    if (components.testFlag(KisAlgebra2D::KisTransformComponent::Shear)) {
        dbg.nospace() << (first ? "" : " | ") << "Shear";
        first = false;
    }
    if (components.testFlag(KisAlgebra2D::KisTransformComponent::Project)) {
        dbg.nospace() << (first ? "" : " | ") << "Project";
        first = false;
    }

    dbg.nospace() << ")";
    return dbg.space();
}