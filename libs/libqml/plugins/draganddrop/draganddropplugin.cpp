/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "draganddropplugin.h"

#include "DeclarativeDragArea.h"
#include "DeclarativeDragDropEvent.h"
#include "DeclarativeDropArea.h"
#include "DeclarativeMimeData.h"

void DragAndDropPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.krita.draganddrop"));

    qmlRegisterType<DeclarativeDropArea>(uri, 1, 0, "DropArea");
    qmlRegisterType<DeclarativeDragArea>(uri, 1, 0, "DragArea");
    qmlRegisterUncreatableType<DeclarativeMimeData>(uri, 1, 0, "MimeData", "MimeData cannot be created from QML.");
    qmlRegisterUncreatableType<DeclarativeDragDropEvent>(uri, 1, 0, "DragDropEvent", "DragDropEvent cannot be created from QML.");
}

