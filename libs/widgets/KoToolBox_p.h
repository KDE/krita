/*
 * SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2005-2008 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2009 Peter Simonsson <peter.simonsson@gmail.com>
 * SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef _KO_TOOLBOX_H_
#define _KO_TOOLBOX_H_

#include <KoCanvasObserverBase.h>

#include <QWidget>
#include <QList>

#include <KoToolManager.h>

class KoCanvasController;
class KoShapeLayer;
class KoToolBoxLayout;

/**
 * KoToolBox is a dock widget that can order tools according to type and
 * priority.
 *
 * The ToolBox is a container for tool buttons which are themselves
 * divided into sections.
 *
 * Adding buttons using addButton() will allow you to show those buttons.  You should connect
 * the button to your handling method yourself.
 *
 * The unique property of this toolbox is that it can be shown horizontal as well as vertical,
 * rotating in a smart way to show the buttons optimally.
 * @see KoToolManager
 */
class KoToolBox : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit KoToolBox();
    ~KoToolBox() override;

public Q_SLOTS:
    /**
     * Set the new active button based on the currently active tool.
     * @param canvas the currently active canvas.
     */
    void setActiveTool(KoCanvasController *canvas);

    /**
     * Show only the dynamic buttons that have a code from parameter codes.
     * The toolbox allows buttons to be optionally registered with a visibilityCode. This code
     * can be passed here and all buttons that have that code are shown. All buttons that
     * have another visibility code registered are hidden.
     * @param codes a list of all the codes to show.
     */
    void setButtonsVisible(const QList<QString> &codes);


    /// Set the orientation of the layout to @p orientation
    void setOrientation(Qt::Orientation orientation);

    void setFloating(bool v);

    KoToolBoxLayout *toolBoxLayout() const;

private:
    /**
     * Add a button to the toolbox.
     * The buttons should all be added before the first showing since adding will not really add
     * them to the UI until setup() is called.
     *
     * @param toolAction the action of the tool
     * @see setup()
     */
    void addButton(KoToolAction *toolAction);

private Q_SLOTS:
    void setCurrentLayer(const KoCanvasController *canvas, const KoShapeLayer* newLayer);

    /// add a tool post-initialization. The tool will also be activated.
    void toolAdded(KoToolAction *toolAction, KoCanvasController *canvas);

    /// set the icon size for all the buttons
    void slotContextIconSize();

protected:
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    class Private;
    Private * const d;
};

#endif // _KO_TOOLBOX_H_
