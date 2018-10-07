/*
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

#ifndef H_KIS_WIDGET_CHOOSER_H_
#define H_KIS_WIDGET_CHOOSER_H_

#include <kritaui_export.h>
#include <QList>
#include <QIcon>
#include <QFrame>

class QToolButton;
class QLabel;
class QButtonGroup;

class KRITAUI_EXPORT KisWidgetChooser: public QFrame
{
    Q_OBJECT
    
    struct Data
    {
        Data(const QString& ID):
            id(ID), widget(0), label(0), chosen(false) { }
        Data(const Data& d):
            id(d.id), widget(d.widget), label(d.label), chosen(d.chosen) { }
        Data(const QString& ID, QWidget* w, QLabel* l):
            id(ID), widget(w), label(l), chosen(false) { }
            
        friend bool operator == (const Data& a, const Data& b) {
            return a.id == b.id;
        }
        
        QString  id;
        QWidget* widget;
        QLabel*  label;
        bool     chosen;
    };
    
    typedef QList<Data>::iterator       Iterator;
    typedef QList<Data>::const_iterator ConstIterator;
    
public:
     KisWidgetChooser(int id, QWidget* parent=0);
    ~KisWidgetChooser() override;
    
    QWidget* chooseWidget(const QString& id);
    void     addWidget(const QString& id, const QString& label, QWidget* widget);
    QWidget* getWidget(const QString& id) const;
    
    template<class TWidget>
    TWidget* addWidget(const QString& id, const QString& label = "") {
        TWidget* widget = new TWidget();
        addWidget(id, label, widget);
        return widget;
    }
    
    template<class TWidget>
    TWidget* getWidget(const QString& id) const {
        return dynamic_cast<TWidget*>(getWidget(id));
    }
public Q_SLOTS:

    void showPopupWidget();
    void updateThemedIcons();
    
private:
    void     removeWidget(const QString& id);
    QLayout* createPopupLayout();
    QLayout* createLayout();
    void     updateArrowIcon();
    
protected Q_SLOTS:
    void slotButtonPressed();
    void slotWidgetChosen(int index);

    // QWidget interface
protected:
    void changeEvent(QEvent *e) override;

private:
    int           m_chooserid;
    QIcon         m_acceptIcon;
    QToolButton*  m_arrowButton;
    QButtonGroup* m_buttons;
    QFrame*       m_popup;
    QString       m_chosenID;
    QList<Data>   m_widgets;
};

#endif // H_KIS_WIDGET_CHOOSER_H_

