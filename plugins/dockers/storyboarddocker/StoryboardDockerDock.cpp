/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "StoryboardDockerDock.h"
#include "CommentDelegate.h"
#include "CommentModel.h"
#include "StoryboardModel.h"
#include "StoryboardDelegate.h"
#include "StoryboardView.h"
#include "DlgExportStoryboard.h"

#include <QMenu>
#include <QButtonGroup>
#include <QDebug>
#include <QStringListModel>
#include <QListView>
#include <QItemSelection>
#include <QSize>
#include <QPrinter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QSvgGenerator>
#include <QMessageBox>

#include <klocalizedstring.h>

#include <KisPart.h>
#include <KisViewManager.h>
#include <kis_node_manager.h>
#include <KisDocument.h>
#include <kis_icon.h>
#include <kis_image_animation_interface.h>
#include <kis_time_span.h>

#include "ui_wdgstoryboarddock.h"
#include "ui_wdgcommentmenu.h"
#include "ui_wdgarrangemenu.h"

enum Mode {
    Column,
    Row,
    Grid
};

enum View {
    All,
    ThumbnailsOnly,
    CommentsOnly
};

class CommentMenu: public QMenu
{
    Q_OBJECT
public:
    CommentMenu(QWidget *parent, StoryboardCommentModel *m_model)
        : QMenu(parent)
        , m_menuUI(new Ui_WdgCommentMenu())
        , model(m_model)
        , delegate(new CommentDelegate(this))
    {
        QWidget* commentWidget = new QWidget(this);
        m_menuUI->setupUi(commentWidget);

        m_menuUI->fieldListView->setDragEnabled(true);
        m_menuUI->fieldListView->setAcceptDrops(true);
        m_menuUI->fieldListView->setDropIndicatorShown(true);
        m_menuUI->fieldListView->setDragDropMode(QAbstractItemView::InternalMove);

        m_menuUI->fieldListView->setModel(model);
        m_menuUI->fieldListView->setItemDelegate(delegate);

        m_menuUI->fieldListView->setEditTriggers(QAbstractItemView::AnyKeyPressed |
                                                    QAbstractItemView::DoubleClicked  );

        m_menuUI->btnAddField->setIcon(KisIconUtils::loadIcon("list-add"));
        m_menuUI->btnDeleteField->setIcon(KisIconUtils::loadIcon("trash-empty"));
        m_menuUI->btnAddField->setIconSize(QSize(22, 22));
        m_menuUI->btnDeleteField->setIconSize(QSize(22, 22));
        connect(m_menuUI->btnAddField, SIGNAL(clicked()), this, SLOT(slotaddItem()));
        connect(m_menuUI->btnDeleteField, SIGNAL(clicked()), this, SLOT(slotdeleteItem()));

        KisAction *commentAction = new KisAction(commentWidget);
        commentAction->setDefaultWidget(commentWidget);
        this->addAction(commentAction);
    }

private Q_SLOTS:
    void slotaddItem()
    {
        int row = m_menuUI->fieldListView->currentIndex().row()+1;
        model->insertRows(row, 1);

        QModelIndex index = model->index(row);
        m_menuUI->fieldListView->setCurrentIndex(index);
        m_menuUI->fieldListView->edit(index);
    }

    void slotdeleteItem()
    {
        model->removeRows(m_menuUI->fieldListView->currentIndex().row(), 1);
    }

private:
    QScopedPointer<Ui_WdgCommentMenu> m_menuUI;
    StoryboardCommentModel *model;
    CommentDelegate *delegate;
};

class ArrangeMenu: public QMenu
{
public:
    ArrangeMenu(QWidget *parent)
        : QMenu(parent)
        , m_menuUI(new Ui_WdgArrangeMenu())
        , modeGroup(new QButtonGroup(this))
        , viewGroup(new QButtonGroup(this))
    {
        QWidget* arrangeWidget = new QWidget(this);
        m_menuUI->setupUi(arrangeWidget);

        modeGroup->addButton(m_menuUI->btnColumnMode, Mode::Column);
        modeGroup->addButton(m_menuUI->btnRowMode, Mode::Row);
        modeGroup->addButton(m_menuUI->btnGridMode, Mode::Grid);

        viewGroup->addButton(m_menuUI->btnAllView, View::All);
        viewGroup->addButton(m_menuUI->btnThumbnailsView, View::ThumbnailsOnly);
        viewGroup->addButton(m_menuUI->btnCommentsView, View::CommentsOnly);

        KisAction *arrangeAction = new KisAction(arrangeWidget);
        arrangeAction->setDefaultWidget(arrangeWidget);
        this->addAction(arrangeAction);
    }

    QButtonGroup* getModeGroup(){ return modeGroup;}
    QButtonGroup* getViewGroup(){ return viewGroup;}

private:
    QScopedPointer<Ui_WdgArrangeMenu> m_menuUI;
    QButtonGroup *modeGroup;
    QButtonGroup *viewGroup;
};

StoryboardDockerDock::StoryboardDockerDock( )
    : QDockWidget(i18nc("Storyboard Docker", "Storyboard"))
    , m_canvas(0)
    , m_ui(new Ui_WdgStoryboardDock())
    , m_exportMenu(new QMenu(this))
    , m_commentModel(new StoryboardCommentModel(this))
    , m_commentMenu(new CommentMenu(this, m_commentModel))
    , m_arrangeMenu(new ArrangeMenu(this))
    , m_storyboardModel(new StoryboardModel(this))
    , m_storyboardDelegate(new StoryboardDelegate(this))
{
    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);
    m_ui->setupUi(mainWidget);

    m_ui->btnExport->setMenu(m_exportMenu);
    m_ui->btnExport->setPopupMode(QToolButton::MenuButtonPopup);

    m_exportAsPdfAction = new KisAction(i18nc("Export storyboard as PDF", "Export as PDF"), m_exportMenu);
    m_exportMenu->addAction(m_exportAsPdfAction);

    m_exportAsSvgAction = new KisAction(i18nc("Export storyboard as SVG", "Export as SVG"));
    m_exportMenu->addAction(m_exportAsSvgAction);
    connect(m_exportAsPdfAction, SIGNAL(triggered()), this, SLOT(slotExportAsPdf()));
    connect(m_exportAsSvgAction, SIGNAL(triggered()), this, SLOT(slotExportAsSvg()));

    m_ui->btnComment->setMenu(m_commentMenu);
    m_ui->btnComment->setPopupMode(QToolButton::MenuButtonPopup);

    m_lockAction = new KisAction(KisIconUtils::loadIcon("unlocked"),
                                i18nc("Freeze keyframe positions and ignore storyboard adjustments", "Freeze Keyframe Data"), m_ui->btnLock);
    m_lockAction->setCheckable(true);
    m_ui->btnLock->setDefaultAction(m_lockAction);
    m_ui->btnLock->setIconSize(QSize(22, 22));
    connect(m_lockAction, SIGNAL(toggled(bool)), this, SLOT(slotLockClicked(bool)));

    m_ui->btnArrange->setMenu(m_arrangeMenu);
    m_ui->btnArrange->setPopupMode(QToolButton::InstantPopup);
    m_ui->btnArrange->setIcon(KisIconUtils::loadIcon("hamburger_menu_dots"));
    m_ui->btnArrange->setIconSize(QSize(22, 22));

    m_modeGroup = m_arrangeMenu->getModeGroup();
    m_viewGroup = m_arrangeMenu->getViewGroup();

    connect(m_modeGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(slotModeChanged(QAbstractButton*)));
    connect(m_viewGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(slotViewChanged(QAbstractButton*)));

    m_storyboardDelegate->setView(m_ui->listView);
    m_storyboardModel->setView(m_ui->listView);
    m_ui->listView->setModel(m_storyboardModel);
    m_ui->listView->setItemDelegate(m_storyboardDelegate);

    m_storyboardModel->setCommentModel(m_commentModel);

    m_modeGroup->button(Mode::Grid)->click();
    m_viewGroup->button(View::All)->click();

    setEnabled(false);
}

StoryboardDockerDock::~StoryboardDockerDock()
{
    delete m_commentModel;
    delete m_storyboardModel;
    delete m_storyboardDelegate;
}

void StoryboardDockerDock::setCanvas(KoCanvasBase *canvas)
{
    if (m_canvas == canvas) {
        return;
    }

    if (m_canvas) {
        disconnect(m_storyboardModel, SIGNAL(sigStoryboardItemListChanged()), this, SLOT(slotUpdateDocumentList()));
        disconnect(m_commentModel, SIGNAL(sigCommentListChanged()), this, SLOT(slotUpdateDocumentList()));
        disconnect(m_canvas->imageView()->document(), SIGNAL(sigStoryboardItemListChanged()), this, SLOT(slotUpdateStoryboardModelList()));
        disconnect(m_canvas->imageView()->document(), SIGNAL(sigStoryboardItemListChanged()), this, SLOT(slotUpdateCommentModelList()));

        //update the lists in KisDocument and empty storyboardModel's list and commentModel's list
        slotUpdateDocumentList();
        m_storyboardModel->resetData(StoryboardItemList());
        m_commentModel->resetData(QVector<StoryboardComment>());
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    setEnabled(m_canvas != 0);

    if (m_canvas && m_canvas->image()) {
        //sync data between KisDocument and models
        slotUpdateStoryboardModelList();
        slotUpdateCommentModelList();

        connect(m_storyboardModel, SIGNAL(sigStoryboardItemListChanged()), SLOT(slotUpdateDocumentList()), Qt::UniqueConnection);
        connect(m_commentModel, SIGNAL(sigCommentListChanged()), SLOT(slotUpdateDocumentList()), Qt::UniqueConnection);
        connect(m_canvas->imageView()->document(), SIGNAL(sigStoryboardItemListChanged()), this, SLOT(slotUpdateStoryboardModelList()), Qt::UniqueConnection);
        connect(m_canvas->imageView()->document(), SIGNAL(sigStoryboardCommentListChanged()), this, SLOT(slotUpdateCommentModelList()), Qt::UniqueConnection);

        m_storyboardModel->setImage(m_canvas->image());
        m_storyboardDelegate->setImageSize(m_canvas->image()->size());
        connect(m_canvas->image(), SIGNAL(sigAboutToBeDeleted()), SLOT(notifyImageDeleted()), Qt::UniqueConnection);

        if (m_nodeManager) {
            m_storyboardModel->slotSetActiveNode(m_nodeManager->activeNode());
        }
    }
}

void StoryboardDockerDock::unsetCanvas()
{
    setCanvas(0);
}

void StoryboardDockerDock::setViewManager(KisViewManager* kisview)
{
    m_nodeManager = kisview->nodeManager();
    if (m_nodeManager) {
        connect(m_nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)), m_storyboardModel, SLOT(slotSetActiveNode(KisNodeSP)));
    }
}

void StoryboardDockerDock::notifyImageDeleted()
{
    //if there is no image
    if (!m_canvas || !m_canvas->image()){
        m_storyboardModel->setImage(0);
    }
}

void StoryboardDockerDock::slotUpdateDocumentList()
{
    m_canvas->imageView()->document()->setStoryboardItemList(m_storyboardModel->getData());
    m_canvas->imageView()->document()->setStoryboardCommentList(m_commentModel->getData());
}

void StoryboardDockerDock::slotUpdateStoryboardModelList()
{
    m_storyboardModel->resetData(m_canvas->imageView()->document()->getStoryboardItemList());
}

void StoryboardDockerDock::slotUpdateCommentModelList()
{
    m_commentModel->resetData(m_canvas->imageView()->document()->getStoryboardCommentsList());
}

void StoryboardDockerDock::slotExportAsPdf()
{
    slotExport(ExportFormat::PDF);
}

void StoryboardDockerDock::slotExportAsSvg()
{
    slotExport(ExportFormat::SVG);
}

void StoryboardDockerDock::slotExport(ExportFormat format)
{
    KisTimeSpan span = m_canvas->image()->animationInterface()->fullClipRange();
    DlgExportStoryboard dlg(format, span);

    if (dlg.exec() == QDialog::Accepted) {
        dlg.hide();
        QApplication::setOverrideCursor(Qt::WaitCursor);

        QVector<QRectF> layoutCellRects;
        QPrinter printer(QPrinter::HighResolution);

        //getting rectangles to paint panels in
        bool layoutSpecifiedBySvg = dlg.layoutSpecifiedBySvgFile();
        if (layoutSpecifiedBySvg) {
            QString svgFileName = dlg.layoutSvgFile();

            layoutCellRects = getLayoutCellRects(svgFileName, &printer);
        }
        else {
            int rows = dlg.rows();
            int columns = dlg.columns();
            printer.setOutputFileName(dlg.saveFileName());
            printer.setPageSize(dlg.pageSize());
            printer.setPageOrientation(dlg.pageOrientation());

            layoutCellRects = getLayoutCellRects(rows, columns, printer.pageRect());
        }

        //getting the range of items to render
        int firstItemFrame = dlg.firstItem();
        QModelIndex firstIndex = m_storyboardModel->lastIndexBeforeFrame(firstItemFrame);
        if (!firstIndex.isValid()) {
            firstIndex = m_storyboardModel->index(0,0);
        }
        else {
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
            firstIndex = firstIndex.siblingAtRow(firstIndex.row() + 1);
#else
            firstIndex = firstIndex.sibling(firstIndex.row() + 1, 0);
#endif
        }

        int lastItemFrame = dlg.lastItem();
        QModelIndex lastIndex  = m_storyboardModel->indexFromFrame(lastItemFrame);
        if (!lastIndex.isValid()) {
            lastIndex = m_storyboardModel->lastIndexBeforeFrame(lastItemFrame);
        }
        if (!lastIndex.isValid()) {
            lastIndex = m_storyboardModel->index(0, 0);
        }

        if (!firstIndex.isValid() || !lastIndex.isValid()) {
            QMessageBox::warning((QWidget*)(&dlg), i18nc("@title:window", "Krita"), i18n("Please enter correct range. There are no panels in the range of frames provided."));
            QApplication::restoreOverrideCursor();
            return;
        }
        int firstItemRow = firstIndex.row();
        int lastItemRow = lastIndex.row();
        qDebug() << "first" <<firstItemRow <<" last "<<lastItemRow;

        int numItems = lastItemRow - firstItemRow + 1;
        if (numItems <= 0) {
            QMessageBox::warning((QWidget*)(&dlg), i18nc("@title:window", "Krita"), i18n("Please enter correct range. There are no panels in the range of frames provided."));
            QApplication::restoreOverrideCursor();
            return;
        }



        //exporting
        if (layoutCellRects.size() == 0) {
                qDebug()<<"0 rects";
        }
        else {
            QPainter p;
            QSvgGenerator *generator;

            if (dlg.format() == ExportFormat::SVG) {
                generator = new QSvgGenerator();
                generator->setFileName(dlg.saveFileName() + "/" + dlg.svgFileBaseName() + "0.svg");
                QSize sz = printer.pageRect().size();
                generator->setSize(sz);
                generator->setViewBox(QRect(0, 0, sz.width(), sz.height()));
                generator->setResolution(printer.resolution());

                p.begin(generator);
            }
            else {
                printer.setOutputFileName(dlg.saveFileName());
                printer.setOutputFormat(QPrinter::PdfFormat);

                p.begin(&printer);
            }

            QFont font = p.font();
            font.setPointSize(dlg.fontSize());
            p.setFont(font);
            StoryboardItemList list = m_storyboardModel->getData();

            for (int i = 0; i < numItems; i++) {
                if (i % layoutCellRects.size() == 0 && i != 0) {
                    if (dlg.format() == ExportFormat::SVG) {
                        p.end();
                        p.eraseRect(printer.pageRect());
                        delete generator;

                        generator = new QSvgGenerator();
                        generator->setFileName(dlg.saveFileName() + "/" + dlg.svgFileBaseName() + QString::number(i / layoutCellRects.size()) + ".svg");
                        QSize sz = printer.pageRect().size();
                        generator->setSize(sz);
                        generator->setViewBox(QRect(0, 0, sz.width(), sz.height()));
                        generator->setResolution(printer.resolution());
                        p.begin(generator);
                    }
                    else {
                        printer.newPage();
                    }
                }
                QRectF cellRect = layoutCellRects.at(i % layoutCellRects.size());

                //draw the cell rectangle
                QPen pen(QColor(1, 0, 0));
                pen.setWidth(5);
                p.setPen(pen);

                ThumbnailData data = qvariant_cast<ThumbnailData>(list.at(i + firstItemRow)->child(StoryboardItem::FrameNumber)->data());
                QPixmap pxmp = qvariant_cast<QPixmap>(data.pixmap);

                //get the thumbnail rectangle and draw it with content
                float scale = qMin(cellRect.width() / pxmp.rect().width(), (cellRect.height() - p.fontMetrics().height()) /  pxmp.rect().height());
                QRectF thumbRect = cellRect;
                thumbRect.setSize(scale * pxmp.rect().size());

                thumbRect.moveTop(thumbRect.top() + p.fontMetrics().height());
                p.drawRect(thumbRect);

                thumbRect.setSize(thumbRect.size() - QSize(30,30));
                thumbRect.moveTopLeft(thumbRect.topLeft() + QPointF(15,15));
                p.drawPixmap(thumbRect, pxmp, pxmp.rect());

                //get the panelInfo rect and draw panel name and duration
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
                int numericFontWidth = p.fontMetrics().horizontalAdvance("0");
#else
                int numericFontWidth = p.fontMetrics().width("0");
#endif

                QRectF panelInfoRect = cellRect;
                panelInfoRect.setHeight(p.fontMetrics().height());
                panelInfoRect.setWidth((scale * pxmp.rect().size()).width() - 6 * numericFontWidth);

                QString str = list.at(i + firstItemRow)->child(StoryboardItem::ItemName)->data().toString();
                p.drawRect(panelInfoRect);
                QRectF boundRect = panelInfoRect;
                p.drawText(panelInfoRect, Qt::AlignLeft | Qt::AlignVCenter, str, &boundRect);

                //get the duration rect and draw duration
                QRectF durationRect = panelInfoRect;
                durationRect.setWidth(6 * numericFontWidth);
                durationRect.moveLeft(panelInfoRect.right());

                QString duration = QString::number(list.at(i + firstItemRow)->child(StoryboardItem::DurationSecond)->data().toInt());
                duration +=i18nc("suffix in spin box in storyboard that means 'seconds'", "s");
                duration += QString::number(list.at(i + firstItemRow)->child(StoryboardItem::DurationFrame)->data().toInt());
                duration +=i18nc("suffix in spin box in storyboard that means 'frames'", "f");

                boundRect = durationRect;
                boundRect.setSize(boundRect.size() - QSize(10,10));
                p.drawRect(durationRect);
                p.drawText(durationRect, Qt::AlignCenter, duration, &boundRect);

                //if the comments are to be drawn below thumbnail
                QTextDocument doc;
                doc.setDocumentMargin(0);
                doc.setDefaultFont(p.font());
                QVector<StoryboardComment> comments = m_commentModel->getData();
                int numComments = comments.size();
                QString comment;
                for (int j = 0; j < numComments; j++) {
                    comment += "<p><b>" + comments.at(j).name + "</b>"; // if arrange options are used check for visibility
                    comment += " : " + qvariant_cast<CommentBox>(list.at(i + firstItemRow)->child(StoryboardItem::Comments + j)->data()).content.toString() + "</p>";
                }

                doc.setHtml(comment);
                doc.setTextWidth(cellRect.width());

                QRectF clipRect = cellRect;
                clipRect.setTop(thumbRect.bottom() + 15);
                QRectF commentRect = clipRect;
                clipRect.moveTopLeft(QPoint(0,0));
                clipRect.setWidth(thumbRect.width());

                if (clipRect.height() > 10) {
                    p.drawRect(commentRect);
                }

                QAbstractTextDocumentLayout::PaintContext ctx;
                ctx.palette.setColor(QPalette::Text, p.pen().color());
                ctx.clip = clipRect;

                //draw the comments
                p.save();
                p.translate(thumbRect.bottomLeft());
                doc.documentLayout()->draw( &p, ctx);
                p.restore();

                if (commentRect.height() < doc.size().height()) {
                    QRectF eRect(QPointF(commentRect.topLeft()), doc.size());
                    eRect.setTop(cellRect.bottom() + 20);
                    p.eraseRect(eRect);
                }
            }
            p.end();
        }
    }
    QApplication::restoreOverrideCursor();
}

void StoryboardDockerDock::slotLockClicked(bool isLocked){
    if (isLocked) {
        m_lockAction->setIcon(KisIconUtils::loadIcon("locked"));
        m_storyboardModel->setLocked(true);
    }
    else {
        m_lockAction->setIcon(KisIconUtils::loadIcon("unlocked"));
        m_storyboardModel->setLocked(false);
    }
}

void StoryboardDockerDock::slotModeChanged(QAbstractButton* button)
{
    int mode = m_modeGroup->id(button);
    if (mode == Mode::Column) {
        m_ui->listView->setFlow(QListView::LeftToRight);
        m_ui->listView->setWrapping(false);
        m_ui->listView->setItemOrientation(Qt::Vertical);
        m_viewGroup->button(View::CommentsOnly)->setEnabled(true);
    }
    else if (mode == Mode::Row) {
        m_ui->listView->setFlow(QListView::TopToBottom);
        m_ui->listView->setWrapping(false);
        m_ui->listView->setItemOrientation(Qt::Horizontal);
        m_viewGroup->button(View::CommentsOnly)->setEnabled(false);           //disable the comments only view
    }
    else if (mode == Mode::Grid) {
        m_ui->listView->setFlow(QListView::LeftToRight);
        m_ui->listView->setWrapping(true);
        m_ui->listView->setItemOrientation(Qt::Vertical);
        m_viewGroup->button(View::CommentsOnly)->setEnabled(true);
    }
    m_storyboardModel->layoutChanged();
}

void StoryboardDockerDock::slotViewChanged(QAbstractButton* button)
{
    int view = m_viewGroup->id(button);
    if (view == View::All) {
        m_ui->listView->setCommentVisibility(true);
        m_ui->listView->setThumbnailVisibility(true);
        m_modeGroup->button(Mode::Row)->setEnabled(true);
    }
    else if (view == View::ThumbnailsOnly) {
        m_ui->listView->setCommentVisibility(false);
        m_ui->listView->setThumbnailVisibility(true);
        m_modeGroup->button(Mode::Row)->setEnabled(true);
    }

    else if (view == View::CommentsOnly) {
        m_ui->listView->setCommentVisibility(true);
        m_ui->listView->setThumbnailVisibility(false);
        m_modeGroup->button(Mode::Row)->setEnabled(false);               //disable the row mode
    }
    m_storyboardModel->layoutChanged();
}

QVector<QRectF> StoryboardDockerDock::getLayoutCellRects(int rows, int columns, QRectF pageRect)
{
    QSizeF pageSize = pageRect.size();
    QRectF border = pageRect;
    QSizeF cellSize(pageSize.width() / columns, pageSize.height() / rows);
    QVector<QRectF> rectVec;

    for (int row = 0; row < rows; row++) {

        QRectF cellRect = border;
        cellRect.moveTop(border.top() + row * cellSize.height());
        cellRect.setSize(cellSize - QSize(200,200));
        for (int column = 0; column < columns; column++) {
            cellRect.moveLeft(border.left() + column * cellSize.width());
            cellRect.setSize(cellSize * 0.9);
            rectVec.push_back(cellRect);
        }
    }
    return rectVec;
}

QVector<QRectF> StoryboardDockerDock::getLayoutCellRects(QString layoutSvgFileName, QPrinter *printer)
{
    QVector<QRectF> rectVec;
    QDomDocument svgDoc;

    QFile f(layoutSvgFileName);
    if (!f.open(QIODevice::ReadOnly ))
    {
        qDebug()<<"svg layout file didn't open";
        return rectVec;
    }

    svgDoc.setContent(&f);
    f.close();

    QDomElement eroot = svgDoc.documentElement();
    QString Type = eroot.tagName();

    QStringList lst = eroot.attribute("viewBox").split(" ");
    QSizeF sizeMM(lst.at(2).toDouble(), lst.at(3).toDouble());
    printer->setPageSizeMM(sizeMM);
    QSizeF size = printer->pageRect().size();
    double scaleFac = size.width() / sizeMM.width();

    QDomNodeList  nodeList = svgDoc.elementsByTagName("rect");
    for(int i = 0; i < nodeList.size(); i++) {
        QDomNode node = nodeList.at(i);
        QDomNamedNodeMap attrMap = node.attributes();

        double x = scaleFac * attrMap.namedItem("x").nodeValue().toDouble();
        double y = scaleFac * attrMap.namedItem("y").nodeValue().toDouble();
        double width = scaleFac * attrMap.namedItem("width").nodeValue().toDouble();
        double height = scaleFac * attrMap.namedItem("height").nodeValue().toDouble();
        rectVec.append(QRectF(x, y, width, height));
    }

    std::sort(rectVec.begin(), rectVec.end(),
                    [](const QRectF & a, const QRectF & b) -> bool
                    {
                        if (a.x() == b.x()) {
                            return a.x() < b.x();
                        }
                        return a.y() < b.y();
                    });

    return rectVec;
}


#include "StoryboardDockerDock.moc"
