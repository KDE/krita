/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

#include <klocalizedstring.h>

#include <KisPart.h>
#include <KisViewManager.h>
#include <kis_node_manager.h>
#include <KisDocument.h>
#include <kis_icon.h>
#include <kis_image_animation_interface.h>

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
    CommentMenu(QWidget *parent, CommentModel *m_model)
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
    CommentModel *model;
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
    , m_commentModel(new CommentModel(this))
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
                                i18nc("Lock addition of keyframes to storyboard", "Lock"), m_ui->btnLock);
    m_lockAction->setCheckable(true);
    m_ui->btnLock->setDefaultAction(m_lockAction);
    m_ui->btnLock->setIconSize(QSize(22, 22));
    connect(m_lockAction, SIGNAL(toggled(bool)), this, SLOT(slotLockClicked(bool)));

    m_ui->btnArrange->setMenu(m_arrangeMenu);
    m_ui->btnArrange->setPopupMode(QToolButton::InstantPopup);
    m_ui->btnArrange->setIcon(KisIconUtils::loadIcon("view-choose"));
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
        m_commentModel->resetData(QVector<Comment>());
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
    DlgExportStoryboard dlg(format);

    if (dlg.exec() == QDialog::Accepted) {
        dlg.hide();
        QApplication::setOverrideCursor(Qt::WaitCursor);

        QVector<QRectF> layoutCellRects;
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat );
        printer.setOutputFileName(dlg.saveFileName());
        printer.setPageSize(QPrinter::A3);

        bool layoutSpecifiedBySvg = dlg.exportSvgFile().isEmpty();
        if (layoutSpecifiedBySvg) {
            QString svgFileName = dlg.exportSvgFile();

            //get rects
            layoutCellRects = getLayoutCellRects(svgFileName);
        }
        else {
            int rows = dlg.rows();
            int columns = dlg.columns();
            int firstItemFrame = dlg.firstItem();
            int lastItemFrame = dlg.lastItem();
            PageSize size = dlg.pageSize();

            //get rects
            layoutCellRects = getLayoutCellRects(rows, columns, printer.pageRect());
        }

        if (dlg.format() == ExportFormat::SVG) {
            QApplication::restoreOverrideCursor();
            return;
        }
        else {
            QPainter p(&printer);

            //take font size as input
            QFont font = p.font();
            font.setPointSize(1.5 * font.pointSize());
            p.setFont(font);
            StoryboardItemList list = m_storyboardModel->getData();

            for (int i = 0; i < list.size(); i++) {
                QRectF cellRect = layoutCellRects.at(i);

                //draw the cell rectangle
                p.setPen(QColor(100, 100, 0));
                p.drawRect(cellRect);

                ThumbnailData data = qvariant_cast<ThumbnailData>(list.at(i)->child(StoryboardItem::FrameNumber)->data());
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
                int numericFontWidth = p.fontMetrics().horizontalAdvance("0");
                QRectF panelInfoRect = cellRect;
                panelInfoRect.setHeight(p.fontMetrics().height());
                panelInfoRect.setWidth((scale * pxmp.rect().size()).width() - 6 * numericFontWidth);

                QString str = list.at(i)->child(StoryboardItem::ItemName)->data().toString();
                p.drawRect(panelInfoRect);
                QRectF boundRect = panelInfoRect;
                p.drawText(panelInfoRect, Qt::AlignLeft | Qt::AlignVCenter, str, &boundRect);

                //get the duration rect and draw duration
                QRectF durationRect = panelInfoRect;
                durationRect.setWidth(6 * numericFontWidth);
                durationRect.moveLeft(panelInfoRect.right());

                QString duration = QString::number(list.at(i)->child(StoryboardItem::DurationSecond)->data().toInt());
                duration +=i18nc("suffix in spin box in storyboard that means 'seconds'", "s");
                duration += QString::number(list.at(i)->child(StoryboardItem::DurationFrame)->data().toInt());
                duration +=i18nc("suffix in spin box in storyboard that means 'frames'", "f");

                boundRect = durationRect;
                boundRect.setSize(boundRect.size() - QSize(10,10));
                p.drawRect(durationRect);
                p.drawText(durationRect, Qt::AlignCenter, duration, &boundRect);

                //if the comments are to be drawn below thumbnail
                QTextDocument doc;
                doc.setDocumentMargin(0);
                doc.setDefaultFont(p.font());
                QVector<Comment> comments = m_commentModel->getData();
                int numComments = comments.size();
                QString comment;
                for (int j = 0; j < numComments; j++) {
                    comment += "<p><b>" + comments.at(j).name + "</b>"; // if arrange options are used check for visibility
                    comment += " : " + qvariant_cast<CommentBox>(list.at(i)->child(StoryboardItem::Comments + j)->data()).content.toString() + "</p>";
                }

                doc.setHtml(comment);
                doc.setTextWidth(cellRect.width());

                QRectF clipRect = cellRect;
                clipRect.setTop(thumbRect.bottom() + 15);
                clipRect.moveTopLeft(QPoint(0,0));
                clipRect.setWidth(thumbRect.width());

                QAbstractTextDocumentLayout::PaintContext ctx;
                ctx.palette.setColor(QPalette::Text, p.pen().color());
                ctx.clip = clipRect;

                //draw the comments
                p.save();
                p.translate(thumbRect.bottomLeft());
                doc.documentLayout()->draw( &p, ctx);
                p.restore();

                QRectF eRect = printer.pageRect();
                eRect.setTop(cellRect.bottom() + 2);
                p.eraseRect(eRect);
            }
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
        cellRect.moveTop(border.top() + row * cellRect.height() + 100);
        cellRect.setSize(cellSize - QSize(200,200));
        for (int column = 0; column < columns; column++) {
            cellRect.moveLeft(border.left() + column * (cellRect.width() + 100));
            rectVec.push_back(cellRect);
        }
    }
    return rectVec;
}

QVector<QRectF> StoryboardDockerDock::getLayoutCellRects(QString layoutSvgFileName)
{
    QVector<QRectF> rectVec;
    return rectVec;
}


#include "StoryboardDockerDock.moc"