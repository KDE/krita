/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "KoReportDesignerItemLabel.h"
#include <KoReportDesignerItemBase.h>
#include <KoReportDesigner.h>
#include "reportscene.h"

#include <QDomDocument>
#include <QPainter>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <koproperty/EditorView.h>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QTextCursor>
#include <QTextDocument>

//
// class ReportEntityLabel
//

void KoReportDesignerItemLabel::init(QGraphicsScene *scene, KoReportDesigner *d)
{
    if (scene)
        scene->addItem(this);

    KoReportDesignerItemRectBase::init(&m_pos, &m_size, m_set, d);

    connect(propertySet(), SIGNAL(propertyChanged(KoProperty::Set&,KoProperty::Property&)),
            this, SLOT(slotPropertyChanged(KoProperty::Set&,KoProperty::Property&)));

    setZValue(Z);
    setFlag(ItemIsFocusable);
    
    m_inlineEdit = new BoundedTextItem(this);
    m_inlineEdit->setVisible(false);
    m_inlineEdit->setFlag(ItemIsFocusable);
    m_inlineEdit->setFlag(ItemIsSelectable, false);
    QTextDocument *doc = new QTextDocument;
    doc->setDocumentMargin(0);
    doc->setPlainText(text());
    m_inlineEdit->setDocument(doc);
    
    connect(m_inlineEdit, SIGNAL(exitEditMode()), this, SLOT(exitInlineEditingMode()));
}

// methods (constructors)
KoReportDesignerItemLabel::KoReportDesignerItemLabel(KoReportDesigner* d, QGraphicsScene * scene, const QPointF &pos)
        : KoReportDesignerItemRectBase(d)
{
    Q_UNUSED(pos);
    init(scene, d);
    setSceneRect(properRect(*d, getTextRect().width(), getTextRect().height()));
    m_name->setValue(m_reportDesigner->suggestEntityName(typeName()));
    
    enterInlineEditingMode();
}

KoReportDesignerItemLabel::KoReportDesignerItemLabel(QDomNode & element, KoReportDesigner * d, QGraphicsScene * s)
        : KoReportItemLabel(element), KoReportDesignerItemRectBase(d), m_inlineEdit(0)
{
    init(s, d);
    setSceneRect(m_pos.toScene(), m_size.toScene());
}

KoReportDesignerItemLabel* KoReportDesignerItemLabel::clone()
{
    QDomDocument d;
    QDomElement e = d.createElement("clone");
    QDomNode n;
    buildXML(d, e);
    n = e.firstChild();
    return new KoReportDesignerItemLabel(n, designer(), 0);
}

// methods (deconstructor)
KoReportDesignerItemLabel::~KoReportDesignerItemLabel()
{}

QRectF KoReportDesignerItemLabel::getTextRect() const
{
    return QFontMetrics(font()).boundingRect(x(), y(), 0, 0, textFlags(), m_text->value().toString());
}

void KoReportDesignerItemLabel::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (m_inlineEdit->isVisible()) {
        return;
    }
    
    // store any values we plan on changing so we can restore them
    QFont f = painter->font();
    QPen  p = painter->pen();
    
    painter->setFont(font());
    painter->setBackgroundMode(Qt::TransparentMode);

    QColor bg = m_backgroundColor->value().value<QColor>();
    bg.setAlphaF(m_backgroundOpacity->value().toReal() * 0.01);

    painter->setPen(m_foregroundColor->value().value<QColor>());

    painter->fillRect(QGraphicsRectItem::rect(), bg);
    painter->drawText(rect(), textFlags(), text());

    if ((Qt::PenStyle)m_lineStyle->value().toInt() == Qt::NoPen || m_lineWeight->value().toInt() <= 0) {
        painter->setPen(QPen(QColor(224, 224, 224)));
    } else {
        painter->setPen(QPen(m_lineColor->value().value<QColor>(), m_lineWeight->value().toInt(), (Qt::PenStyle)m_lineStyle->value().toInt()));
    }

    painter->drawRect(QGraphicsRectItem::rect());
    painter->setPen(m_foregroundColor->value().value<QColor>());

    drawHandles(painter);

    // restore an values before we started just in case
    painter->setFont(f);
    painter->setPen(p);
}

void KoReportDesignerItemLabel::buildXML(QDomDocument & doc, QDomElement & parent)
{
    //kDebug();
    QDomElement entity = doc.createElement(QLatin1String("report:") + typeName());

    // properties
    addPropertyAsAttribute(&entity, m_name);
    addPropertyAsAttribute(&entity, m_text);
    addPropertyAsAttribute(&entity, m_verticalAlignment);
    addPropertyAsAttribute(&entity, m_horizontalAlignment);
    entity.setAttribute("report:z-index", zValue());

    // bounding rect
    buildXMLRect(doc, entity, &m_pos, &m_size);

    //text style info
    buildXMLTextStyle(doc, entity, textStyle());

    //Line Style
    buildXMLLineStyle(doc, entity, lineStyle());

    parent.appendChild(entity);
}

void KoReportDesignerItemLabel::slotPropertyChanged(KoProperty::Set &s, KoProperty::Property &p)
{
    Q_UNUSED(s);

    if (p.name() == "Name") {
        //For some reason p.oldValue returns an empty string
        if (!m_reportDesigner->isEntityNameUnique(p.value().toString(), this)) {
            p.setValue(m_oldName);
        } else {
            m_oldName = p.value().toString();
        }
    } else if (p.name() == "caption") {
        m_inlineEdit->setPlainText(p.value().toString());
    }

    KoReportDesignerItemRectBase::propertyChanged(s, p);
    if (m_reportDesigner) m_reportDesigner->setModified(true);

}

void KoReportDesignerItemLabel::enterInlineEditingMode()
{
    if (!m_inlineEdit->isVisible()) {
        m_inlineEdit->setVisible(true);
        m_inlineEdit->setPlainText(text());
        m_inlineEdit->setFocus();
        
        QTextCursor c = m_inlineEdit->textCursor();
        c.select(QTextCursor::Document);
        m_inlineEdit->setTextCursor(c);
        
        m_inlineEdit->setFont(m_font->value().value<QFont>());
        m_inlineEdit->setDefaultTextColor(m_foregroundColor->value().value<QColor>());
        m_inlineEdit->setBackgroudColor(m_backgroundColor->value().value<QColor>());
        m_inlineEdit->setBackgroudOpacity(m_backgroundOpacity->value().toInt());
        m_inlineEdit->setForegroundColor(m_foregroundColor->value().value<QColor>());
        m_inlineEdit->setFont(m_font->value().value<QFont>());
        
        update();
    }
}

void KoReportDesignerItemLabel::exitInlineEditingMode()
{
    if (m_inlineEdit->isVisible()) {        
        m_inlineEdit->setVisible(false);
        setText(m_inlineEdit->toPlainText());
    }
}

void KoReportDesignerItemLabel::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    enterInlineEditingMode();
}

void KoReportDesignerItemLabel::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_F2) {
        enterInlineEditingMode();
    } else {
        QGraphicsRectItem::keyReleaseEvent(event);
    }
}
