/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004  Alexander Dymo <cloudtemple@mskat.net>

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

#include "sizepolicyedit.h"
#include "editoritem.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qsizepolicy.h>
#include <qmap.h>
#include <qtooltip.h>

#ifdef QT_ONLY
// \todo
#else
#include <klocale.h>
#endif

using namespace KoProperty;

QMap<QString, QVariant> *SizePolicyEdit::m_spValues = 0;

SizePolicyEdit::SizePolicyEdit(Property *property, QWidget *parent, const char *name)
 : Widget(property, parent, name)
{
	setHasBorders(false);
//	QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
	m_edit = new QLabel(this);
	m_edit->setIndent(KPROPEDITOR_ITEM_MARGIN);
	m_edit->setBackgroundMode(Qt::PaletteBase);
//	m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_edit->setMinimumHeight(5);
	setEditor(m_edit);
//	l->addWidget(m_edit);
	setFocusWidget(m_edit);


	if(!m_spValues) {
		m_spValues = new QMap<QString, QVariant>();
		(*m_spValues)[i18nc("Size Policy", "Fixed")] = QSizePolicy::Fixed;
		(*m_spValues)[i18nc("Size Policy", "Minimum")] = QSizePolicy::Minimum;
		(*m_spValues)[i18nc("Size Policy", "Maximum")] = QSizePolicy::Maximum;
		(*m_spValues)[i18nc("Size Policy", "Preferred")] = QSizePolicy::Preferred;
		(*m_spValues)[i18nc("Size Policy", "Expanding")] = QSizePolicy::Expanding;
		(*m_spValues)[i18nc("Size Policy", "Minimum Expanding")] = QSizePolicy::MinimumExpanding;
		(*m_spValues)[i18nc("Size Policy", "Ignored")] = QSizePolicy::Ignored;
	}
}

SizePolicyEdit::~SizePolicyEdit()
{
	delete m_spValues;
	m_spValues = 0;
}

QVariant
SizePolicyEdit::value() const
{
	return m_value;
}

void
SizePolicyEdit::setValue(const QVariant &value, bool emitChange)
{
	m_value = value;
	m_edit->setText(QString("%1/%2/%3/%4").arg(findDescription(value.value<QSizePolicy>().horData())).
		arg(findDescription(value.value<QSizePolicy>().verData())).
		arg(value.value<QSizePolicy>().horStretch()).arg(value.value<QSizePolicy>().verStretch()));
	this->setToolTip( m_edit->text());

	if (emitChange)
		emit valueChanged(this);
}

void
SizePolicyEdit::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
//	p->eraseRect(r);
//	p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
	QRect rect(r);
	rect.setBottom(r.bottom()+1);
	Widget::drawViewer(p, cg, rect, 
		QString("%1/%2/%3/%4").arg(findDescription(value.value<QSizePolicy>().horData())).
		arg(findDescription(value.value<QSizePolicy>().verData())).
		arg(value.value<QSizePolicy>().horStretch()).arg(value.value<QSizePolicy>().verStretch()));
}

QString
SizePolicyEdit::findDescription(const QVariant &value) const
{
	if(!m_spValues)
		return QString::null;

	QMap<QString, QVariant>::ConstIterator endIt = m_spValues->constEnd();
	for (QMap<QString, QVariant>::ConstIterator it = m_spValues->constBegin(); it != endIt; ++ it) {
		if (it.data() == value)
			return it.key();
	}
	return QString::null;;
}

void
SizePolicyEdit::setReadOnlyInternal(bool readOnly)
{
	Q_UNUSED(readOnly);
}

#include "sizepolicyedit.moc"
