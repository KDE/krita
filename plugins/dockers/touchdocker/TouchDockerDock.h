/*
 *  SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TOUCHDOCKER_DOCK_H
#define TOUCHDOCKER_DOCK_H

#include <QDockWidget>
#include <QIcon>
#include <KoCanvasObserverBase.h>
#include <QPointer>

#include <kis_canvas2.h>

class KoDialog;
class QQuickWidget;

class TouchDockerDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
    Q_PROPERTY(bool allowClose READ allowClose WRITE setAllowClose)
    Q_PROPERTY(QString currentSketchPage READ currentSketchPage WRITE setCurrentSketchPage NOTIFY currentSketchPageChanged)
    Q_PROPERTY(QObject* sketchKisView READ sketchKisView WRITE setSketchKisView NOTIFY sketchKisViewChanged)

public:
    TouchDockerDock();
    ~TouchDockerDock() override;
    QString observerName() override { return "TouchDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

    bool allowClose() const;
    void setAllowClose(bool allow);

    QString currentSketchPage() const;
    void setCurrentSketchPage(QString newPage);

    QObject *sketchKisView() const;
    void setSketchKisView(QObject *newView);

    virtual void closeEvent(QCloseEvent *event);

Q_SIGNALS:
    void closeRequested();
    void currentSketchPageChanged();
    void sketchKisViewChanged();

public Q_SLOTS:

    void slotButtonPressed(const QString &id);
    void slotOpenImage(QString path);
    void slotSaveAs(QString path, QString mime);

    void hideFileOpenDialog();
    void hideFileSaveAsDialog();

    QString imageForButton(QString id);
    QString textForButton(QString id);
    QAction *action(QString id) const;

private:

    void showFileOpenDialog();
    void showFileSaveAsDialog();
    void changeEvent(QEvent* event) override;

    void tabletEvent(QTabletEvent *event) override;

    KoDialog *createDialog(const QString qml);

    QPointer<KisCanvas2> m_canvas;
    QQuickWidget *m_quickWidget {0};

    class Private;
    const QScopedPointer<Private> d;

};


#endif

