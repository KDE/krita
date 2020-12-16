/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KO_TOOL_DOCKER_H
#define KO_TOOL_DOCKER_H

#include <QDockWidget>
#include <QPointer>
#include <KisKineticScroller.h>

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

public Q_SLOTS:
    /**
     * Update the option widgets to the argument one, removing the currently set widget.
     */
    void setOptionWidgets(const QList<QPointer<QWidget> > &optionWidgetList);
    void slotScrollerStateChange(QScroller::State state);

    /**
     * Returns whether the docker has an optionwidget attached
     */
    bool hasOptionWidget();

private:
    Q_PRIVATE_SLOT(d, void locationChanged(Qt::DockWidgetArea area))

    class Private;
    Private * const d;
};

#endif
