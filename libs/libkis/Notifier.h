/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
 * The Notifier can be used to be informed of state changes in the Krita application.
 */
class KRITALIBKIS_EXPORT Notifier : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Notifier)

    Q_PROPERTY(bool Active READ active WRITE setActive)

public:
    explicit Notifier(QObject *parent = 0);
    ~Notifier() override;

    /**
     * @return true if the Notifier is active.
     */
    bool active() const;
    
    /**
     * Enable or disable the Notifier
     */
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
     * @brief windowCreated is emitted whenever a window is being created
     * @param window the window; this is called from the constructor of the window, before the xmlgui file is loaded
     */
    void windowIsBeingCreated(Window *window);

    /**
     * @brief windowIsCreated is emitted after main window is completely created
     */
    void windowCreated();

    /**
     * @brief configurationChanged is emitted every time Krita's configuration
     * has changed.
     */
    void configurationChanged();

private Q_SLOTS:

    void imageCreated(KisDocument *document);

    void viewCreated(KisView *view);
    void viewClosed(KisView *view);

    void windowIsBeingCreated(KisMainWindow *window);


private:
    struct Private;
    Private *const d;

};

#endif // LIBKIS_NOTIFIER_H
