/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#ifndef KRITANAMESPACE_H
#define KRITANAMESPACE_H

#include <QObject>

class ImageBuilder;
class KritaNamespace : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* ImageBuilder READ imageBuilder CONSTANT)
    Q_PROPERTY(QObject* Window READ window WRITE setWindow NOTIFY windowChanged)
    Q_PROPERTY(QObject* MouseTracker READ mouseTracker CONSTANT)
    Q_PROPERTY(QObject* VirtualKeyboardController READ virtualKeyboardController CONSTANT)
    Q_PROPERTY(QObject* ProgressProxy READ progressProxy CONSTANT)

public:
    explicit KritaNamespace(QObject* parent = 0);
    virtual ~KritaNamespace();

    QObject *imageBuilder() const;
    QObject *window() const;
    void setWindow(QObject* window);
    Q_SIGNAL void windowChanged();
    QObject *mouseTracker() const;
    QObject *virtualKeyboardController() const;
    QObject *progressProxy() const;

    Q_INVOKABLE bool fileExists(const QString& filename) const;

private:
    class Private;
    Private * const d;
};

#endif // KRITANAMESPACE_H
