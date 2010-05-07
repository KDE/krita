/*
 * Copyright (C) 2007-2008 Thomas Zander <zander@kde.org>
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
#include "FolderShape.h"
#include "FolderShapeModel.h"
#include "IconShape.h"
#include "TemplateShape.h"
#include "ClipboardProxyShape.h"
#include "ItemStore.h"

#include <KoViewConverter.h>

#include <QPainter>

FolderShape::FolderShape()
    : KoShapeContainer(new FolderShapeModel(this))
{
}

void FolderShape::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

void FolderShape::setSize(const QSizeF &size)
{
    KoShapeContainer::setSize(size);
    static_cast<FolderShapeModel*>(model())->folderResized();
}

QDomDocument FolderShape::save()
{
    QDomDocument doc;
    QDomElement element = doc.createElement("book");
    doc.appendChild(element);
    foreach (KoShape *child, shapes()) {
        IconShape *ic = dynamic_cast<IconShape*>(child);
        if (ic) {
            ic->save(element);
            continue;
        }
        ClipboardProxyShape *proxy = dynamic_cast<ClipboardProxyShape*>(child);
        if (proxy) {
            QDomElement clipboard = doc.createElement("clipboard");
            element.appendChild(clipboard);
            QDomText text = doc.createCDATASection( QString::fromAscii( proxy->clipboardData(), proxy->clipboardData().size() ) );
            clipboard.appendChild(text);
            continue;
        }
    }
    return doc;
}

void FolderShape::load(const QDomDocument &document)
{
    QDomElement root = document.firstChildElement();
    QDomElement item = root.firstChildElement();
    while (!item.isNull()) {
        if (item.tagName() == "template") {
            TemplateShape *t = TemplateShape::createShape(item);
            Q_ASSERT(t);
            addShape(t);
        } else if (item.tagName() == "clipboard") {
            QByteArray data = item.text().toLatin1();
            KoShape *clipboardShape = ItemStore::createShapeFromPaste(data);
            if (clipboardShape) {
                ClipboardProxyShape *proxy = new ClipboardProxyShape(clipboardShape, data);
                ItemStore is;
                is.setClipboardShape(proxy);
                proxy->setParent(this);
            }
        }
        item = item.nextSiblingElement();
    }
}

