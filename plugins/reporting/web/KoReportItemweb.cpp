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
#include "KoReportItemweb.h"
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <KoGlobal.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <kglobalsettings.h>
#include <QBuffer>
#include <kcodecs.h>
 #include <QGraphicsRectItem>
#include <renderobjects.h>

#include <QtWebKit>
#include <QtWebKit/QWebHistory>
#include <QWebView>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtCore/QUrl>
#include <QtGui/QAction>
#include <QTextBrowser>
#include <QWebPage>
#include <QtGui/QWidget>
#include <QtGui/QApplication>

/*WebBrowserWidget::WebBrowserWidget(QWidget *parent)
        : QWidget(parent)
	,m_readOnly(false)
	,m_urlChanged_enabled(false)
{
    setFocusPolicy(Qt::StrongFocus);
    setMinimumHeight(sizeHint().height());
    setMinimumWidth(minimumHeight());

    m_view = new QGraphicsWebView();
    m_view->load(QUrl("http://www.kde.org"));
//    v_layout = new QVBoxLayout();
//    v_layout->addWidget(m_view);

    setLayout(v_layout);
    v_layout->addStretch();


};

WebBrowserWidget::~WebBrowserWidget()
{

}


WebBrowserWidget::WebBrowserWidget()  
{
}



bool WebBrowserWidget::cursorAtStart()
{
    return true; //! \todo ?
}

bool WebBrowserWidget::cursorAtEnd()
{
    return true; //! \todo ?
}


QVariant WebBrowserWidget::value()
{
    if (dataSource().isEmpty()) {
        //not db-aware
        return QVariant();
    }
    //db-aware mode
    
    return m_url;


}

bool WebBrowserWidget::valueIsNull()
{
    return (m_url).isEmpty();

}
void WebBrowserWidget::clear()
{
    setUrl("www.google.com");
}



void WebBrowserWidget::setInvalidState(const QString& displayText)
{
    Q_UNUSED(displayText);

//    if (!dataSource().isEmpty()) {
//        m_url.clear();
//    }
    setReadOnly(true);
}

void WebBrowserWidget::setValueInternal(const QVariant &add, bool removeOld)
{
    Q_UNUSED(add); //compares  
    Q_UNUSED(removeOld);

    if (isReadOnly())
        return;
    m_urlChanged_enabled= false;		//if removeold is true then change the Url to value of add as specified in kexidataitem interface.cpp

    if (removeOld)
        { 			//set property editor to add
	 setUrl(add.toString()); 
	}       

    m_urlChanged_enabled = true;
}

bool WebBrowserWidget::valueIsEmpty()
{
    return false;
}


bool WebBrowserWidget::isReadOnly() const
{
    return m_readOnly;
}


void  WebBrowserWidget::setReadOnly(bool val)
{
    m_readOnly=val;
}*/
KoReportItemweb::KoReportItemweb(QDomNode & element)
{
//    createProperties();
    QDomNodeList nl = element.childNodes();
    QString n;
    QDomNode node;
 QDomElement e = element.toElement();
    my_url->setValue(e.attribute("report:address"));

    m_name->setValue(element.toElement().attribute("report:name"));
 
    parseReportRect(element.toElement(), &m_pos, &m_size);
}

void KoReportItemweb::createProperties()
{
    m_set = new KoProperty::Set(0, "Web");
    my_url = new KoProperty::Property("item-web-url", QStringList(), QStringList(), QString(), i18n("url"));
    m_controlSource = new KoProperty::Property("item-data-source", QStringList(), QStringList(), QString(), i18n("Data Source"));
    m_set->addProperty(m_controlSource);
    //QStringList keys, strings;
    //keys << "clip" << "stretch";
    //strings << i18n("Clip") << i18n("Stretch");
    //m_resizeMode = new KoProperty::Property("resize-mode", keys, strings, "clip", i18n("Resize Mode"));

    //m_staticImage = new KoProperty::Property("static-image", QPixmap(), i18n("Static Image"));

    addDefaultProperties();
    m_set->addProperty(m_controlSource);
    m_set->addProperty(my_url);
    //m_set->addProperty(m_staticImage);
}

KoReportItemweb::~KoReportItemweb()
{
    delete m_set;
}
QString KoReportItemweb::typeName() const
{
    return "web";
}

/*bool KoReportItemweb::isInline() const
{
    return !(inlineImageData().isEmpty());
}*/

/*QByteArray KoReportItemweb::inlineImageData() const
{
    QPixmap pixmap = m_staticImage->value().value<QPixmap>();
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::ReadWrite);
    pixmap.save(&buffer, "PNG");   // writes pixmap into ba in PNG format,
    //TODO should i remember the format used, or save as PNG as its lossless?

    QByteArray imageEncoded(KCodecs::base64Encode(buffer.buffer(), true));
    return imageEncoded;
}

void KoReportItemweb::setInlineImageData(QByteArray dat, const QString &fn)
{
    if (!fn.isEmpty()) {
        QPixmap pix(fn);
        if (!pix.isNull())
            m_staticImage->setValue(pix);
        else {
            QPixmap blank(1, 1);
            blank.fill();
            m_staticImage->setValue(blank);
        }
    } else {
        const QByteArray binaryStream(KCodecs::base64Decode(dat));
        const QPixmap pix(QPixmap::fromImage(QImage::fromData(binaryStream), Qt::ColorOnly));
        m_staticImage->setValue(pix);
    }

}

QString KoReportItemweb::mode() const
{
    return m_resizeMode->value().toString();
}

void KoReportItemweb::setMode(const QString &m)
{
    if (mode() != m) {
        m_resizeMode->setValue(m);
    }
}
*/

void KoReportItemweb::setUrl(const QString& url)
{
     m_url=QUrl(url);
     m_view->load(m_url);
}//ok
int KoReportItemweb::render(OROPage* page, OROSection* section,  QPointF offset, QVariant data, KRScriptHandler *script)
{
    Q_UNUSED(script)

//    QString uudata;
//    QByteArray imgdata;

QPointF pos = m_pos.toScene();
    QSizeF size = m_size.toScene();
 pos += offset;
/*    if (!isInline()) {
        imgdata = data.toByteArray();
    } else {
       0 uudata = inlineImageData();
        imgdata = KCodecs::base64Decode(uudata.toLatin1());
    }

    QImage img;
    img.loadFromData(imgdata);
    OROImage * id = new OROImage();
    id->setImage(img);
    if (mode().toLower() == "stretch") {
        id->setScaled(true);
        id->setAspectRatioMode(Qt::KeepAspectRatio);
        id->setTransformationMode(Qt::SmoothTransformation);
    }

    id->setPosition(m_pos.toScene() + offset);
    id->setSize(m_size.toScene());
    if (page) {
        page->addPrimitive(id);
    }
    
    if (section) {
        OROImage *i2 = dynamic_cast<OROImage*>(id->clone());
        i2->setPosition(m_pos.toPoint());
        section->addPrimitive(i2);
    }
    
    if (!page) {
        delete id;
    }
OROImage * id = new OROImage();
    id->setImage(*m_view);

    id->setPosition(m_pos.toScene() + offset);
    id->setSize(m_size.toScene());
    if (page) {
        page->addPrimitive(id);
    }

QGraphicsScene scene; 

   QGraphicsItem view(&scene);
   view.setFrameShape(QFrame::NoFrame);
   view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

 
   m_view->resize(300, 300);*/
   m_view->load(QUrl("http://www.kde.org"));

//   scene.addItem(view);
//   view.resize(300,300);
//   view.show();
   
    return 0; //Item doesnt stretch the section height
}



