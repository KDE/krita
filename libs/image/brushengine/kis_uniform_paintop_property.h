/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_UNIFORM_PAINT_OP_PROPERTY_H
#define __KIS_UNIFORM_PAINT_OP_PROPERTY_H

#include <QScopedPointer>
#include <QObject>

#include "kritaimage_export.h"


class KRITAIMAGE_EXPORT KisUniformPaintOpProperty : public QObject
{
    Q_OBJECT
public:
    enum Type {
        Int = 0,
        Double,
        Bool,
        Combo
    };

public:
    KisUniformPaintOpProperty(Type type,
                              const QString &id,
                              const QString &name,
                              QObject *parent);
    ~KisUniformPaintOpProperty();

    QString id() const;
    QString name() const;
    Type type() const;

    int valueInt() const;
    qreal valueDouble() const;
    bool valueBool() const;
    int valueCombo() const;

public Q_SLOTS:
    void setValueInt(int value);
    void setValueDouble(qreal value);
    void setValueBool(bool value);
    void setValueCombo(int value);

Q_SIGNALS:
    void sigValueIntChanged(int value);
    void sigValueDoubleChanged(qreal value);
    void sigValueBoolChanged(bool value);
    void sigValueComboChanged(int value);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_UNIFORM_PAINT_OP_PROPERTY_H */
