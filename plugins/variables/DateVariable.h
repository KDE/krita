/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#include <KoVariable.h>

#include <QString>
#include <QVariant>
#include <QDateTime>

/**
 * Base class for in-text variables.
 * A variable is a field inserted into the text and the content is set to a specific value that
 * is used as text.  This class is pretty boring in that it has just a setValue() to alter the
 * text shown; we depend on plugin writers to create more exciting ways to update variables.
 */
class DateVariable : public KoVariable {
public:
    enum DateType {
        Fixed
    };

    /**
     * Constructor.
     */
    DateVariable(DateType type);

    void setProperties(const KoProperties *props);

    QWidget *createOptionsWidget();

    QString definition() const { return m_definition; }
    void setDefinition(const QString &definition);

    int offset() const { return m_offset; }
    void setOffset(int offset);

private:
    void update();

    DateType m_type;
    QString m_definition;
    QDateTime m_time;
    int m_offset;
};
