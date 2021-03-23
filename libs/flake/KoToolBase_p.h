/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2010 KO GmbH <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOTOOLBASE_P_H
#define KOTOOLBASE_P_H

#include "KoDocumentResourceManager.h"
#include "KoCanvasResourceProvider.h"
#include "KoCanvasBase.h"
#include "KoShapeController.h"
#include <QHash>
#include <QWidget>
#include <QString>
#include <QPointer>
#include <string.h> // for the qt version check

class QAction;
class KoToolBase;
class KoToolFactoryBase;

class KoToolBasePrivate
{
public:
    KoToolBasePrivate(KoToolBase *qq, KoCanvasBase *canvas_)
        : currentCursor(Qt::ArrowCursor),
        q(qq),
        canvas(canvas_),
        isInTextMode(false),
        isActivated(false)
    {
    }

    ~KoToolBasePrivate()
    {
        Q_FOREACH (QPointer<QWidget> optionWidget, optionWidgets) {
            if (optionWidget) {
                optionWidget->setParent(0);
                delete optionWidget;
            }
        }
        optionWidgets.clear();
    }

    void connectSignals()
    {
        if (canvas) { // in the case of KoToolManagers dummytool it can be zero :(
            KoCanvasResourceProvider * crp = canvas->resourceManager();
            Q_ASSERT_X(crp, "KoToolBase::KoToolBase", "No Canvas KoResourceManager");
            if (crp)
                q->connect(crp, SIGNAL(canvasResourceChanged(int, const QVariant &)),
                        SLOT(canvasResourceChanged(int, const QVariant &)));

            // can be 0 in the case of Calligra Sheets
            KoDocumentResourceManager *scrm = canvas->shapeController()->resourceManager();
            if (scrm) {
                q->connect(scrm, SIGNAL(resourceChanged(int, const QVariant &)),
                        SLOT(documentResourceChanged(int, const QVariant &)));
            }
        }
    }

    QList<QPointer<QWidget> > optionWidgets; ///< the optionwidgets associated with this tool
    bool optionWidgetsCreated {false};
    QCursor currentCursor;
    KoToolBase *q;
    KoToolFactoryBase *factory {0};
    KoCanvasBase *canvas; ///< the canvas interface this tool will work for.
    bool isInTextMode;
    bool maskSyntheticEvents{false}; ///< Whether this tool masks synthetic events
    bool isActivated;
    QRectF lastDecorationsRect;
};

#endif
