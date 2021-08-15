/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSCREENTONEGENERATORTEMPLATESAMPLER_H
#define KISSCREENTONEGENERATORTEMPLATESAMPLER_H

#include <QtGlobal>
#include <QVector>
#include <QTransform>

#include <cmath>

template <typename Template>
class KisScreentoneGeneratorAlignedTemplateSampler
{
public:
    KisScreentoneGeneratorAlignedTemplateSampler(const Template &the_template)
        : m_template(the_template)
    {}

    qreal operator()(int x, int y) const
    {
        // Get the coordinates in template space
        QPointF p(
            static_cast<qreal>(x) + static_cast<int>(std::round(m_template.screenPosition().x())) + 0.5,
            static_cast<qreal>(y) + static_cast<int>(std::round(m_template.screenPosition().y())) + 0.5
        );
        // Get the coordinates in screen space
        const QPointF screenPos = m_template.templateToScreenTransform().map(p);
        // Get x/y indices in macrocell units or the current macrocell tile
        // position
        const qreal a = -std::floor(screenPos.x() / static_cast<qreal>(m_template.macrocellSize().width()));
        const qreal b = -std::floor(screenPos.y() / static_cast<qreal>(m_template.macrocellSize().height()));
        // Get the correspondent point in the (0, 0) macrocell tile
        p += QPointF(a * m_template.v1().x() + b * m_template.v2().x(), a * m_template.v1().y() + b * m_template.v2().y());

        x = static_cast<int>(std::floor(p.x())) + m_template.originOffset().x();
        y = static_cast<int>(std::floor(p.y())) + m_template.originOffset().y();
        const int macrocellPointIndex = y * m_template.templateSize().width() + x;
        return m_template.templateData()[macrocellPointIndex];
    }

private:
    const Template& m_template;
};

template <typename Template>
class KisScreentoneGeneratorUnAlignedTemplateSampler
{
public:
    KisScreentoneGeneratorUnAlignedTemplateSampler(const Template &the_template)
        : m_template(the_template)
    {}

    qreal operator()(int x, int y) const
    {
        // Get the coordinates in screen space
        qreal xx, yy;
        m_template.imageToScreenTransform().map(static_cast<qreal>(x) + 0.5, static_cast<qreal>(y) + 0.5, &xx, &yy);
        // Convert to coordinate inside the macrocell
        xx -= std::floor(xx / m_template.macrocellSize().width()) * m_template.macrocellSize().width();
        yy -= std::floor(yy / m_template.macrocellSize().height()) * m_template.macrocellSize().height();
        // Get template coordinates
        QPointF templatePoint = m_template.screenToTemplateTransform().map(QPointF(xx, yy)) +
                                m_template.originOffset() - QPointF(0.5, 0.5);
        
        // Bilinear interpolation
        // Get integer coordinates of the template points to use in the interpolation
        const int ix0 =
            templatePoint.x() < 0.0 ? m_template.templateSize().width() - 1 :
                (templatePoint.x() >= m_template.templateSize().width() ? 0.0 :
                    static_cast<int>(std::floor(templatePoint.x())));
        const int iy0 =
            templatePoint.y() < 0.0 ? m_template.templateSize().height() - 1 :
                (templatePoint.y() >= m_template.templateSize().height() ? 0.0 :
                    static_cast<int>(std::floor(templatePoint.y())));
        const int ix1 = ix0 == m_template.templateSize().width() - 1 ? 0 : ix0 + 1;
        const int iy1 = iy0 == m_template.templateSize().height() - 1 ? 0 : iy0 + 1;
        // Get the template values for the points
        const qreal topLeftValue = m_template.templateData()[iy0 * m_template.templateSize().width() + ix0];
        const qreal topRightValue = m_template.templateData()[iy0 * m_template.templateSize().width() + ix1];
        const qreal bottomLeftValue = m_template.templateData()[iy1 * m_template.templateSize().width() + ix0];
        const qreal bottomRightValue = m_template.templateData()[iy1 * m_template.templateSize().width() + ix1];
        // Get the fractional part of the point to use in the interpolation
        const qreal fractionalX = templatePoint.x() - std::floor(templatePoint.x());
        const qreal fractionalY = templatePoint.y() - std::floor(templatePoint.y());
        // Perform bilinear interpolation
        const qreal a = topLeftValue * (1.0 - fractionalX) + topRightValue * fractionalX;
        const qreal b = bottomLeftValue * (1.0 - fractionalX) + bottomRightValue * fractionalX;
        const qreal c = a * (1.0 - fractionalY) + b * fractionalY;
        // ----
        return c;
    }

private:
    const Template& m_template;
};

#endif
