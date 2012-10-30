/* This file is part of the KDE project
 * Copyright (C) 2011 Steven Kakoczky <steven.kakoczky@gmail.com>
 * Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#include "KoAnnotationBalloon.h"
#include <QDateTime>

KoAnnotationBalloon::KoAnnotationBalloon(int position, KoAnnotation *data, QWidget *parent):
		KoBalloon(position, parent),
		d(data)
{
	QGridLayout *layout = new QGridLayout(this);

    m_authorLabel = new QLabel(d->creator());
    m_dateLabel = new QLabel(d->date());//d->date().toString("MM/dd/yyyy hh:mm"));
    m_textContent = new QTextEdit("d->text()", this);

	QFont font = m_authorLabel->font();
	font.setPointSize(8);
	m_authorLabel->setFont(font);
	m_dateLabel->setFont(font);

	layout->addWidget(m_textContent, 0, 0, 1, 2);
	layout->addWidget(m_authorLabel, 1, 0);
	layout->addWidget(m_dateLabel, 2, 0);

    m_optionButton = new QPushButton(this);
    m_options = new QMenu(this);
    m_options->addAction("Delete");
    m_optionButton->setMenu(m_options);

	layout->addWidget(m_optionButton, 1, 1, 2, 1, Qt::AlignCenter);

	setLayout(layout);
	QPalette pal = palette();
	pal.setColor(QPalette::Window, Qt::green);
	setPalette(pal);
	setAutoFillBackground(true);
}

