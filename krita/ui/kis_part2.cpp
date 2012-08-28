/* This file is part of the KDE project
   Copyright (C) 2012 C. Boemann <cbo@kogmbh.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "kis_part2.h"
#include "kis_view2.h"
#include "kis_doc2.h"
#include "kis_factory2.h"
#include "kis_config.h"
#include "kis_clipboard.h"
#include "kis_custom_image_widget.h"
#include "kis_shape_controller.h"

#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceEngine.h>
#include <KoCanvasBase.h>
#include <KoToolManager.h>
#include <KoInteractionTool.h>
#include <KoShapeRegistry.h>
#include <KoShapeManager.h>
#include <KoDocument.h>
#include <KoShapeBasedDocumentBase.h>

#include <kapplication.h>
#include <kglobal.h>
#include <kmessagebox.h>

#include <QApplication>

KisPart2::KisPart2(QObject *parent)
    : KoPart(parent)
    , m_dieOnError(false)
{
    setComponentData(KisFactory2::componentData(), false);
    setTemplateType("krita_template");
}

KisPart2::~KisPart2()
{
}

void KisPart2::setDocument(KisDoc2 *document)
{
    KoPart::setDocument(document);
    m_document = document;
}

KoView *KisPart2::createViewInstance(QWidget *parent)
{
    qApp->setOverrideCursor(Qt::WaitCursor);
    KisView2 *v = new KisView2(this, m_document, parent);

    //XXX : fix this ugliness
    dynamic_cast<KisShapeController*>(m_document->shapeController())->setInitialShapeForView(v);
    KoToolManager::instance()->switchToolRequested("KritaShape/KisToolBrush");

    // XXX: this prevents a crash when opening a new document after opening a
    // a document that has not been touched! I have no clue why, though.
    // see: https://bugs.kde.org/show_bug.cgi?id=208239.
    setModified(true);
    setModified(false);
    qApp->restoreOverrideCursor();

    return v;
}

QGraphicsItem *KisPart2::createCanvasItem()
{
    // XXX: It's time we implement this!
    return 0;
}

void KisPart2::showStartUpWidget(KoMainWindow *parent, bool alwaysShow)
{
    // print error if the lcms engine is not available
    if (!KoColorSpaceEngineRegistry::instance()->contains("icc")) {
        // need to wait 1 event since exiting here would not work.
        m_errorMessage = i18n("The Calligra LittleCMS color management plugin is not installed. Krita will quit now.");
        m_dieOnError = true;
        QTimer::singleShot(0, this, SLOT(showErrorAndDie()));
    }

    KoPart::showStartUpWidget(parent, alwaysShow);

    KisConfig cfg;
    if (cfg.firstRun()) {
        QStringList qtversion = QString(qVersion()).split('.');
        if (qtversion[0] == "4" && qtversion[1] <= "6" && qtversion[2].toInt() < 3) {
            m_errorMessage = i18n("Krita needs at least Qt 4.6.3 to work correctly. Your Qt version is %1. If you have a graphics tablet it will not work correctly!", qVersion());
            m_dieOnError = false;
            QTimer::singleShot(0, this, SLOT(showErrorAndDie()));
        }

        cfg.setFirstRun(false);
    }
}

QList<KoPart::CustomDocumentWidgetItem> KisPart2::createCustomDocumentWidgets(QWidget *parent)
{
    KisConfig cfg;

    int w = cfg.defImageWidth();
    int h = cfg.defImageHeight();
    bool clipAvailable = false;

    QSize sz = KisClipboard::instance()->clipSize();
    if (sz.isValid() && sz.width() != 0 && sz.height() != 0) {
        w = sz.width();
        h = sz.height();
        clipAvailable = true;
    }

    QList<KoPart::CustomDocumentWidgetItem> widgetList;
    KoPart::CustomDocumentWidgetItem item;
    item.widget = new KisCustomImageWidget(parent, qobject_cast<KisDoc2*>(document()), w, h, clipAvailable, cfg.defImageResolution(), cfg.defColorModel(), cfg.defColorDepth(), cfg.defColorProfile(), "unnamed");
    widgetList << item;

    return widgetList;
}



void KisPart2::showErrorAndDie()
{
    KMessageBox::error(widget(),
                       m_errorMessage,
                       i18n("Installation error"));
    if (m_dieOnError) {
        exit(10);
    }
}

