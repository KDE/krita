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
#ifndef KRIMAGEDATA_H
#define KRIMAGEDATA_H
#include <KoReportItemBase.h>
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

//the widget class is specified here 
#include <QtGui/QWidget>
#include <QtGui/QPushButton>
#include "widgetfactory.h"	//these already inherit Qt headers reqd
#include "container.h"
//#include <FormWidgetInterface.h>
//#include <plugins/forms/kexiformdataiteminterface.h>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtCore/QUrl>
class QWebView;
class QLineEdit;
class QVBoxLayout;
class QLabel;
class QAction;
class QWebHistory;
class ToolBar; //added
class QHBoxLayout;
class QLabel;
class QUrl;
 QUrl m_url;
class KEXIFORMUTILS_EXPORT WebBrowserWidget :  public QWidget, 
					       public KexiFormDataItemInterface,
					       public KFormDesigner::FormWidgetInterface
{
    Q_OBJECT
    Q_PROPERTY(QString dataSource READ dataSource WRITE setDataSource)
    Q_PROPERTY(QString dataSourcePartClass READ dataSourcePartClass WRITE setDataSourcePartClass)
    Q_PROPERTY(QUrl url READ url WRITE seturl)

    
public:
	WebBrowserWidget();    
	~WebBrowserWidget();
	WebBrowserWidget(QWidget *parent=0);
void setValueInternal(const QVariant&, bool b){}
void setInvalidState(const QString& q){}
void setReadOnly(bool b1){}
bool valueIsNull();
bool valueIsEmpty();
bool cursorAtStart();
bool cursorAtEnd();
void clear();
QVariant value();

    inline QString dataSource() const {
        return KexiFormDataItemInterface::dataSource();
    }
    inline QString dataSourcePartClass() const {
        return KexiFormDataItemInterface::dataSourcePartClass();
    }

inline QUrl url() const {
	
	return m_url;
    }


public slots:
void setDataSource(const QString &ds);

void setDataSourcePartClass(const QString &partClass);

void seturl(QUrl m_url);


//void openUrl();
 void onLoadFinished(bool finished);
 void loadPreviousPage();
 void  loadNextPage(); 
 void onreload(); 
  void openUrl(); 

  private:
  QAction* m_softkeyAction;
    QWebView* m_view;
    QLineEdit* m_lineEdit;
    QLabel *m_label;
    QVBoxLayout* v_layout;
    ToolBar* m_toolbar;
    QWebHistory* m_history;
  
};

class ToolBar : public QWidget
{
    Q_OBJECT

public:
    ToolBar(QWidget *parent = 0);
signals:
   void goBack();
   void goForward();
   void  doreload();
    
private slots:
    void onBackPressed(); 
    void onForward();
    
    void onReload();
  
   //void loadPreviousPage();
    
private:

    QPushButton* m_backButton;
    QPushButton* m_forward;
    QHBoxLayout* m_layout;
    QPushButton* m_reload;  
};

namespace Scripting
{
class web;
}

/**
 @author
*/
class KoReportItemweb : public KoReportItemBase
{
public:
    KoReportItemweb() {
        createProperties();
    }
    KoReportItemweb(QDomNode & element);
    ~KoReportItemweb();

    virtual QString typeName() const;
    virtual int render(OROPage* page, OROSection* section,  QPointF offset, QVariant data, KRScriptHandler *script);
    using KoReportItemBase::render;

    virtual QString itemDataSource() const;

    inline QString url() const {
	
	return m_url.toString();
    }
    void setDataSource(const QString &ds);



protected:
    KoProperty::Property* m_url;
    void setUrl(const QString& url);
  KoProperty::Property * m_dataSource;
WebBrowserWidget* web;
    /*void setMode(const QString&);
    void setInlineImageData(QByteArray, const QString& = QString());
    void setColumn(const QString&);
    QString mode() const;
    bool isInline() const;
    QByteArray inlineImageData() const;
    */
private:
    virtual void createProperties();

    friend class Scripting::web;
};


#endif

