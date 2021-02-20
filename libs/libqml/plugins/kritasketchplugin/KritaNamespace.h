/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
