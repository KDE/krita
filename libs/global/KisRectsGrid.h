/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISRECTSGRID_H
#define KISRECTSGRID_H

#include <QRect>
#include <QVector>
#include "kritaglobal_export.h"

class KRITAGLOBAL_EXPORT KisRectsGrid
{
public:
    KisRectsGrid(int gridSize = 64);

    QRect alignRect(const QRect &rc) const;

    QVector<QRect> addRect(const QRect &rc);
    QVector<QRect> removeRect(const QRect &rc);

    QVector<QRect> addAlignedRect(const QRect &rc);
    QVector<QRect> removeAlignedRect(const QRect &rc);

    bool contains(const QRect &rc) const;

    QRect boundingRect() const;

private:
    void resize(const QRect &newMappedAreaSize);
    static QRect shrinkRectToAlignedGrid(const QRect &srcRect, int lod);

private:
    int m_gridSize;
    int m_logGridSize;
    QVector<quint8> m_mapping;
    QRect m_mappedAreaSize; // measured in col/row

};

#endif // KISRECTSGRID_H
