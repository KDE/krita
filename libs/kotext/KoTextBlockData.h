/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOTEXTBLOCKDATA_H
#define KOTEXTBLOCKDATA_H

#include <QTextBlockUserData>

#include <koffice_export.h>

/**
 * This class is used to store properties for KoText layouting inside Qt QTextBlock
 * instances.
 */
class KOTEXT_EXPORT KoTextBlockData : public QTextBlockUserData {
public:
    KoTextBlockData();
    ~KoTextBlockData();

    /// return if this block has up-to-date counter data
    bool hasCounterData() const;
    /// return the width (in pt) of the counter.
    double counterWidth() const { return m_counterWidth; }
    /// set the width of the counter in pt.
    void setCounterWidth(double width) { m_counterWidth = width; }
    /// set the exact text that will be painted as the counter
    void setCounterText(const QString &text) { m_counterText = text; }
    /// return the exact text that will be painted as the counter
    const QString &counterText() const { return m_counterText; }

    void setCounterPosition(QPointF position) { m_counterPos = position; }
    QPointF counterPosition() const { return m_counterPos; }

private:
    double m_counterWidth;
    QString m_counterText;
    QPointF m_counterPos;
};

#endif
