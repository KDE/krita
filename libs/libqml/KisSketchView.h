/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@kogmbh.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KRITA_SKETCH_VIEW_H
#define KRITA_SKETCH_VIEW_H

#include <QQuickItem>

#include "krita_sketch_export.h"

class KRITA_SKETCH_EXPORT KisSketchView : public QQuickItem
{
    Q_OBJECT
//    Q_PROPERTY(QObject *selectionManager READ selectionManager NOTIFY viewChanged)
//    Q_PROPERTY(QObject *selectionExtras READ selectionExtras NOTIFY viewChanged)
    Q_PROPERTY(QObject *view READ view NOTIFY viewChanged)
//    Q_PROPERTY(QString file READ file WRITE setFile NOTIFY fileChanged)
//    Q_PROPERTY(QString fileTitle READ fileTitle NOTIFY fileChanged);
//    Q_PROPERTY(bool modified READ isModified NOTIFY modifiedChanged)

//    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged);
//    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged);

//    Q_PROPERTY(int imageHeight READ imageHeight NOTIFY imageSizeChanged)
//    Q_PROPERTY(int imageWidth READ imageWidth NOTIFY imageSizeChanged)

public:
    explicit KisSketchView(QQuickItem* parent = 0);
    virtual ~KisSketchView();

    QObject* selectionManager() const;
    QObject* selectionExtras() const;
    QObject* doc() const;
    QObject* view() const;
    QString file() const;
    QString fileTitle() const;
    bool isModified() const;

    virtual void componentComplete();
    virtual void geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry);

    void setFile(const QString &file);

    void showFloatingMessage(const QString message, const QIcon& icon);

    bool canUndo() const;
    bool canRedo() const;

    int imageHeight() const;
    int imageWidth() const;

public Q_SLOTS:
    void undo();
    void redo();

    void zoomIn();
    void zoomOut();

    void save();
    void saveAs(const QString& fileName, const QString& mimeType);

    void documentAboutToBeDeleted();
    void documentChanged();
    void centerDoc();

    void activate();

Q_SIGNALS:
    // This is directly forwarded from the document, which means that
    // value 0-100 means in progress
    // value -1 means completed
    void progress(int value);
    void viewChanged();
    void fileChanged();
    void modifiedChanged();
    void floatingMessageRequested(const QString &message, const QString &iconName);
    void interactionStarted();
    void loadingFinished();
    void savingFinished();
    void canUndoChanged();
    void canRedoChanged();
    void imageSizeChanged();

protected:
    virtual bool event(QEvent* event);
    // QT5TODO
//     virtual bool sceneEvent(QEvent* event);

private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void imageUpdated(const QRect &updated))
    Q_PRIVATE_SLOT(d, void documentOffsetMoved())
    Q_PRIVATE_SLOT(d, void zoomChanged())
    Q_PRIVATE_SLOT(d, void resetDocumentPosition())
    Q_PRIVATE_SLOT(d, void removeNodeAsync(KisNodeSP removedNode))
};

#endif // KRITA_SKETCH_CANVAS_H
