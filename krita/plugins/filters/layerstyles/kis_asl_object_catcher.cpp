/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_asl_object_catcher.h"

#include <QString>
#include <QPointF>
#include <QColor>

#include "kis_debug.h"


KisAslObjectCatcher::KisAslObjectCatcher()
    : m_arrayMode(false)
{
}

KisAslObjectCatcher::~KisAslObjectCatcher()
{
}

void KisAslObjectCatcher::addDouble(const QString &path, double value) {
    qDebug() <<  (m_arrayMode ? "[A]" : "[ ]") << path << "double" << value;
}

void KisAslObjectCatcher::addInteger(const QString &path, int value) {
    qDebug() <<  (m_arrayMode ? "[A]" : "[ ]") << path << "int" << value;
}

void KisAslObjectCatcher::addEnum(const QString &path, const QString &typeId, const QString &value) {
    qDebug() <<  (m_arrayMode ? "[A]" : "[ ]") << path << "enum" << ppVar(typeId) << ppVar(value);
}

void KisAslObjectCatcher::addUnitFloat(const QString &path, const QString &unit, double value) {
    qDebug() <<  (m_arrayMode ? "[A]" : "[ ]") << path << "unitfloat" << ppVar(unit) << ppVar(value);
}

void KisAslObjectCatcher::addText(const QString &path, const QString &value) {
    qDebug() <<  (m_arrayMode ? "[A]" : "[ ]") << path << "text" << value;
}

void KisAslObjectCatcher::addBoolean(const QString &path, bool value) {
    qDebug() <<  (m_arrayMode ? "[A]" : "[ ]") << path << "bool" << value;
}

void KisAslObjectCatcher::addColor(const QString &path, const QColor &value) {
    qDebug() <<  (m_arrayMode ? "[A]" : "[ ]") << path << "color" << value;
}

void KisAslObjectCatcher::addPoint(const QString &path, const QPointF &value) {
    qDebug() <<  (m_arrayMode ? "[A]" : "[ ]") << path << "point" << value;
}

void KisAslObjectCatcher::addCurve(const QString &path, const QString &name, const QVector<QPointF> &points) {
    qDebug() <<  (m_arrayMode ? "[A]" : "[ ]") << path << "curve" << name << ppVar(points.size());
}

void KisAslObjectCatcher::addPattern(const QString &path, const KoPattern *value)
{
    qDebug() <<  (m_arrayMode ? "[A]" : "[ ]") << path << "pattern" << value;
}

void KisAslObjectCatcher::setArrayMode(bool value) {
    m_arrayMode = value;
}

