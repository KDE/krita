/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 KO GmbH <boud@kogmbh.com>
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

#ifndef KOTOOLPRIVATE_H
#define KOTOOLPRIVATE_H

#include <QMap>
#include <QHash>
#include <QWidget>
#include <QString>
#include <QSet>

class QWidget;
class KAction;
class KoToolBase;
class KoCanvasBase;

class KoToolBasePrivate
{
public:
    KoToolBasePrivate(KoToolBase *qq, KoCanvasBase *canvas_)
        : currentCursor(Qt::ArrowCursor),
        q(qq),
        canvas(canvas_),
        readWrite(true)
    {
    }

    ~KoToolBasePrivate()
    {
#if QT_VERSION < 0x040603        
        foreach(QWidget *optionWidget, optionWidgets) {
            optionWidget->setParent(0);
            delete optionWidget;
        }
        optionWidgets.clear();
#else
        qDeleteAll(optionWidgets);
#endif
    }

    QMap<QString, QWidget *> optionWidgets; ///< the optionwidgets associated with this tool
    QCursor currentCursor;
    QHash<QString, KAction*> actionCollection;
    QString toolId;
    QList<QAction*> popupActionList;
    QSet<KAction*> readOnlyActions;
    KoToolBase *q;
    KoCanvasBase *canvas; ///< the canvas interface this tool will work for.
    bool readWrite;
};

#endif
