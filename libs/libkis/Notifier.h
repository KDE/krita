/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef LIBKIS_NOTIFIER_H
#define LIBKIS_NOTIFIER_H

#include <QObject>

#include "kritalibkis_export.h"
#include <kis_types.h>
#include "libkis.h"
#include <KisDocument.h>
#include <KisView.h>
#include <KisMainWindow.h>

/**
 * Notifier
 */
class KRITALIBKIS_EXPORT Notifier : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Notifier)

    Q_PROPERTY(bool Active READ active WRITE setActive)

public:
    explicit Notifier(QObject *parent = 0);
    virtual ~Notifier();

    bool active() const;
    void setActive(bool value);

Q_SIGNALS:

    /**
     * @brief applicationClosing is emitted when the application is about to close. This
     * happens after any documents and windows are closed.
     */
    void applicationClosing();

    /**
     * @brief imageCreated is emitted whenever a new image is created and registered with
     * the application.
     */
    void imageCreated(Document *image);

    /**
     * @brief imageSaved is emitted whenever a document is saved.
     * @param filename the filename of the document that has been saved.
     */
    void imageSaved(const QString &filename);

    /**
     * @brief imageClosed is emitted whenever the last view on an image is closed. The image
     * does not exist anymore in Krita
     * @param filename the filename of the image.
     */
    void imageClosed(const QString &filename);

    /**
     * @brief viewCreated is emitted whenever a new view is created.
     * @param view the view
     */
    void viewCreated(View *view);

    /**
     * @brief viewClosed is emitted whenever a view is closed
     * @param view the view
     */
    void viewClosed(View *view);

    /**
     * @brief windowCreated is emitted whenever a window is created
     * @param window the window
     */
    void windowCreated(Window *window);


private Q_SLOTS:

    void imageCreated(KisDocument *document);

    void viewCreated(KisView *view);
    void viewClosed(KisView *view);

    void windowCreated(KisMainWindow *window);


private:
    struct Private;
    Private *const d;

};

#endif // LIBKIS_NOTIFIER_H
