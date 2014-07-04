/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 * Copyright (C) 2013 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class KisCanvasResourceProvider;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(bool allowClose READ allowClose WRITE setAllowClose)
    Q_PROPERTY(bool slateMode READ slateMode NOTIFY slateModeChanged)
    Q_PROPERTY(QString applicationName READ applicationName CONSTANT)
    Q_PROPERTY(QString currentSketchPage READ currentSketchPage WRITE setCurrentSketchPage NOTIFY currentSketchPageChanged)
    Q_PROPERTY(bool temporaryFile READ temporaryFile WRITE setTemporaryFile NOTIFY temporaryFileChanged)
    Q_PROPERTY(QObject* sketchKisView READ sketchKisView WRITE setSketchKisView NOTIFY sketchKisViewChanged)

public:
    explicit MainWindow(QStringList fileNames, QWidget* parent = 0, Qt::WindowFlags flags = 0);
    virtual ~MainWindow();

    bool allowClose() const;
    void setAllowClose(bool allow);

    bool slateMode() const;
    void setSlateMode(bool newValue);

    QString applicationName() const {
        return QLatin1String("KRITA GEMINI");
    }

    QString currentSketchPage() const;
    void setCurrentSketchPage(QString newPage);

    bool temporaryFile() const;
    void setTemporaryFile(bool newValue);

    QObject* sketchKisView() const;
    void setSketchKisView(QObject* newView);

    virtual void closeEvent(QCloseEvent* event);

    Q_INVOKABLE QString openImage();

    bool forceFullScreen();
    void forceFullScreen(bool newValue);
public Q_SLOTS:
    void minimize();
    void closeWindow();

    void switchToSketch();
    void switchToDesktop(bool justLoaded = false);
    void documentChanged();
    void resetWindowTitle();
    void resourceChanged(int key, const QVariant& v);
    void resourceChangedSketch(int key, const QVariant& v);

Q_SIGNALS:
    void closeRequested();
    void switchedToSketch();
    void slateModeChanged();
    void currentSketchPageChanged();
    void temporaryFileChanged();
    void sketchKisViewChanged();
    void documentSaved();

private Q_SLOTS:
    void switchDesktopForced();
    void switchSketchForced();
    void adjustZoomOnDocumentChangedAndStuff();
    void sketchChange();

private:
    void cloneResources(KisCanvasResourceProvider *from, KisCanvasResourceProvider *to);
    class Private;
    Private * const d;

#ifdef Q_OS_WIN
    bool winEvent(MSG * message, long * result);
#endif
};

#endif // MAINWINDOW_H
