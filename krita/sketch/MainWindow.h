/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Q_PROPERTY(bool allowClose READ allowClose WRITE setAllowClose)
    Q_PROPERTY(QString currentSketchPage READ currentSketchPage WRITE setCurrentSketchPage NOTIFY currentSketchPageChanged)
	Q_PROPERTY(QObject* sketchKisView READ sketchKisView WRITE setSketchKisView NOTIFY sketchKisViewChanged)
public:
    explicit MainWindow(QStringList fileNames, QWidget* parent = 0, Qt::WindowFlags flags = 0);
    virtual ~MainWindow();

    bool allowClose() const;
    void setAllowClose(bool allow);

    QString currentSketchPage() const;
    void setCurrentSketchPage(QString newPage);

	QObject* sketchKisView() const;
	void setSketchKisView(QObject* newView);

    virtual void resizeEvent(QResizeEvent* event);
    virtual void closeEvent(QCloseEvent* event);

public Q_SLOTS:
    void minimize();
    void closeWindow();
	void adjustZoomOnDocumentChangedAndStuff();
    void resetWindowTitle();

Q_SIGNALS:
    void closeRequested();
    void switchedToSketch();
    void currentSketchPageChanged();
	void sketchKisViewChanged();

private:
    class Private;
    Private * const d;
};

#endif // MAINWINDOW_H
