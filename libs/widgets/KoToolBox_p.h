/*
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2005-2008 Thomas Zander <zander@kde.org>
 * Copyright (c) 2009 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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
     * Using the buttongroup id passed in addButton() you can set the new active button.
     * If the id does not resolve to a visible button, this call is ignored.
     * @param canvas the currently active canvas.
     * @param id an id to identify the button to activate.
     */
    void setActiveTool(KoCanvasController *canvas, int id);

    /**
     * Show only the dynamic buttons that have a code from parameter codes.
     * The toolbox allows buttons to be optionally registered with a visibilityCode. This code
     * can be passed here and all buttons that have that code are shown. All buttons that
     * have another visibility code registered are hidden.
     * @param canvas the currently active canvas.
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
