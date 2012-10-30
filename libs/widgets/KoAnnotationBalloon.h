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

#ifndef KOANNOTATIONBALLOON_H
#define KOANNOTATIONBALLOON_H

#include <KoBalloon.h>
#include <KoInlineNote.h>
#include <QLabel>
#include <QTextEdit>
#include <QMenu>
#include <QPushButton>
#include <QTextFrame>
#include "kowidgets_export.h"
#include "KoAnnotation.h"

#include <QGridLayout>

class KOWIDGETS_EXPORT KoAnnotationBalloon : public KoBalloon
{
public:
	explicit KoAnnotationBalloon(int position, KoAnnotation *data, QWidget *parent = 0);

public slots:

private:
	KoAnnotation *d;
	QTextEdit *m_textContent;
	QLabel *m_authorLabel;
	QLabel *m_dateLabel;

    QPushButton *m_optionButton;
    QMenu *m_options;
};

#endif // KOANNOTATIONBALLOON_H
