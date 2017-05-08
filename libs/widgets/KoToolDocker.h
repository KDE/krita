/* This file is part of the KDE project
 *
 * Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2006 Thomas Zander <zander@kde.org>
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
#ifndef KO_TOOL_DOCKER_H
#define KO_TOOL_DOCKER_H

#include <QDockWidget>
#include <QPointer>

class QWidget;

#include <kritawidgets_export.h>
#include <KoCanvasObserverBase.h>
/**
 * The tool docker shows the tool option widget associated with the
 * current tool and the current canvas.
 */
class KRITAWIDGETS_EXPORT KoToolDocker : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    explicit KoToolDocker(QWidget *parent = 0);
    ~KoToolDocker() override;

    void resetWidgets();

    /// reimplemented
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;


protected:
    void resizeEvent(QResizeEvent* event) override; ///< reimplemented from QWidget
public Q_SLOTS:
    /**
     * Update the option widgets to the argument one, removing the currently set widget.
     */
    void setOptionWidgets(const QList<QPointer<QWidget> > &optionWidgetList);

    /**
     * Returns whether the docker has an optionwidget attached
     */
    bool hasOptionWidget();

    /**
     * set the tab option
     */
    void setTabEnabled(bool enabled);

private:
    Q_PRIVATE_SLOT(d, void toggleTab())
    Q_PRIVATE_SLOT(d, void locationChanged(Qt::DockWidgetArea area))

    class Private;
    Private * const d;
};

#endif
