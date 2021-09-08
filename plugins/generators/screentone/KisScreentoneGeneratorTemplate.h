/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSCREENTONEGENERATORTEMPLATE_H
#define KISSCREENTONEGENERATORTEMPLATE_H

#include <QtGlobal>
#include <QVector>
#include <QTransform>

#include "KisScreentoneGeneratorConfiguration.h"

class KisScreentoneGeneratorTemplate
{
public:
    KisScreentoneGeneratorTemplate(const KisScreentoneGeneratorConfigurationSP config);

    inline const QVector<qreal>& templateData() const { return m_templateData; }
    inline const QTransform& imageToScreenTransform() const { return m_imageToScreenTransform; }
    inline const QTransform& screenToTemplateTransform() const { return m_screenToTemplateTransform; }
    inline const QTransform& templateToScreenTransform() const { return m_templateToScreenTransform; }
    inline const QPointF& screenPosition() const { return m_screenPosition; }
    inline const QSize& macrocellSize() const { return m_macrocellSize; }
    inline const QSize& templateSize() const { return m_templateSize; }
    inline const QPoint& originOffset() const{ return m_originOffset; }
    inline const QPointF& v1() const { return m_v1; }
    inline const QPointF& v2() const { return m_v2; }

private:
    QVector<qreal> m_templateData;
    QTransform m_imageToScreenTransform, m_screenToTemplateTransform, m_templateToScreenTransform;
    QPointF m_screenPosition;
    QSize m_macrocellSize;
    QSize m_templateSize;
    QPoint m_originOffset;
    QPointF m_v1, m_v2;

    template <typename ScreentoneFunction>
    void makeTemplate(const KisScreentoneGeneratorConfigurationSP config, ScreentoneFunction screentoneFunction);
    QVector<int> makeCellOrderList(int macrocellColums, int macrocellRows) const;
};

#endif
