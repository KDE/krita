/* This file is part of the KDE project
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
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

#ifndef KOIMAGESELECTIONWIDGET_H
#define KOIMAGESELECTIONWIDGET_H

#include "komain_export.h"

#include <QtGui/QWidget>
#include <QtCore/QMap>

class KoImageCollection;
class KoImageData;
class KoShape;
class KoResourceManager;

class KOMAIN_EXPORT KoImageSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KoImageSelectionWidget(KoImageCollection *collection, QWidget *parent = 0);
    virtual ~KoImageSelectionWidget();

    /// return if the user selected a valid image and we successfully downloaded it.
    bool hasValidImage() const;
    /// return the image data resulting from the users choice
    KoImageData *imageData() const;

    static KoImageData *selectImage(KoImageCollection *collection, QWidget *parent);
    static KoShape *selectImageShape(KoResourceManager *documentResourceManager, QWidget *parent);

signals:
    /**
     * Emitted when the image object has successfully been created.
     * The user should not be allowed to press Ok the image became available.
     */
    void imageAvailable(bool);

private:
    class Private;
    Private *d;
    Q_PRIVATE_SLOT(d, void acceptFileSelection())
    Q_PRIVATE_SLOT(d, void setImageData(KJob*))
};

#endif
