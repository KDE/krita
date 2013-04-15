/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_shade_selector_line_combo_box.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QDoubleSpinBox>
#include <QLabel>

#include <klocale.h>

#include "kis_shade_selector_line.h"

class LineEditor : public KisShadeSelectorLineBase {
    QDoubleSpinBox* m_hueDelta;
    QDoubleSpinBox* m_saturationDelta;
    QDoubleSpinBox* m_valueDelta;
    QDoubleSpinBox* m_hueShift;
    QDoubleSpinBox* m_saturationShift;
    QDoubleSpinBox* m_valueShift;

public:
    LineEditor(QWidget* parent) : KisShadeSelectorLineBase(parent)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);

        QHBoxLayout* lineOne = new QHBoxLayout();
        layout->addLayout(lineOne);
        lineOne->addWidget(new QLabel(i18n("Delta: ")));

        m_hueDelta = new QDoubleSpinBox();
        lineOne->addWidget(m_hueDelta);
        m_saturationDelta = new QDoubleSpinBox();
        lineOne->addWidget(m_saturationDelta);
        m_valueDelta = new QDoubleSpinBox();
        lineOne->addWidget(m_valueDelta);

        QHBoxLayout* lineTwo = new QHBoxLayout();
        layout->addLayout(lineTwo);
        lineTwo->addWidget(new QLabel(i18n("Shift: ")));

        m_hueShift = new QDoubleSpinBox();
        lineTwo->addWidget(m_hueShift);
        m_saturationShift = new QDoubleSpinBox();
        lineTwo->addWidget(m_saturationShift);
        m_valueShift = new QDoubleSpinBox();
        lineTwo->addWidget(m_valueShift);


        m_hueDelta->setRange(-1, 1);
        m_saturationDelta->setRange(-1, 1);
        m_valueDelta->setRange(-1, 1);
        m_hueShift->setRange(-1, 1);
        m_saturationShift->setRange(-1, 1);
        m_valueShift->setRange(-1, 1);

        m_hueDelta->setSingleStep(0.1);
        m_saturationDelta->setSingleStep(0.1);
        m_valueDelta->setSingleStep(0.1);
        m_hueShift->setSingleStep(0.05);
        m_saturationShift->setSingleStep(0.05);
        m_valueShift->setSingleStep(0.05);
    }

    QString toString() const
    {
        return QString("%1|%2|%3|%4|%5|%6|%7")
                .arg(m_lineNumber)
                .arg(m_hueDelta->value())
                .arg(m_saturationDelta->value())
                .arg(m_valueDelta->value())
                .arg(m_hueShift->value())
                .arg(m_saturationShift->value())
                .arg(m_valueShift->value());
    }

    void fromString(const QString &string)
    {
        QStringList strili = string.split('|');
        m_lineNumber = strili.at(0).toInt();
        m_hueDelta->setValue(strili.at(1).toDouble());
        m_saturationDelta->setValue(strili.at(2).toDouble());
        m_valueDelta->setValue(strili.at(3).toDouble());
        if(strili.size()==4) return;            // don't crash, if reading old config files.
        m_hueShift->setValue(strili.at(4).toDouble());
        m_saturationShift->setValue(strili.at(5).toDouble());
        m_valueShift->setValue(strili.at(6).toDouble());
    }
};

class KisShadeSelectorLineComboBoxPrivate : public QWidget {
public:
    int spacing;
    int selectorWidth;
    QRect highlightArea;
    LineEditor* lineEditor;

    KisShadeSelectorLineComboBoxPrivate(QWidget* parent) :
            QWidget(parent, Qt::Popup),
            spacing(10),
            selectorWidth(100),
            highlightArea(-1,-1,0,0)
    {
        setMouseTracking(true);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setSpacing(spacing);

        layout->addWidget(new KisShadeSelectorLine(1.0, 0.0, 0.0, this));
        layout->addWidget(new KisShadeSelectorLine(0.1, 0.0, 0.0, this));
        layout->addWidget(new KisShadeSelectorLine(0.2, 0.0, 0.0, this));
        layout->addWidget(new KisShadeSelectorLine(0.0, 0.5, 0.0, this));
        layout->addWidget(new KisShadeSelectorLine(0.0, 1.0, 0.0, this));
        layout->addWidget(new KisShadeSelectorLine(0.0, 0.0, 0.5, this));
        layout->addWidget(new KisShadeSelectorLine(0.0, 0.0, 1.0, this));
        layout->addWidget(new KisShadeSelectorLine(0.0, 0.5, 0.5, this));
        layout->addWidget(new KisShadeSelectorLine(0.0, 1.0, 1.0, this));
        layout->addWidget(new KisShadeSelectorLine(0.0, -0.5, 0.5, this));
        layout->addWidget(new KisShadeSelectorLine(0.0, -1.0, 1.0, this));
        layout->addWidget(new KisShadeSelectorLine(0.0, 0.5, 0.5, this, -0.04));
        layout->addWidget(new KisShadeSelectorLine(0.0, 0.5, 0.5, this, +0.04));
        layout->addWidget(new KisShadeSelectorLine(0.0, -0.5, 0.5, this, -0.04));
        layout->addWidget(new KisShadeSelectorLine(0.0, -0.5, 0.5, this, +0.04));

        lineEditor = new LineEditor(this);
        layout->addWidget(lineEditor);

        for(int i=0; i<this->layout()->count(); i++) {
            KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(this->layout()->itemAt(i)->widget());
            if(item!=0) {
//                item->setMaximumWidth(selectorWidth);
//                item->setMinimumWidth(selectorWidth);
                item->setMouseTracking(true);
                item->setEnabled(false);
                item->setColor(QColor(190,50,50));
                item->showHelpText();
            }
        }
    }

protected:
    void paintEvent(QPaintEvent *)
    {
        QPainter painter(this);
        painter.fillRect(0,0,width(), height(), QColor(128,128,128));
        painter.fillRect(highlightArea, palette().highlight());
    }

    void mouseMoveEvent(QMouseEvent * e)
    {
        if(rect().contains(e->pos())) {
            for(int i=0; i<layout()->count(); i++) {
                KisShadeSelectorLineBase* item = dynamic_cast<KisShadeSelectorLineBase*>(layout()->itemAt(i)->widget());

                Q_ASSERT(item);
                if(item && item->geometry().adjusted(-spacing/2, -spacing/2, spacing/2, spacing/2).contains(e->pos())) {
                    QRect oldArea=highlightArea;
                    highlightArea=item->geometry().adjusted(-spacing/2, -spacing/2, spacing/2, spacing/2);
                    m_lastActiveConfiguration=item->toString();
                    update(highlightArea);
                    update(oldArea);
                }
            }
        }
        else {
            highlightArea.setRect(-1,-1,0,0);
        }
    }

    void mousePressEvent(QMouseEvent* e)
    {
        if(rect().contains(e->pos())) {
            mouseMoveEvent(e);
            KisShadeSelectorLineComboBox* parent = dynamic_cast<KisShadeSelectorLineComboBox*>(this->parent());
            Q_ASSERT(parent);
            parent->setConfiguration(m_lastActiveConfiguration);
        }
        hide();
        e->accept();
    }
    QString m_lastActiveConfiguration;
};

KisShadeSelectorLineComboBox::KisShadeSelectorLineComboBox(QWidget *parent) :
    QComboBox(parent),
    m_private(new KisShadeSelectorLineComboBoxPrivate(this)),
    m_currentLine(new KisShadeSelectorLine(0,0,0,this))
{
    QGridLayout* l = new QGridLayout(this);
    l->addWidget(m_currentLine);

    m_currentLine->setEnabled(false);
    m_currentLine->setColor(QColor(190,50,50));

    // 30 pixels for the arrow of the combobox
//    setMinimumWidth(m_private->selectorWidth+m_private->spacing+30);
//    setMaximumWidth(minimumWidth());
//    m_currentLine->setMaximumWidth(m_private->selectorWidth);

//    setMaximumHeight(QWIDGETSIZE_MAX);

    updateSettings();
}

void KisShadeSelectorLineComboBox::hidePopup()
{
    QComboBox::hidePopup();
    m_private->hide();
}

void KisShadeSelectorLineComboBox::showPopup()
{
    // only show if this is not the popup
    QComboBox::showPopup();
    m_private->move(mapToGlobal(QPoint(0,-300)));
    m_private->show();
}

void KisShadeSelectorLineComboBox::setConfiguration(const QString &stri)
{
    m_currentLine->fromString(stri);
    m_private->lineEditor->fromString(stri);
}

QString KisShadeSelectorLineComboBox::configuration() const
{
    return m_currentLine->toString();
}

void KisShadeSelectorLineComboBox::setLineNumber(int n)
{
    m_currentLine->setLineNumber(n);
    for(int i=0; i<m_private->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(m_private->layout()->itemAt(i)->widget());
        if(item!=0) {
            item->setLineNumber(n);
        }
    }
}

//QSize KisShadeSelectorLineComboBox::sizeHint() const
//{
//    return minimumSize();
//}

void KisShadeSelectorLineComboBox::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
    m_currentLine->setMaximumWidth(width()-30-m_private->spacing);
    m_private->setMinimumWidth(qMax(280, width()));
    m_private->setMaximumWidth(qMax(280, width()));
}

void KisShadeSelectorLineComboBox::updateSettings()
{
    m_currentLine->updateSettings();
    for(int i=0; i<m_private->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(m_private->layout()->itemAt(i)->widget());
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
    for(int i=0; i<m_private->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(m_private->layout()->itemAt(i)->widget());
        if(item!=0) {
            item->m_gradient=b;
        }
    }

    update();
}

void KisShadeSelectorLineComboBox::setPatches(bool b)
{
    m_currentLine->m_gradient=!b;
    for(int i=0; i<m_private->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(m_private->layout()->itemAt(i)->widget());
        if(item!=0) {
            item->m_gradient=!b;
        }
    }

    update();
}

void KisShadeSelectorLineComboBox::setPatchCount(int count)
{
    m_currentLine->m_patchCount=count;
    for(int i=0; i<m_private->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(m_private->layout()->itemAt(i)->widget());
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
    setMinimumHeight(height+m_private->spacing);
    setMaximumHeight(height+m_private->spacing);

    update();
}
