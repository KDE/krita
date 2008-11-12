/*
 * KoShapeEndLines.cpp
 *
 *  Created on: 10 nov. 2008
 *      Author: alexia
 */

#include "KoShapeEndLines.h"
#include <QGridLayout>

#include <klocale.h>

KoShapeEndLines::KoShapeEndLines(QWidget* parent): QDockWidget(parent){
	// TODO Auto-generated constructor stub
	setWindowTitle(i18n("End lines"));

	    QWidget* mainWidget = new QWidget(this);
	    QGridLayout* mainLayout = new QGridLayout(mainWidget);
	    mainLayout->setMargin(0);
	    setWidget(mainWidget);

	    m_quickView = new QListView (mainWidget);
	    mainLayout->addWidget(m_quickView, 0, 0);
	    m_quickView->setViewMode(QListView::IconMode);
	    m_quickView->setDragDropMode(QListView::DragOnly);
	    m_quickView->setSelectionMode(QListView::SingleSelection);
	    m_quickView->setResizeMode(QListView::Adjust);
	    m_quickView->setFlow(QListView::LeftToRight);
	    m_quickView->setGridSize(QSize(40, 44));
	    m_quickView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	    m_quickView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	    m_quickView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	    m_quickView->setTextElideMode(Qt::ElideNone);
	    m_quickView->setWordWrap(true);
}

KoShapeEndLines::~KoShapeEndLines() {
	// TODO Auto-generated destructor stub
}
