/* This file is part of the KDE project

   Copyright (c) 2010 Cyril Oblikov <munknex@gmail.com>

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

#include "TreeShapeConfigWidget.h"
#include "TreeShape.h"
#include "TreeTool.h"

#include <klocale.h>
#include "kdebug.h"

TreeShapeConfigWidget::TreeShapeConfigWidget(TreeTool *tool, QWidget *parent)
        : QWidget(parent), m_tool(tool)
{
    widget.setupUi(this);

    widget.treeType->clear();
    widget.treeType->addItem(i18n("Organigram (Down)"));
    widget.treeType->addItem(i18n("Organigram (Up)"));
    widget.treeType->addItem(i18n("Organigram (Left)"));
    widget.treeType->addItem(i18n("Organigram (Right)"));
//     widget.treeType->addItem(i18n("Tree (Left)"));
//     widget.treeType->addItem(i18n("Tree (Right)"));
//     widget.treeType->addItem(i18n("Map (Clockwise)"));
//     widget.treeType->addItem(i18n("Map (AntiClockwise)"));
//     widget.treeType->addItem(i18n("Follow Parent"));

    widget.rootType->clear();
    widget.rootType->addItem(i18n("Rectangle"));
    widget.rootType->addItem(i18n("Ellipse"));
    widget.rootType->addItem(i18n("None"));
//     widget.rootType->addItem(i18n("Follow Parent"));

    widget.connectionType->clear();
    widget.connectionType->addItem(i18n("Standart"));
    widget.connectionType->addItem(i18n("Lines"));
    widget.connectionType->addItem(i18n("Straight"));
    widget.connectionType->addItem(i18n("Curve"));
//     widget.connectionType->addItem(i18n("Follow Parent"));

    connect(widget.treeType, SIGNAL(currentIndexChanged(int)), m_tool, SLOT(changeStructure(int)));
    connect(widget.rootType, SIGNAL(currentIndexChanged(int)), m_tool, SLOT(changeShape(int)));
    connect(widget.connectionType, SIGNAL(currentIndexChanged(int)), m_tool, SLOT(changeConnectionType(int)));
}

void TreeShapeConfigWidget::updateParameters(TreeShape *tree)
{
    if (!tree) {
        kDebug() << "not tree";
        widget.treeType->setEnabled(false);
        widget.rootType->setEnabled(false);
        widget.connectionType->setEnabled(false);
        return;
    }

    kDebug() << "not tree";
    widget.treeType->blockSignals(true);
    widget.rootType->blockSignals(true);
    widget.connectionType->blockSignals(true);

    widget.treeType->setEnabled(true);
    widget.rootType->setEnabled(true);
    widget.connectionType->setEnabled(true);

    widget.treeType->setCurrentIndex(tree->structure());
    widget.rootType->setCurrentIndex(tree->rootType());
    widget.connectionType->setCurrentIndex(tree->connectionType());

    widget.treeType->blockSignals(false);
    widget.rootType->blockSignals(false);
    widget.connectionType->blockSignals(false);
}

#include <TreeShapeConfigWidget.moc>
