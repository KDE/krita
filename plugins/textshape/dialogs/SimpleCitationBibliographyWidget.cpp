/* This file is part of the KDE project
 * Copyright (C) 2010 C. Boemann <cbo@boemann.dk>
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

#include "SimpleCitationBibliographyWidget.h"
#include "ReferencesTool.h"
#include "BibliographyPreview.h"
#include "BibliographyTemplate.h"
#include <KoBibliographyInfo.h>
#include <KoTextEditor.h>

#include <QAction>
#include <QDebug>

#include <QWidget>
#include <QSignalMapper>

SimpleCitationBibliographyWidget::SimpleCitationBibliographyWidget(ReferencesTool *tool, QWidget *parent)
    : QWidget(parent)
    , m_blockSignals(false)
    , m_referenceTool(tool)
    , m_signalMapper(0)
{
    widget.setupUi(this);
    Q_ASSERT(tool);

    m_templateGenerator = new BibliographyTemplate(KoTextDocument(m_referenceTool->editor()->document()).styleManager());

    widget.addCitation->setDefaultAction(tool->action("insert_citation"));
    connect(widget.addCitation, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));

    widget.addBibliography->setDefaultAction(tool->action("insert_bibliography"));
    widget.addBibliography->setNumColumns(1);
    connect(widget.addBibliography, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.addBibliography, SIGNAL(aboutToShowMenu()), this, SLOT(prepareTemplateMenu()));
    connect(widget.addBibliography, SIGNAL(itemTriggered(int)), this, SLOT(applyTemplate(int)));

    widget.configureBibliography->setDefaultAction(tool->action("configure_bibliography"));
    connect(widget.configureBibliography, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
}

SimpleCitationBibliographyWidget::~SimpleCitationBibliographyWidget()
{
    delete m_templateGenerator;
}

void SimpleCitationBibliographyWidget::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
}

void SimpleCitationBibliographyWidget::prepareTemplateMenu()
{
    m_previewGenerator.clear();
    if (m_signalMapper) {
        delete m_signalMapper;
        m_signalMapper = 0;
    }
    qDeleteAll(m_templateList.begin(), m_templateList.end());
    m_templateList.clear();

    m_signalMapper = new QSignalMapper();

    m_templateList = m_templateGenerator->templates();

    connect(m_signalMapper, SIGNAL(mapped(int)), this, SLOT(pixmapReady(int)));

    int index = 0;
    foreach (KoBibliographyInfo *info, m_templateList) {
        BibliographyPreview *preview = new BibliographyPreview();
        preview->setStyleManager(KoTextDocument(m_referenceTool->editor()->document()).styleManager());
        preview->setPreviewSize(QSize(200, 120));
        preview->updatePreview(info);
        connect(preview, SIGNAL(pixmapGenerated()), m_signalMapper, SLOT(map()));
        m_signalMapper->setMapping(preview, index);
        m_previewGenerator.append(preview);
        ++index;

        //put dummy pixmaps until the actual pixmap previews are generated and added in pixmapReady()
        if (!widget.addBibliography->hasItemId(index)) {
            QPixmap pmm(QSize(200, 120));
            pmm.fill(Qt::white);
            widget.addBibliography->addItem(pmm, index);
        }
    }
    if (widget.addBibliography->isFirstTimeMenuShown()) {
        widget.addBibliography->addSeparator();
        widget.addBibliography->addAction(m_referenceTool->action("insert_custom_bibliography"));
        connect(m_referenceTool->action("insert_custom_bibliography"),
                SIGNAL(triggered()), this, SLOT(insertCustomBibliography()), Qt::UniqueConnection);
    }
}

void SimpleCitationBibliographyWidget::insertCustomBibliography()
{
    m_templateGenerator->moveTemplateToUsed(m_templateList.at(0));
    m_referenceTool->insertCustomBibliography(m_templateList.at(0));
}

void SimpleCitationBibliographyWidget::pixmapReady(int templateId)
{
    // +1 to the templateId is because formattingButton does not allow id = 0
    widget.addBibliography->addItem(m_previewGenerator.at(templateId)->previewPixmap(), templateId + 1);
    disconnect(m_previewGenerator.at(templateId), SIGNAL(pixmapGenerated()), m_signalMapper, SLOT(map()));
    m_previewGenerator.at(templateId)->deleteLater();
}

void SimpleCitationBibliographyWidget::applyTemplate(int templateId)
{
    m_templateGenerator->moveTemplateToUsed(m_templateList.at(templateId - 1));
    m_referenceTool->editor()->insertBibliography(m_templateList.at(templateId - 1));
}
