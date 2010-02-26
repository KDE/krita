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

#include "KoReportPreRenderer.h"
#include "renderobjects.h"
#include "KoReportData.h"
#include "barcodes.h"
#include <kdeversion.h>

#include <QPrinter>
#include <QFontMetrics>
#include <QPainter>

#include <labelsizeinfo.h>
#include <kcodecs.h>
#include <KoPageFormat.h>
#include <kdebug.h>
#include <KoDpi.h>

#include <krobjectdata.h>
#include <krtextdata.h>
#include <krbarcodedata.h>
#include <krfielddata.h>
#include <krimagedata.h>
#include <krlabeldata.h>
#include <krlinedata.h>
#include <krchartdata.h>
#include <krcheckdata.h>

#include "scripting/krscripthandler.h"
#include <krreportdata.h>
#include <krdetailsectiondata.h>
#include <QResizeEvent>
#include <QApplication>

#include <KDChartAbstractDiagram>
#include <KDChartAbstractCoordinatePlane>
#include <KDChartChart>

//
// KoReportPreRendererPrivate
// This class is the private class that houses all the internal
// variables so we can provide a cleaner interface to the user
// without presenting to them things that they don't need to see
// and may change over time.
//
class KoReportPreRendererPrivate : public QObject
{
    Q_OBJECT
public:
    KoReportPreRendererPrivate();
    virtual ~KoReportPreRendererPrivate();

    bool m_valid;

    ORODocument* m_document;
    OROPage*     m_page;
    KRReportData* m_reportData;

    qreal m_yOffset;      // how far down the current page are we
    qreal m_topMargin;    // value stored in the correct units
    qreal m_bottomMargin; // -- same as above --
    qreal m_leftMargin;   // -- same as above --
    qreal m_rightMargin;  // -- same as above --
    qreal m_maxHeight;    // -- same as above --
    qreal m_maxWidth;     // -- same as above --
    int m_pageCounter;    // what page are we currently on?
    int m_recordCount;

    KoReportData* m_kodata;
    QList<OROTextBox*> m_postProcText;

    void createNewPage();
    qreal finishCurPage(bool = false);
    qreal finishCurPageSize(bool = false);

    void renderDetailSection(KRDetailSectionData &);
    qreal renderSection(const KRSectionData &);
    qreal renderSectionSize(const KRSectionData &);

    ///Scripting Stuff
    KRScriptHandler *m_scriptHandler;
    void initEngine();

signals:
    void enteredGroup(const QString&, const QVariant&);
    void exitedGroup(const QString&, const QVariant&);
    void renderingSection(KRSectionData*, OROPage*, QPointF);
};

KoReportPreRendererPrivate::KoReportPreRendererPrivate()
{
    m_valid = false;
    m_reportData = 0;
    m_document = 0;
    m_page = 0;
    m_yOffset = 0.0;
    m_topMargin = m_bottomMargin = 0.0;
    m_leftMargin = m_rightMargin = 0.0;
    m_pageCounter = 0;
    m_maxHeight = m_maxWidth = 0.0;
    m_kodata = 0;
}

KoReportPreRendererPrivate::~KoReportPreRendererPrivate()
{
    if (m_reportData) {
        delete m_reportData;
        m_reportData = 0;
    }

    m_postProcText.clear();
}

void KoReportPreRendererPrivate::createNewPage()
{
    if (m_pageCounter > 0)
        finishCurPage();

    m_pageCounter++;

    //Update the page count script value
    m_scriptHandler->setPageNumber(m_pageCounter);
    m_scriptHandler->newPage();

    m_page = new OROPage(0);
    m_document->addPage(m_page);

    //TODO calculate past page
    bool lastPage = false;

    m_yOffset = m_topMargin;

    if (m_pageCounter == 1 && m_reportData->m_pageHeaderFirst)
        renderSection(*(m_reportData->m_pageHeaderFirst));
    else if (lastPage == true && m_reportData->m_pageHeaderLast)
        renderSection(*(m_reportData->m_pageHeaderLast));
    else if ((m_pageCounter % 2) == 1 && m_reportData->m_pageHeaderOdd)
        renderSection(*(m_reportData->m_pageHeaderOdd));
    else if ((m_pageCounter % 2) == 0 && m_reportData->m_pageHeaderAny)
        renderSection(*(m_reportData->m_pageHeaderAny));
    else if (m_reportData->m_pageHeaderAny)
        renderSection(*(m_reportData->m_pageHeaderAny));
}

qreal KoReportPreRendererPrivate::finishCurPageSize(bool lastPage)
{
    qreal retval = 0.0;

    if (lastPage && m_reportData->m_pageFooterLast)
        retval = renderSectionSize(* (m_reportData->m_pageFooterLast));
    else if (m_pageCounter == 1 && m_reportData->m_pageFooterFirst)
        retval = renderSectionSize(* (m_reportData->m_pageFooterFirst));
    else if ((m_pageCounter % 2) == 1 && m_reportData->m_pageFooterOdd)
        retval = renderSectionSize(* (m_reportData->m_pageFooterOdd));
    else if ((m_pageCounter % 2) == 0 && m_reportData->m_pageFooterEven)
        retval = renderSectionSize(* (m_reportData->m_pageFooterEven));
    else if (m_reportData->m_pageFooterAny)
        retval = renderSectionSize(* (m_reportData->m_pageFooterAny));

    kDebug() << retval;
    return retval;
}

qreal KoReportPreRendererPrivate::finishCurPage(bool lastPage)
{

    qreal offset = m_maxHeight - m_bottomMargin;
    qreal retval = 0.0;

    kDebug() << offset;

    if (lastPage && m_reportData->m_pageFooterLast) {
        kDebug() << "Last Footer";
        m_yOffset = offset - renderSectionSize(* (m_reportData->m_pageFooterLast));
        retval = renderSection(* (m_reportData->m_pageFooterLast));
    } else if (m_pageCounter == 1 && m_reportData->m_pageFooterFirst) {
        kDebug() << "First Footer";
        m_yOffset = offset - renderSectionSize(* (m_reportData->m_pageFooterFirst));
        retval = renderSection(* (m_reportData->m_pageFooterFirst));
    } else if ((m_pageCounter % 2) == 1 && m_reportData->m_pageFooterOdd) {
        kDebug() << "Odd Footer";
        m_yOffset = offset - renderSectionSize(* (m_reportData->m_pageFooterOdd));
        retval = renderSection(* (m_reportData->m_pageFooterOdd));
    } else if ((m_pageCounter % 2) == 0 && m_reportData->m_pageFooterEven) {
        kDebug() << "Even Footer";
        m_yOffset = offset - renderSectionSize(* (m_reportData->m_pageFooterEven));
        retval = renderSection(* (m_reportData->m_pageFooterEven));
    } else if (m_reportData->m_pageFooterAny) {
        kDebug() << "Any Footer";
        m_yOffset = offset - renderSectionSize(* (m_reportData->m_pageFooterAny));
        retval = renderSection(* (m_reportData->m_pageFooterAny));
    }

    return retval;
}

void KoReportPreRendererPrivate::renderDetailSection(KRDetailSectionData & detailData)
{
    kDebug();

    if (detailData.m_detailSection) {
        if (m_kodata/* && !curs->eof()*/) {
            QStringList keys;
            QStringList keyValues;
            bool status = false;
            QList<int> shownGroups;
            ORDetailGroupSectionData * grp = 0;

            status = m_kodata->moveFirst();
            m_recordCount = m_kodata->recordCount();

            kDebug() << "Record Count:" << m_recordCount;

            for (int i = 0; i < (int) detailData.m_groupList.count(); ++i) {
                grp = detailData.m_groupList[i];
                //If the group has a header or footer, then emit a change of group value
                if(grp->m_groupFooter || grp->m_groupHeader) {
                    // we get here only if group is *shown*
                    shownGroups << i;
                    keys.append(grp->m_column);
                    if (!keys.last().isEmpty())
                        keyValues.append(m_kodata->value(m_kodata->fieldNumber(keys.last())).toString());
                    else
                        keyValues.append(QString());

                    //Tell interested parties we're about to render a header
                    kDebug() << "EMIT1";
                    emit(enteredGroup(keys.last(), keyValues.last()));
                }
                if (grp->m_groupHeader)
                    renderSection(*(grp->m_groupHeader));
            }

            while (status) {
                long l = m_kodata->at();

                kDebug() << "At:" << l << "Y:" << m_yOffset;

                if (renderSectionSize(*(detailData.m_detailSection)) + finishCurPageSize((l + 1 == m_recordCount)) + m_bottomMargin + m_yOffset >= m_maxHeight) {
                    if (l > 0) {
                        kDebug() << "...moving prev";
                        m_kodata->movePrevious();
                        kDebug() << "...creating new page";
                        createNewPage();
                        kDebug() << "...moving next";
                        m_kodata->moveNext();
                    }
                }

                renderSection(*(detailData.m_detailSection));
                kDebug() << "...moving next";
                if (m_kodata)
                    status = m_kodata->moveNext();
                kDebug() << "...done";

                if (status == true && keys.count() > 0) {
                    // check to see where it is we need to start
                    int pos = -1; // if it's still -1 by the time we are done then no keyValues changed
                    for (int i = 0; i < keys.count(); ++i) {
                        if (keyValues[i] != m_kodata->value(m_kodata->fieldNumber(keys[i])).toString()) {
                            pos = i;
                            break;
                        }
                    }
                    // don't bother if nothing has changed
                    if (pos != -1) {
                        // roll back the query and go ahead if all is good
                        status = m_kodata->movePrevious();
                        if (status == true) {
                            // print the footers as needed
                            // any changes made in this for loop need to be duplicated
                            // below where the footers are finished.
                            bool do_break = false;
                            for (int i = shownGroups.count() - 1; i >= 0; i--) {
                                if (do_break)
                                    createNewPage();
                                do_break = false;
                                grp = detailData.m_groupList[shownGroups.at(i)];

                                if (grp->m_groupFooter) {
                                    if (renderSectionSize(*(grp->m_groupFooter)) + finishCurPageSize() + m_bottomMargin + m_yOffset >= m_maxHeight)
                                        createNewPage();
                                    renderSection(*(grp->m_groupFooter));
                                }

                                if (ORDetailGroupSectionData::BreakAfterGroupFooter == grp->m_pagebreak)
                                    do_break = true;
                            }
                            // step ahead to where we should be and print the needed headers
                            // if all is good
                            status = m_kodata->moveNext();
                            if (do_break)
                                createNewPage();
                            if (status == true) {
                                for (int i = 0; i < shownGroups.count(); ++i) {
                                    grp = detailData.m_groupList[shownGroups.at(i)];

                                    if (grp->m_groupHeader) {
                                        if (renderSectionSize(*(grp->m_groupHeader)) + finishCurPageSize() + m_bottomMargin + m_yOffset >= m_maxHeight) {
                                            m_kodata->movePrevious();
                                            createNewPage();
                                            m_kodata->moveNext();
                                        }
                                        
                                        if (!keys[i].isEmpty())
                                            keyValues[i] = m_kodata->value(m_kodata->fieldNumber(keys[i])).toString();

                                        //Tell interested parties thak key values changed
                                        kDebug() << "EMIT2";

                                        renderSection(*(grp->m_groupHeader));
                                    }


                                }
                            }
                        }
                    }
                }
            }

            if (keys.size() > 0 && m_kodata->movePrevious()) {
                // finish footers
                // duplicated changes from above here
                for (int i = shownGroups.count() - 1; i >= 0; i--) {
                    grp = detailData.m_groupList[shownGroups.at(i)];

                    if (grp->m_groupFooter) {
                        if (renderSectionSize(*(grp->m_groupFooter)) + finishCurPageSize() + m_bottomMargin + m_yOffset >= m_maxHeight)
                            createNewPage();
                        renderSection(*(grp->m_groupFooter));
                        emit(exitedGroup(keys[i], keyValues[i]));
                    }
                }
            }
        }
        if (KRDetailSectionData::BreakAtEnd == detailData.m_pageBreak)
            createNewPage();
    }
}

qreal KoReportPreRendererPrivate::renderSectionSize(const KRSectionData & sectionData)
{
    qreal intHeight = POINT_TO_INCH(sectionData.height()) * KoDpi::dpiY();

    if (sectionData.objects().count() == 0)
        return intHeight;

    QList<KRObjectData*> objects = sectionData.objects();
    KRObjectData * elemThis;
    foreach(KRObjectData *ob, objects) {
        elemThis = ob;
        //++it;
        // TODO: See if this can be simplified anymore than it already is.
        //       All we need to know is how much stretch we are going to get.
        if (elemThis->type() == KRObjectData::EntityText) {
            KRTextData * t = elemThis->toText();

            QPointF pos = t->m_pos.toScene();
            QSizeF size = t->m_size.toScene();
            pos += QPointF(m_leftMargin, m_yOffset);

            QRectF trf(pos, size);

            QString qstrValue;
            qreal   intStretch      = trf.top() - m_yOffset;
            qreal   intRectHeight   = trf.height();

            QFont f = t->m_font->value().value<QFont>();

            qstrValue = m_kodata->value(t->m_controlSource->value().toString()).toString();
            if (qstrValue.length()) {
                int pos = 0;
                int idx;
                QChar separator;
                QRegExp re("\\s");
                QPrinter prnt(QPrinter::HighResolution);
                QFontMetrics fm(f, &prnt);

                int   intRectWidth    = (int)((t->m_size.toPoint().width() / 72) * prnt.resolution());

                while (qstrValue.length()) {
                    idx = re.indexIn(qstrValue, pos);
                    if (idx == -1) {
                        idx = qstrValue.length();
                        separator = QChar('\n');
                    } else
                        separator = qstrValue.at(idx);

                    if (fm.boundingRect(qstrValue.left(idx)).width() < intRectWidth || pos == 0) {
                        pos = idx + 1;
                        if (separator == '\n') {
                            qstrValue = qstrValue.mid(idx + 1, qstrValue.length());
                            pos = 0;

                            intStretch += intRectHeight;
                        }
                    } else {
                        qstrValue = qstrValue.mid(pos, qstrValue.length());
                        pos = 0;

                        intStretch += intRectHeight;
                    }
                }

                intStretch += (t->m_bottomPadding / 100.0);

                if (intStretch > intHeight)
                    intHeight = intStretch;
            }
        }
    }

    return intHeight;
}

qreal KoReportPreRendererPrivate::renderSection(const KRSectionData & sectionData)
{
    qreal intHeight = POINT_TO_INCH(sectionData.height()) * KoDpi::dpiY();
    kDebug() << "Name: " << sectionData.name() << " Height: " << intHeight << "Objects: " << sectionData.objects().count();

    //_handler->populateEngineParameters(_query->getQuery());

    emit(renderingSection(const_cast<KRSectionData*>(&sectionData), m_page, QPointF(m_leftMargin, m_yOffset)));

    //Create a pre-rendered section for this section and add it to the document
    OROSection *sec = new OROSection(m_document);
    sec->setHeight(sectionData.height());
    sec->setBackgroundColor(sectionData.backgroundColor());
    sec->setType(sectionData.type());
    m_document->addSection(sec);

    //Render section background
    ORORect* bg = new ORORect();
    bg->setPen(QPen(Qt::NoPen));
    bg->setBrush(sectionData.backgroundColor());
    qreal w = m_page->document()->pageOptions().widthPx() - m_page->document()->pageOptions().getMarginRight() - m_leftMargin;

    bg->setRect(QRectF(m_leftMargin, m_yOffset, w, intHeight));
    m_page->addPrimitive(bg, true);

    QList<KRObjectData*> objects = sectionData.objects();
    KRObjectData * elemThis;
    foreach(KRObjectData *ob, objects) {
        elemThis = ob;
        if (elemThis->type() == KRObjectData::EntityLabel) {
            KRLabelData * l = elemThis->toLabel();
            QPointF pos = l->m_pos.toScene();
            //QSizeF size = l->_size.toScene();

            pos += QPointF(m_leftMargin, m_yOffset);

            OROTextBox * tb = new OROTextBox();
            tb->setPosition(pos);
            tb->setSize(l->m_size.toScene());
            tb->setFont(l->font());
            tb->setText(l->text());
            tb->setFlags(l->textFlags());
            tb->setTextStyle(l->textStyle());
            tb->setLineStyle(l->lineStyle());
            m_page->addPrimitive(tb);

            OROTextBox *tb2 = dynamic_cast<OROTextBox*>(tb->clone());
            tb2->setPosition(l->m_pos.toPoint());
            sec->addPrimitive(tb2);
        } else if (elemThis->type() == KRObjectData::EntityField) {
            KRFieldData* f = elemThis->toField();

            QPointF pos = f->m_pos.toScene();
            QSizeF size = f->m_size.toScene();
            pos += QPointF(m_leftMargin, m_yOffset);

            OROTextBox * tb = new OROTextBox();
            tb->setPosition(pos);
            tb->setSize(size);
            tb->setFont(f->font());
            tb->setFlags(f->textFlags());
            tb->setTextStyle(f->textStyle());
            tb->setLineStyle(f->lineStyle());

            QString str;

            QString cs = f->m_controlSource->value().toString();
            if (cs.left(1) == "=") { //Everything after = is treated as code
                if (!cs.contains("PageTotal()")) {
#if KDE_IS_VERSION(4,2,88)
                    QVariant v = m_scriptHandler->evaluate(cs.mid(1));
#else
                    QVariant v = m_scriptHandler->evaluate(f->entityName());
#endif

                    str = v.toString();
                } else {
#if KDE_IS_VERSION(4,2,88)
                    str = cs.mid(1);
#else
                    str = f->entityName();
#endif
                    m_postProcText.append(tb);
                }
            } else if (cs.left(1) == "$") { //Everything past $ is treated as a string
                str = cs.mid(1);
            } else {
                //QString qry = "Data Source";
                QString clm = f->m_controlSource->value().toString();

                //populateData(f->data(), dataThis);
                //str = dataThis.getValue();
                str = m_kodata->value(clm).toString();
            }
            tb->setText(str);
            m_page->addPrimitive(tb);

            OROTextBox *tb2 = dynamic_cast<OROTextBox*>(tb->clone());
            tb2->setPosition(f->m_pos.toPoint());
            sec->addPrimitive(tb2);


        } else if (elemThis->type() == KRObjectData::EntityText) {
            QString qstrValue;
            KRTextData * t = elemThis->toText();

            QString cs = t->m_controlSource->value().toString();

            kDebug() << cs;

            if (cs.left(1) == "$") { //Everything past $ is treated as a string
                qstrValue = cs.mid(1);
            } else {
                qstrValue = m_kodata->value(t->m_controlSource->value().toString()).toString();
            }

            QPointF pos = t->m_pos.toScene();
            QSizeF size = t->m_size.toScene();
            pos += QPointF(m_leftMargin, m_yOffset);

            QRectF trf(pos, size);

            int     intLineCounter  = 0;
            qreal   intStretch      = trf.top() - m_yOffset;
            qreal   intBaseTop      = trf.top();
            qreal   intRectHeight   = trf.height();

            QFont f = t->font();

            kDebug() << qstrValue;

            if (qstrValue.length()) {
                QRectF rect = trf;

                int pos = 0;
                int idx;
                QChar separator;
                QRegExp re("\\s");
                QPrinter prnt(QPrinter::HighResolution);
                QFontMetrics fm(f, &prnt);

//                int   intRectWidth    = (int)(trf.width() * prnt.resolution()) - 10;
                int   intRectWidth    = (int)((t->m_size.toPoint().width() / 72) * prnt.resolution());

                while (qstrValue.length()) {
                    idx = re.indexIn(qstrValue, pos);
                    if (idx == -1) {
                        idx = qstrValue.length();
                        separator = QChar('\n');
                    } else
                        separator = qstrValue.at(idx);

                    if (fm.boundingRect(qstrValue.left(idx)).width() < intRectWidth || pos == 0) {
                        pos = idx + 1;
                        if (separator == '\n') {
                            QString line = qstrValue.left(idx);
                            qstrValue = qstrValue.mid(idx + 1, qstrValue.length());
                            pos = 0;

                            rect.setTop(intBaseTop + (intLineCounter * intRectHeight));
                            rect.setBottom(rect.top() + intRectHeight);

                            OROTextBox * tb = new OROTextBox();
                            tb->setPosition(rect.topLeft());
                            tb->setSize(rect.size());
                            tb->setFont(t->font());
                            tb->setText(line);
                            tb->setFlags(t->textFlags());
                            tb->setTextStyle(t->textStyle());
                            tb->setLineStyle(t->lineStyle());
                            m_page->addPrimitive(tb);

                            OROTextBox *tb2 = dynamic_cast<OROTextBox*>(tb->clone());
                            tb2->setPosition(t->m_pos.toPoint());
                            sec->addPrimitive(tb2);

                            intStretch += intRectHeight;
                            intLineCounter++;
                        }
                    } else {
                        QString line = qstrValue.left(pos - 1);
                        qstrValue = qstrValue.mid(pos, qstrValue.length());
                        pos = 0;

                        rect.setTop(intBaseTop + (intLineCounter * intRectHeight));
                        rect.setBottom(rect.top() + intRectHeight);

                        OROTextBox * tb = new OROTextBox();
                        tb->setPosition(rect.topLeft());
                        tb->setSize(rect.size());
                        tb->setFont(t->font());
                        tb->setText(line);
                        tb->setFlags(t->textFlags());
                        tb->setTextStyle(t->textStyle());
                        tb->setLineStyle(t->lineStyle());
                        m_page->addPrimitive(tb);

                        intStretch += intRectHeight;
                        intLineCounter++;
                    }
                }

                intStretch += (t->m_bottomPadding / 100.0);

                if (intStretch > intHeight)
                    intHeight = intStretch;
            }
        } else if (elemThis->type() == KRObjectData::EntityLine) {
            KRLineData * l = elemThis->toLine();
            OROLine * ln = new OROLine();
            QPointF s = l->m_start.toScene();
            QPointF e = l->m_end.toScene();
            QPointF offset(m_leftMargin, m_yOffset);
            s += offset;
            e += offset;

            ln->setStartPoint(s);
            ln->setEndPoint(e);
            ln->setLineStyle(l->lineStyle());
            m_page->addPrimitive(ln);

            OROLine *l2 = dynamic_cast<OROLine*>(ln->clone());
            l2->setStartPoint(l->m_start.toPoint());
            l2->setEndPoint(l->m_end.toPoint());
            sec->addPrimitive(l2);
        } else if (elemThis->type() == KRObjectData::EntityBarcode) {
            KRBarcodeData * bc = elemThis->toBarcode();

            QPointF pos = bc->m_pos.toScene();
            QSizeF size = bc->m_size.toScene();
            pos += QPointF(m_leftMargin, m_yOffset);

            QRectF rect = QRectF(pos, size);

            QString val = m_kodata->value(bc->m_controlSource->value().toString()).toString();
            QString fmt = bc->m_format->value().toString();
            int align = bc->alignment();
            if (fmt == "3of9")
                render3of9(m_page, rect, val, align);
            else if (fmt == "3of9+")
                renderExtended3of9(m_page, rect, val, align);
            else if (fmt == "128")
                renderCode128(m_page, rect, val, align);
            else if (fmt == "ean13")
                renderCodeEAN13(m_page, rect, val, align);
            else if (fmt == "ean8")
                renderCodeEAN8(m_page, rect, val, align);
            else if (fmt == "upc-a")
                renderCodeUPCA(m_page, rect, val, align);
            else if (fmt == "upc-e")
                renderCodeUPCE(m_page, rect, val, align);
            else {
                //logMessage("Encountered unknown barcode format: %s",(const char*)bc->format);
            }
        } else if (elemThis->type() == KRObjectData::EntityImage) {
            KRImageData * im = elemThis->toImage();
            QString uudata;
            QByteArray imgdata;
            if (!im->isInline()) {
//TODO load images from database

            } else {
                uudata = im->inlineImageData();
                imgdata = KCodecs::base64Decode(uudata.toLatin1());
            }

            QImage img;
            img.loadFromData(imgdata);
            OROImage * id = new OROImage();
            id->setImage(img);
            if (im->mode().toLower() == "stretch") {
                id->setScaled(true);
                id->setAspectRatioMode(Qt::KeepAspectRatio);
                id->setTransformationMode(Qt::SmoothTransformation);
            }
            QPointF pos = im->m_pos.toScene();
            QSizeF size = im->m_size.toScene();

            pos += QPointF(m_leftMargin, m_yOffset);

            id->setPosition(pos);
            id->setSize(size);
            m_page->addPrimitive(id);

            OROImage *i2 = dynamic_cast<OROImage*>(id->clone());
            i2->setPosition(im->m_pos.toPoint());
            sec->addPrimitive(i2);
        } else if (elemThis->type() == KRObjectData::EntityChart) {
            KRChartData * ch = elemThis->toChart();
            ch->setConnection(m_kodata);

            QStringList masterFields = ch->masterFields();
            for (int i = 0; i < masterFields.size(); ++i) {
                if (!masterFields[i].simplified().isEmpty()) {
                    //            ch->setLinkData(masterFields[i], _query->getQuery()->value(_query->fieldNumber(masterFields[i])));
                }
            }
            ch->populateData();
            if (ch->widget()) {
                OROPicture * id = new OROPicture();
                ch->widget()->setFixedSize(ch->m_size.toScene().toSize());

                QPainter p(id->picture());

                ch->widget()->diagram()->coordinatePlane()->parent()->paint(&p, QRect(QPoint(0, 0), ch->m_size.toScene().toSize()));

                QPointF pos = ch->m_pos.toScene();
                QSizeF size = ch->m_size.toScene();

                pos += QPointF(m_leftMargin, m_yOffset);

                id->setPosition(pos);
                id->setSize(size);
                m_page->addPrimitive(id);

                OROPicture *p2 = dynamic_cast<OROPicture*>(id->clone());
                p2->setPosition(ch->m_pos.toPoint());
                sec->addPrimitive(p2);
            }
        } else if (elemThis->type() == KRObjectData::EntityCheck) {
            KRCheckData *cd = elemThis->toCheck();
            OROCheck *chk = new OROCheck();

            QPointF pos = cd->m_pos.toScene();
            QSizeF size = cd->m_size.toScene();
            pos += QPointF(m_leftMargin, m_yOffset);

            chk->setPosition(pos);
            chk->setSize(size);

            chk->setLineStyle(cd->lineStyle());
            chk->setForegroundColor(cd->m_foregroundColor->value().value<QColor>());
            chk->setCheckType(cd->m_checkStyle->value().toString());

            QString str;

            QString cs = cd->controlSource();
            kDebug() << "EntityCheck CS:" << cs;

            if (cs.left(1) == "=") {
#if KDE_IS_VERSION(4,2,88)
                str = m_scriptHandler->evaluate(cs.mid(1)).toString();
#else
                str = m_scriptHandler->evaluate(cd->entityName()).toString();
#endif
            } else {
                QString clm = cd->m_controlSource->value().toString();
                str = m_kodata->value(clm).toString();
            }

            bool v = false;

            str = str.toLower();

            kDebug() << "Check Value:" << str;
            if (str == "t" || str == "true" || str == "1")
                v = true;

            chk->setValue(v);

            m_page->addPrimitive(chk);
            OROCheck *chk2 = dynamic_cast<OROCheck*>(chk->clone());
            chk2->setPosition(cd->m_pos.toPoint());
            sec->addPrimitive(chk2);
        } else {
            kDebug() << "Encountered an unknown element while rendering a section.";
        }
    }

    m_yOffset += intHeight;

    kDebug() << m_yOffset;
    return intHeight;
}

void KoReportPreRendererPrivate::initEngine()
{
    m_scriptHandler = new KRScriptHandler(m_kodata, m_reportData);

    connect(this, SIGNAL(enteredGroup(const QString&, const QVariant&)), m_scriptHandler, SLOT(slotEnteredGroup(const QString&, const QVariant&)));

    connect(this, SIGNAL(exitedGroup(const QString&, const QVariant&)), m_scriptHandler, SLOT(slotExitedGroup(const QString&, const QVariant&)));

    connect(this, SIGNAL(renderingSection(KRSectionData*, OROPage*, QPointF)), m_scriptHandler, SLOT(slotEnteredSection(KRSectionData*, OROPage*, QPointF)));
}

//
// ORPreRender
//

KoReportPreRenderer::KoReportPreRenderer(const QDomElement & pDocument)
{
    d = new KoReportPreRendererPrivate();
    setDom(pDocument);
}

KoReportPreRenderer::~KoReportPreRenderer()
{
}

void KoReportPreRenderer::setName(const QString &n)
{
    d->m_reportData->setName(n);
}

ORODocument* KoReportPreRenderer::generate()
{
    kDebug();
    if (d == 0 || !d->m_valid || d->m_reportData == 0 || d->m_kodata == 0)
        return 0;

    // Do this check now so we don't have to undo a lot of work later if it fails
    LabelSizeInfo label;
    if (d->m_reportData->page.getPageSize() == "Labels") {
        label = LabelSizeInfo::find(d->m_reportData->page.getLabelType());
        if (label.isNull())
            return 0;
    }

    kDebug() << "Creating Document";
    d->m_document = new ORODocument(d->m_reportData->m_title);

    d->m_pageCounter  = 0;
    d->m_yOffset      = 0.0;

    kDebug() << "Calculating Margins";
    if (!label.isNull()) {
        if (d->m_reportData->page.isPortrait()) {
            d->m_topMargin = (label.startY() / 100.0);
            d->m_bottomMargin = 0;
            d->m_rightMargin = 0;
            d->m_leftMargin = (label.startX() / 100.0);
        } else {
            d->m_topMargin = (label.startX() / 100.0);
            d->m_bottomMargin = 0;
            d->m_rightMargin = 0;
            d->m_leftMargin = (label.startY() / 100.0);
        }
    } else {
        d->m_topMargin    = d->m_reportData->page.getMarginTop();
        d->m_bottomMargin = d->m_reportData->page.getMarginBottom();
        d->m_rightMargin  = d->m_reportData->page.getMarginRight();
        d->m_leftMargin   = d->m_reportData->page.getMarginLeft();
        kDebug() << "Margins:" << d->m_topMargin << d->m_bottomMargin << d->m_rightMargin << d->m_leftMargin;
    }

    kDebug() << "Calculating Page Size";
    ReportPageOptions rpo(d->m_reportData->page);
    // This should reflect the information of the report page size
    if (d->m_reportData->page.getPageSize() == "Custom") {
        d->m_maxWidth = d->m_reportData->page.getCustomWidth();
        d->m_maxHeight = d->m_reportData->page.getCustomHeight();
    } else {
        if (!label.isNull()) {
            d->m_maxWidth = label.width();
            d->m_maxHeight = label.height();
            rpo.setPageSize(label.paper());
        } else {
            // lookup the correct size information for the specified size paper
            d->m_maxWidth = KoPageFormat::width(KoPageFormat::formatFromString(d->m_reportData->page.getPageSize()), KoPageFormat::Portrait);
            d->m_maxHeight = KoPageFormat::height(KoPageFormat::formatFromString(d->m_reportData->page.getPageSize()), KoPageFormat::Portrait);

            KoUnit pageUnit(KoUnit::Millimeter);
            d->m_maxWidth = KoUnit::toInch(pageUnit.fromUserValue(d->m_maxWidth)) * KoDpi::dpiX();
            d->m_maxHeight = KoUnit::toInch(pageUnit.fromUserValue(d->m_maxHeight)) * KoDpi::dpiY();
        }
    }

    if (!d->m_reportData->page.isPortrait()) {
        qreal tmp = d->m_maxWidth;
        d->m_maxWidth = d->m_maxHeight;
        d->m_maxHeight = tmp;
    }

    kDebug() << "Page Size:" << d->m_maxWidth << d->m_maxHeight;

    d->m_document->setPageOptions(rpo);
    d->m_kodata->setSorting(d->m_reportData->m_detailSection->m_sortedFields);
    d->m_kodata->open();
    d->initEngine();

    //Loop through all abjects that have been registered, and register them with the script handler
    if (d->m_scriptHandler) {
        QMapIterator<QString, QObject*> i(m_scriptObjects);
        while (i.hasNext()) {
            i.next();
            d->m_scriptHandler->registerScriptObject(i.value(), i.key());

            //!TODO This is a hack
            if (i.key() == "field")
                QObject::connect(d->m_scriptHandler, SIGNAL(groupChanged(const QString&)), i.value(), SLOT(setWhere(const QString&)));
        }
    }

    //execute the script
    d->m_scriptHandler->trigger();

    d->createNewPage();
    if (!label.isNull()) {
// Label Print Run
        int row = 0;
        int col = 0;

        // remember the initial margin setting as we will be modifying
        // the value and restoring it as we move around
        qreal margin = d->m_leftMargin;

        d->m_yOffset = d->m_topMargin;

        qreal w = (label.width() / 100.0);
        qreal wg = (label.xGap() / 100.0);
        qreal h = (label.height() / 100.0);
        qreal hg = (label.yGap() / 100.0);
        int numCols = label.columns();
        int numRows = label.rows();
        qreal tmp;

        // flip the value around if we are printing landscape
        if (!d->m_reportData->page.isPortrait()) {
            w = (label.height() / 100.0);
            wg = (label.yGap() / 100.0);
            h = (label.width() / 100.0);
            hg = (label.xGap() / 100.0);
            numCols = label.rows();
            numRows = label.columns();
        }

        KRDetailSectionData * detailData = d->m_reportData->m_detailSection;
        if (detailData->m_detailSection) {
            KoReportData *mydata = d->m_kodata;

            if (mydata && mydata->recordCount() > 0) { /* && !((query = orqThis->getQuery())->eof()))*/
                mydata->moveFirst();
                do {
                    tmp = d->m_yOffset; // store the value as renderSection changes it
                    d->renderSection(*(detailData->m_detailSection));
                    d->m_yOffset = tmp; // restore the value that renderSection modified

                    col++;
                    d->m_leftMargin += w + wg;
                    if (col >= numCols) {
                        d->m_leftMargin = margin; // reset back to original value
                        col = 0;
                        row++;
                        d->m_yOffset += h + hg;
                        if (row >= numRows) {
                            d->m_yOffset = d->m_topMargin;
                            row = 0;
                            d->createNewPage();
                        }
                    }
                } while (mydata->moveNext());
            }
        }

    } else {
// Normal Print Run
        if (d->m_reportData->m_reportHeader) {
            d->renderSection(*(d->m_reportData->m_reportHeader));
        }

        if (d->m_reportData->m_detailSection) {
            d->renderDetailSection(*(d->m_reportData->m_detailSection));
        }

        if (d->m_reportData->m_reportFooter) {
            if (d->renderSectionSize(*(d->m_reportData->m_reportFooter)) + d->finishCurPageSize(true) + d->m_bottomMargin + d->m_yOffset >= d->m_maxHeight) {
                d->createNewPage();
            }
            d->renderSection(*(d->m_reportData->m_reportFooter));
        }
    }
    d->finishCurPage(true);

    // _postProcText contains those text boxes that need to be updated
    // with information that wasn't available at the time it was added to the document
    d->m_scriptHandler->setPageTotal(d->m_document->pages());

    for (int i = 0; i < d->m_postProcText.size(); i++) {
        OROTextBox * tb = d->m_postProcText.at(i);

        d->m_scriptHandler->setPageNumber(tb->page()->page() + 1);

        tb->setText(d->m_scriptHandler->evaluate(tb->text()).toString());
    }

    d->m_scriptHandler->displayErrors();

    d->m_kodata->close();
    delete d->m_scriptHandler;
    delete d->m_kodata
    ;
    d->m_postProcText.clear();

    ORODocument * pDoc = d->m_document;
    d->m_document = 0;
    return pDoc;
}

void KoReportPreRenderer::setSourceData(KoReportData *data)
{
    if (d && data) {
        d->m_kodata = data;
    }
}

bool KoReportPreRenderer::setDom(const QDomElement &docReport)
{
    if (d) {
        if (d->m_reportData)
            delete d->m_reportData;
        d->m_valid = false;

	if (docReport.tagName() != "report:content") {
		kDebug() << "report schema is invalid";
		return false;
	}
	
        d->m_reportData = new KRReportData(docReport);
        d->m_valid = d->m_reportData->isValid();
    }
    return isValid();
}

bool KoReportPreRenderer::isValid() const
{
    if (d && d->m_valid)
        return true;
    return false;
}

void KoReportPreRenderer::registerScriptObject(QObject* obj, const QString& name)
{
    kDebug() << name;
    m_scriptObjects[name] = obj;
}

#include <orprerenderprivate.moc>
