/*
 * Copyright (C) 2006-2008 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#include "ShapeSelector.h"
#include "IconShape.h"
#include "TemplateShape.h"
#include "Canvas.h"
#include "FolderShape.h"

#include <KoShapeManager.h>
#include <KoToolManager.h>
#include <KoSelection.h>
#include <KoCreateShapesTool.h>
#include <KoCanvasController.h>

#include <QFile>
#include <KLocale>
#include <KMessageBox>
#include <kio/netaccess.h>

// ************** ShapeSelector ************
ShapeSelector::ShapeSelector(QWidget *parent)
    : QDockWidget(i18n("Shapes"), parent),
    m_itemStore(this)
{
    setObjectName("ShapeSelector");
}

ShapeSelector::~ShapeSelector()
{
}

void ShapeSelector::itemSelected()
{
    KoShape *koShape = m_itemStore.shapeManager()->selection()->firstSelectedShape();
    if (koShape == 0)
        return;
    IconShape *shape= dynamic_cast<IconShape*>(koShape);
    if (shape == 0)
        return;
    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();

    if (canvasController) {
        KoCreateShapesTool * tool = KoToolManager::instance()->shapeCreatorTool( canvasController->canvas() );
        shape->visit( tool );
        KoToolManager::instance()->switchToolRequested(KoCreateShapesTool_ID);
    }
}

void ShapeSelector::setSize(const QSize &size)
{
    if (m_itemStore.mainFolder())
        m_itemStore.mainFolder()->setSize(QSizeF(size));
}

void ShapeSelector::addItems(const KUrl &url, FolderShape *targetFolder)
{
    QString localFile;
    if (KIO::NetAccess::download(url, localFile, this)) {
        QFile file(localFile);
        addItems(file, targetFolder);
        KIO::NetAccess::removeTempFile(localFile);
    } else {
        KMessageBox::error(this, KIO::NetAccess::lastErrorString() );
    }
}

void ShapeSelector::addItems(QFile &file, FolderShape *targetFolder)
{
    QDomDocument doc;
    if (doc.setContent(&file)) {
        if (targetFolder == 0)
            targetFolder = m_itemStore.folders()[0];

        QDomElement root = doc.firstChildElement();
        QDomElement element = root.firstChildElement();
        while (!element.isNull()) {
            if (element.tagName() == "template") {
                TemplateShape *ts = TemplateShape::createShape(element);
                targetFolder->addShape(ts);
                m_itemStore.addShape(ts);
            }
            element = root.nextSiblingElement();
        }
    }
    file.close();
}

#include <ShapeSelector.moc>
