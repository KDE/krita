/*
 * Copyright (C) 2008 Thomas Zander <zander@kde.org>
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
#include "RightClickStrategy.h"
#include "FolderShape.h"
#include "SelectStrategy.h"
#include "Canvas.h"
#include "IconShape.h"
#include "FolderBorder.h"
#include <KoShapeContainerModel.h>

#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoPointerEvent.h>

#include <QMenu>
#include <QAction>
#include <QDomDocument>
#include <QFile>
#include <QMouseEvent>

#include <KLocale>

RightClickStrategy::RightClickStrategy(Canvas *canvas, KoShape *clickedShape, KoPointerEvent &event)
    : m_canvas(canvas),
    m_clickedShape(clickedShape)
{
    m_lastPosition = event.point;
    SelectStrategy(canvas, clickedShape, event); // properly updates the selection
}

void RightClickStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    m_lastPosition = mouseLocation;
}

void RightClickStrategy::finishInteraction( Qt::KeyboardModifiers modifiers )
{
    Q_UNUSED(modifiers);
    QMenu menu;
    QAction *save = 0, *zoomInAction = 0, *zoomOutAction = 0, *removeFolder = 0;
    QAction *load = new QAction(i18n("Open..."), &menu);
    menu.addAction(load);

    if (m_canvas->shapeManager()->selection()->count()) {
        save = new QAction(i18n("Save Book..."), &menu);
        menu.addAction(save);
    }
    QAction *newFolder = new QAction(i18n("New Folder..."), &menu);
    menu.addAction(newFolder);

    if (m_canvas->zoomIndex() < 2) {
        zoomInAction = new QAction(i18n("Zoom In"), &menu);
        menu.addAction(zoomInAction);
    }
    if (m_canvas->zoomIndex() >= 0) {
        zoomOutAction = new QAction(i18n("Zoom Out"), &menu);
        menu.addAction(zoomOutAction);
    }
    FolderShape *folder = dynamic_cast<FolderShape*>(m_clickedShape);
    if (folder && folder != m_canvas->itemStore()->mainFolder()) {
        removeFolder = new QAction(i18n("Remove Folder"), &menu);
        menu.addAction(removeFolder);
    }

    QAction *selected = m_canvas->popup(&menu, m_lastPosition);
    if (selected == 0)
        return;
    if (selected == save) {
        saveSelection();
    } else if (selected == load) {
        this->load();
    } else if (selected == newFolder) {
        createNewFolder();
    } else if (selected == zoomInAction) {
        m_canvas->zoomIn(m_lastPosition);
    } else if (selected == zoomOutAction) {
        m_canvas->zoomOut(m_lastPosition);
    } else if (selected == removeFolder) {
        Q_ASSERT(folder);
        m_canvas->itemStore()->removeFolder(folder);
        FolderShape *main = m_canvas->itemStore()->mainFolder();
        if (main) {
            main->update();
            main->setBorder(0);
            main->setSize(m_canvas->size());
            main->update();
            main->setPosition(QPointF(0,0));
            m_canvas->resetDocumentOffset();
        }
        delete folder;
    }
}

void RightClickStrategy::createNewFolder()
{
    FolderShape *fs = new FolderShape();
    // TODO show dialog to name the folder
    fs->setName("New folder");
    if (m_canvas->itemStore()->folders().count() == 1 && m_canvas->itemStore()->folders()[0]->border() == 0) {
        // the first new folder, lets be kinds and resize the folders to make it managable for the user.
        FolderShape *oldFolder = m_canvas->itemStore()->folders()[0];
        oldFolder->update();
        oldFolder->setBorder(new FolderBorder());
        KoInsets insets = oldFolder->borderInsets();
        oldFolder->setPosition(QPointF(insets.left, insets.top));
        QSizeF size = oldFolder->size(); // we can assume size is the size of the whole widget. So we just make each use half of that.
        size.setWidth(size.width() / 2.);
        oldFolder->setSize(size);
        if (oldFolder->model()->count()) {
            // Make sure we don't resize the folder to small for its contents.
            QPointF bottom = oldFolder->model()->shapes().last()->absolutePosition(KoFlake::BottomRightCorner);
            if (bottom.y() > size.height())
                oldFolder->setSize(QSizeF(size.width(), bottom.y()));
        }
        oldFolder->update();

        fs->setPosition(QPointF(size.width(), insets.top));
        fs->setSize(size);
    } else {
        fs->setAbsolutePosition(m_lastPosition);
        fs->setSize(QSizeF(150, 100));
    }

    m_canvas->itemStore()->addFolder(fs);
}

void RightClickStrategy::saveSelection()
{
    KoShape *shape = m_canvas->shapeManager()->selection()->firstSelectedShape();
    FolderShape *fs = dynamic_cast<FolderShape*>(shape);
    if (fs == 0)
        fs = dynamic_cast<FolderShape*>(shape->parent());
    if (fs == 0) // TODO give feedback ?
        return;

    // TODO open file dialog
    QFile file("foo");
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QDomDocument doc = fs->save();
    file.write(doc.toByteArray());
    file.close();
}

void RightClickStrategy::load()
{
    //m_canvas->m_parent->addItems(KUrl(file.trimmed()), folder);
}

