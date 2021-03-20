/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef IMAGEBUILDER_H
#define IMAGEBUILDER_H

#include <QObject>
#include <QVariantMap>

class ImageBuilder : public QObject
{
    Q_OBJECT
public:
    explicit ImageBuilder(QObject* parent = 0);
    virtual ~ImageBuilder();

    Q_INVOKABLE QString createBlankImage(int width, int height, int resolution);
    Q_INVOKABLE QString createBlankImage(const QVariantMap& options);
    Q_INVOKABLE QString createImageFromClipboard();
    Q_INVOKABLE QString createImageFromWebcam(int width, int height, int resolution);
    Q_INVOKABLE QString createImageFromTemplate(const QVariantMap& options);

private Q_SLOTS:
    void createImageFromClipboardDelayed();
};

#endif // IMAGEBUILDER_H
