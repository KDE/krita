/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_shade_selector_line_combo_box.h"

#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>

#include <QGridLayout>
#include <QPainter>

#include <klocalizedstring.h>

#include "kis_shade_selector_line.h"
#include "kis_shade_selector_line_combo_box_popup.h"
#include "kis_color_selector_base_proxy.h"

#include "kis_global.h"


KisShadeSelectorLineComboBox::KisShadeSelectorLineComboBox(QWidget *parent) :
    QComboBox(parent),
    m_popup(new KisShadeSelectorLineComboBoxPopup(this)),
    m_parentProxy(new KisColorSelectorBaseProxyNoop()),
    m_currentLine(new KisShadeSelectorLine(0,0,0, m_parentProxy.data(), this))
{
    QGridLayout* l = new QGridLayout(this);
    l->addWidget(m_currentLine);

    m_currentLine->setAttribute(Qt::WA_TransparentForMouseEvents);

    KoColor color;
    color.fromQColor(QColor(190, 50, 50));
    m_currentLine->setColor(color);

    updateSettings();
}

KisShadeSelectorLineComboBox::~KisShadeSelectorLineComboBox()
{
}

void KisShadeSelectorLineComboBox::hidePopup()
{
    QComboBox::hidePopup();
    m_popup->hide();
}

void KisShadeSelectorLineComboBox::showPopup()
{
    QComboBox::showPopup();
    m_popup->show();

    const int widgetMargin = 20;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    QRect geom = this->geometry();
    QPoint p(geom.width() / 2 + geom.left(), geom.height() / 2 + geom.top());
    QScreen *screen = qApp->screenAt(p);
    QRect fitRect;
    if (screen) {
       fitRect = kisGrowRect(screen->availableGeometry(), -widgetMargin);
    }
    else {
        fitRect = kisGrowRect(QRect(0, 0, 1024, 768), -widgetMargin);
    }
#else
    QRect fitRect = kisGrowRect(QApplication::desktop()->screenGeometry(), -widgetMargin);
#endif
    QRect popupRect = m_popup->rect();
    popupRect.moveTo(mapToGlobal(QPoint()));
    popupRect = kisEnsureInRect(popupRect, fitRect);

    m_popup->move(popupRect.topLeft());
    m_popup->setConfiguration(m_currentLine->toString());
}

void KisShadeSelectorLineComboBox::setConfiguration(const QString &stri)
{
    m_currentLine->fromString(stri);
    update();
}

QString KisShadeSelectorLineComboBox::configuration() const
{
    return m_currentLine->toString();
}

void KisShadeSelectorLineComboBox::setLineNumber(int n)
{
    m_currentLine->setLineNumber(n);
    for(int i=0; i<m_popup->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(m_popup->layout()->itemAt(i)->widget());
        if(item!=0) {
            item->setLineNumber(n);
        }
    }
}

void KisShadeSelectorLineComboBox::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
    m_currentLine->setMaximumWidth(width()-30-m_popup->spacing);
    m_popup->setMinimumWidth(qMax(280, width()));
    m_popup->setMaximumWidth(qMax(280, width()));
}

void KisShadeSelectorLineComboBox::updateSettings()
{
    m_currentLine->updateSettings();
    for(int i=0; i<m_popup->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(m_popup->layout()->itemAt(i)->widget());
        if(item!=0) {
            item->updateSettings();
            item->m_lineHeight=30;
            item->setMaximumHeight(30);
            item->setMinimumHeight(30);
        }
    }

    setLineHeight(m_currentLine->m_lineHeight);
}


void KisShadeSelectorLineComboBox::setGradient(bool b)
{
    m_currentLine->m_gradient=b;
    for(int i=0; i<m_popup->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(m_popup->layout()->itemAt(i)->widget());
        if(item!=0) {
            item->m_gradient=b;
        }
    }

    update();
}

void KisShadeSelectorLineComboBox::setPatches(bool b)
{
    m_currentLine->m_gradient=!b;
    for(int i=0; i<m_popup->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(m_popup->layout()->itemAt(i)->widget());
        if(item!=0) {
            item->m_gradient=!b;
        }
    }

    update();
}

void KisShadeSelectorLineComboBox::setPatchCount(int count)
{
    m_currentLine->m_patchCount=count;
    for(int i=0; i<m_popup->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(m_popup->layout()->itemAt(i)->widget());
        if(item!=0) {
            item->m_patchCount=count;
        }
    }

    update();
}

void KisShadeSelectorLineComboBox::setLineHeight(int height)
{
    m_currentLine->m_lineHeight=height;
    m_currentLine->setMinimumHeight(height);
    m_currentLine->setMaximumHeight(height);
    setMinimumHeight(height+m_popup->spacing);
    setMaximumHeight(height+m_popup->spacing);

    update();
}
