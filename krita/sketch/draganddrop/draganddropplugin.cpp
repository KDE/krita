/*
    Copyright 2011 by Marco Martin <mart@kde.org>

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
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

