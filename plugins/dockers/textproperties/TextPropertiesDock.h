/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TEXTPROPERTIESDOCK_H
#define TEXTPROPERTIESDOCK_H

#include <QDockWidget>
#include <QIcon>
#include <KoCanvasObserverBase.h>
#include <QPointer>

#include <kis_canvas2.h>

class KoDialog;
class QQuickWidget;

class TextPropertiesDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    TextPropertiesDock();
    ~TextPropertiesDock();
    QString observerName() override { return "TextPropertiesDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

private:
    QPointer<KisCanvas2> m_canvas;
    QQuickWidget *m_quickWidget {0};

    class Private;
    const QScopedPointer<Private> d;
};

#endif // TEXTPROPERTIESDOCK_H
