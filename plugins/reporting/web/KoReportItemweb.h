/*
 * Kexi Report Plugin
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
#ifndef WEBBROWSER
#define WEBBROWSER
 #include <QGraphicsView>
#include <KoReportItemBase.h>
 #include <QWebFrame>
#include <QRect>
#include <QPainter>
#include <qdom.h>
#include "krpos.h"
#include "krsize.h"
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <KoGlobal.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <kglobalsettings.h>
 #include <QGraphicsScene>
//the widget class is specified here 
/*
#include <QtGui/QWidget>
#include <QtGui/QPushButton>
//#include "widgetfactory.h"	
//#include "container.h"
//#include <formeditor/FormWidgetInterface.h>
//#include <plugins/forms/kexiformdataiteminterface.h>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtCore/QUrl>
#include <QGraphicsWebView>
class QWebView;
//class QLineEdit;
class QVBoxLayout;
class QLabel;
class QAction;
class QWebHistory;
//class ToolBar; //added
class QHBoxLayout;
class QLabel;
class QUrl;
class QGraphicsWebView;
class QWebframe;
class QGraphicsItem;
class WebBrowserWidget :  public QWidget
{
	
    Q_OBJECT
//    Q_PROPERTY(QString dataSource READ dataSource WRITE setDataSource)
//    Q_PROPERTY(QString dataSourcePartClass READ dataSourcePartClass WRITE setDataSourcePartClass)
    Q_PROPERTY(QString url READ url WRITE setUrl)

    
public:
    WebBrowserWidget();    
    ~WebBrowserWidget();
    WebBrowserWidget(QWidget *parent=0);
    
    inline QString url() const {
	
	return m_url.toString();
    }

    virtual QVariant value();
    virtual void setInvalidState(const QString& displayText);
    virtual bool valueIsNull();
    virtual bool valueIsEmpty();
    virtual bool cursorAtStart();
    virtual bool cursorAtEnd();
    virtual void clear();
    bool isReadOnly() const ;
    virtual void setReadOnly(bool readOnly);  
    void paintweb();

//    void setDataSource(const QString &ds);
//    void setDataSourcePartClass(const QString &ds);
    
 //   void loadPreviousPage();
 //   void loadNextPage(); 
 //   void onreload(); 

protected:
//    virtual void setValueInternal(const QVariant& add, bool removeOld); 
  //  void updateUrl();
    bool m_readOnly;
    
friend class KoReportItemweb;

private:
//    QAction* m_softkeyAction;

    
//    QLineEdit* m_lineEdit;
    QLabel *m_label;
    QVBoxLayout* v_layout;
//    ToolBar* m_toolbar;
    bool m_urlChanged_enabled;   
    QWebHistory* m_history;
   
};

namespace Scripting
{
class web;
}

/**
 @author
*/
class KoReportItemweb : public KoReportItemBase
{     Q_OBJECT 
public:
    KoReportItemweb() {
    //    createProperties();
    }
    KoReportItemweb(QDomNode & element);
    ~KoReportItemweb();
    virtual QString typeName() const;

    virtual int render(OROPage* page, OROSection* section,  QPointF offset, QVariant data, KRScriptHandler *script);
    using KoReportItemBase::render;

public slots:
    void setUrl(const QString& url);

protected:
    KoProperty::Property* my_url;
    KoProperty::Property * m_controlSource;
    QUrl m_url;
    QGraphicsWebView* m_view;
    virtual void createProperties();
  
/*private:
    virtual void createProperties();
*/
    friend class Scripting::web;
    
};


#endif

