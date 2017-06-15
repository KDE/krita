
/*
 * Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <ToolReferenceImages.h>

#include <QPainter>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDesktopServices>
#include <QFile>
#include <QLineF>

#include <kis_debug.h>
#include <klocalizedstring.h>
#include <QMessageBox>

#include <KoIcon.h>
#include <kis_icon.h>
#include <KoFileDialog.h>
#include <KoViewConverter.h>
#include <KoPointerEvent.h>

#include <canvas/kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <KisViewManager.h>

#include "kis_global.h"

#include <math.h>

ToolReferenceImages::ToolReferenceImages(KoCanvasBase * canvas)
    : KisTool(canvas, KisCursor::arrowCursor())
{
    setObjectName("ToolReferenceImages");
}

ToolReferenceImages::~ToolReferenceImages()
{
}

void ToolReferenceImages::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    // Add code here to initialize your tool when it got activated
    KisTool::activate(toolActivation, shapes);
}

void ToolReferenceImages::deactivate()
{
    KisTool::deactivate();
}

void ToolReferenceImages::beginPrimaryAction(KoPointerEvent *event)
{
}

void ToolReferenceImages::continuePrimaryAction(KoPointerEvent *event)
{
}

void ToolReferenceImages::endPrimaryAction(KoPointerEvent *event)
{
    setMode(KisTool::HOVER_MODE);
    event->ignore();
}

void ToolReferenceImages::mouseMoveEvent(KoPointerEvent *event)
{
}

void ToolReferenceImages::paint(QPainter& _gc, const KoViewConverter &_converter)
{

}

void ToolReferenceImages::removeAllReferenceImages()
{
}

void ToolReferenceImages::loadReferenceImages()
{
/*
    KoFileDialog dialog(m_canvas->viewManager()->mainWindow(), KoFileDialog::OpenFile, "OpenReferenceImage");
    dialog.setCaption(i18n("Select a Reference Image"));
    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
    // dialog.setMimeTypeFilters(QStringList() << "application/x-krita-assistant", "application/x-krita-");
    QString filename = dialog.filename();
    if (filename.isEmpty()) return;
    if (!QFileInfo(filename).exists()) return;

    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    m_canvas->updateCanvas();
*/
}

void ToolReferenceImages::saveReferenceImages()
{
}



QWidget *ToolReferenceImages::createOptionWidget()
{
/*        
    if (!m_optionsWidget) {
        m_optionsWidget = new QWidget;
        m_options.setupUi(m_optionsWidget);

        // See https://bugs.kde.org/show_bug.cgi?id=316896
        QWidget *specialSpacer = new QWidget(m_optionsWidget);
        specialSpacer->setObjectName("SpecialSpacer");
        specialSpacer->setFixedSize(0, 0);
        m_optionsWidget->layout()->addWidget(specialSpacer);

        m_options.loadButton->setIcon(KisIconUtils::loadIcon("document-open"));
        m_options.saveButton->setIcon(KisIconUtils::loadIcon("document-save"));
        m_options.deleteButton->setIcon(KisIconUtils::loadIcon("edit-delete"));

        QList<KoID> assistants;
        Q_FOREACH (const QString& key, KisPaintingAssistantFactoryRegistry::instance()->keys()) {
            QString name = KisPaintingAssistantFactoryRegistry::instance()->get(key)->name();
            assistants << KoID(key, name);
        }
        qSort(assistants.begin(), assistants.end(), KoID::compareNames);
        Q_FOREACH(const KoID &id, assistants) {
            m_options.comboBox->addItem(id.name(), id.id());
        }

        connect(m_options.saveButton, SIGNAL(clicked()), SLOT(saveAssistants()));
        connect(m_options.loadButton, SIGNAL(clicked()), SLOT(loadAssistants()));
        connect(m_options.deleteButton, SIGNAL(clicked()), SLOT(removeAllAssistants()));
    }
    return m_optionsWidget;
*/    
    return 0;
}

