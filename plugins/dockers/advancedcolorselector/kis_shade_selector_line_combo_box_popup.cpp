/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_shade_selector_line_combo_box.h"
#include "kis_shade_selector_line_combo_box_popup.h"

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPainter>

#include "kis_global.h"
#include "kis_shade_selector_line.h"
#include "kis_shade_selector_line_editor.h"
#include "kis_color_selector_base_proxy.h"


KisShadeSelectorLineComboBoxPopup::KisShadeSelectorLineComboBoxPopup(QWidget* parent)
    : QWidget(parent, Qt::Popup),
      spacing(10),
      m_lastHighlightedItem(0),
      m_lastSelectedItem(0),
      m_lineEditor(0),
      m_parentProxy(new KisColorSelectorBaseProxyNoop())
{
    setMouseTracking(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(spacing);

    layout->addWidget(new KisShadeSelectorLine(1.0, 0.0, 0.0, m_parentProxy.data(), this));
    layout->addWidget(new KisShadeSelectorLine(0.1, 0.0, 0.0, m_parentProxy.data(), this));
    layout->addWidget(new KisShadeSelectorLine(0.2, 0.0, 0.0, m_parentProxy.data(), this));
    layout->addWidget(new KisShadeSelectorLine(0.0, 0.5, 0.0, m_parentProxy.data(), this));
    layout->addWidget(new KisShadeSelectorLine(0.0, 1.0, 0.0, m_parentProxy.data(), this));
    layout->addWidget(new KisShadeSelectorLine(0.0, 0.0, 0.5, m_parentProxy.data(), this));
    layout->addWidget(new KisShadeSelectorLine(0.0, 0.0, 1.0, m_parentProxy.data(), this));
    layout->addWidget(new KisShadeSelectorLine(0.0, 0.5, 0.5, m_parentProxy.data(), this));
    layout->addWidget(new KisShadeSelectorLine(0.0, 1.0, 1.0, m_parentProxy.data(), this));
    layout->addWidget(new KisShadeSelectorLine(0.0, -0.5, 0.5, m_parentProxy.data(), this));
    layout->addWidget(new KisShadeSelectorLine(0.0, -1.0, 1.0, m_parentProxy.data(), this));
    layout->addWidget(new KisShadeSelectorLine(0.0, 0.5, 0.5, m_parentProxy.data(), this, -0.04));
    layout->addWidget(new KisShadeSelectorLine(0.0, 0.5, 0.5, m_parentProxy.data(), this, +0.04));
    layout->addWidget(new KisShadeSelectorLine(0.0, -0.5, 0.5, m_parentProxy.data(), this, -0.04));

    KisShadeSelectorLine* preview = new KisShadeSelectorLine(0.0, -0.5, 0.5, m_parentProxy.data(), this, +0.04);
    m_lineEditor = new KisShadeSelectorLineEditor(this, preview);
    layout->addWidget(preview);
    layout->addWidget(m_lineEditor);

    connect(m_lineEditor, SIGNAL(requestActivateLine(QWidget*)), SLOT(activateItem(QWidget*)));

    for(int i=0; i<this->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(this->layout()->itemAt(i)->widget());
        if(item!=0) {
            item->setMouseTracking(true);
            item->setEnabled(false);
            KoColor color;
            color.fromQColor(QColor(190, 50, 50));
            item->setColor(color);
            item->showHelpText();
        }
    }
}

KisShadeSelectorLineComboBoxPopup::~KisShadeSelectorLineComboBoxPopup()
{
}

void KisShadeSelectorLineComboBoxPopup::setConfiguration(const QString &string)
{
    m_lineEditor->fromString(string);
}

void KisShadeSelectorLineComboBoxPopup::updateSelectedArea(const QRect &newRect)
{
    QRect oldSelectedArea = m_selectedArea;
    m_selectedArea = newRect;
    update(oldSelectedArea);
    update(m_selectedArea);
}

void KisShadeSelectorLineComboBoxPopup::updateHighlightedArea(const QRect &newRect)
{
    QRect oldHighlightArea = m_highlightedArea;
    m_highlightedArea = newRect;
    update(oldHighlightArea);
    update(m_highlightedArea);
}

void KisShadeSelectorLineComboBoxPopup::activateItem(QWidget *widget)
{
    KisShadeSelectorLineBase* item = dynamic_cast<KisShadeSelectorLineBase*>(widget);
    KIS_ASSERT_RECOVER_RETURN(item);

    QRect itemRect = kisGrowRect(item->geometry(), spacing / 2 - 1);
    m_lastSelectedItem = item;

    updateSelectedArea(itemRect);
}

void KisShadeSelectorLineComboBoxPopup::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(0,0,width(), height(), QColor(128,128,128));
    painter.fillRect(m_selectedArea, palette().highlight());

    painter.setPen(QPen(palette().highlight(), 2));
    painter.drawRect(m_highlightedArea);
}

void KisShadeSelectorLineComboBoxPopup::mouseMoveEvent(QMouseEvent * e)
{
    if(rect().contains(e->pos())) {
        for(int i = 0; i < layout()->count(); i++) {
            KisShadeSelectorLineBase* item = dynamic_cast<KisShadeSelectorLineBase*>(layout()->itemAt(i)->widget());
            KIS_ASSERT_RECOVER_RETURN(item);

            QRect itemRect = kisGrowRect(item->geometry(), spacing / 2 - 1);
            if(itemRect.contains(e->pos())) {
                m_lastHighlightedItem = item;

                updateHighlightedArea(itemRect);
            }
        }
    }
    else {
        updateHighlightedArea(QRect());
    }
}

void KisShadeSelectorLineComboBoxPopup::mousePressEvent(QMouseEvent* e)
{
    if(rect().contains(e->pos())) {
        mouseMoveEvent(e);

        m_lastSelectedItem = m_lastHighlightedItem;

        if (m_lastSelectedItem != m_lineEditor) {
            m_lineEditor->blockSignals(true);
            m_lineEditor->fromString(m_lastSelectedItem->toString());
            m_lineEditor->blockSignals(false);
        }
        updateSelectedArea(m_highlightedArea);
    }
    if (m_lastSelectedItem) {
        KisShadeSelectorLineComboBox *parent = dynamic_cast<KisShadeSelectorLineComboBox*>(this->parent());
        Q_ASSERT(parent);
        parent->setConfiguration(m_lastSelectedItem->toString());
    }
    e->accept();

    this->parentWidget()->update();
    hide();
}

