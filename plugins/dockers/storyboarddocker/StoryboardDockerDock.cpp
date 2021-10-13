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
#include "StoryboardUtils.h"
#include "DlgExportStoryboard.h"
#include "KisAddRemoveStoryboardCommand.h"

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
#include <QSvgRenderer>
#include <QGraphicsSvgItem>
#include <QMessageBox>
#include <QSizePolicy>

#include <klocalizedstring.h>

#include <KisPart.h>
#include <KisViewManager.h>
#include <kis_node_manager.h>
#include <KisDocument.h>
#include <kis_icon.h>
#include <kis_image_animation_interface.h>
#include <kis_time_span.h>
#include <kis_global.h>

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
        m_menuUI->btnDeleteField->setIcon(KisIconUtils::loadIcon("edit-delete"));
        m_menuUI->btnAddField->setIconSize(QSize(16, 16));
        m_menuUI->btnDeleteField->setIconSize(QSize(16, 16));
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


inline QMap<QString, QDomNode> rootItemsInSvg(const QDomDocument &d){
    QMap<QString, QDomNode> nodeMap;
    QDomNodeList svgs = d.elementsByTagName("svg");
    KIS_ASSERT_RECOVER_RETURN_VALUE(svgs.size() > 0, nodeMap);
    QDomNode svg = svgs.at(0);
    QDomNodeList children = svg.toElement().childNodes();
    for (int i = 0; i < children.count(); i++) {
        QString id = children.at(i).toElement().attribute("id");
        if (id.isEmpty())
            continue;

        nodeMap.insert(id, children.at(i));
    }
    return nodeMap;
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
    m_ui->btnExport->setPopupMode(QToolButton::InstantPopup);

    m_exportAsPdfAction = new KisAction(i18nc("Export storyboard as PDF", "Export as PDF"), m_exportMenu);
    m_exportMenu->addAction(m_exportAsPdfAction);

    m_exportAsSvgAction = new KisAction(i18nc("Export storyboard as SVG", "Export as SVG"));
    m_exportMenu->addAction(m_exportAsSvgAction);
    connect(m_exportAsPdfAction, SIGNAL(triggered()), this, SLOT(slotExportAsPdf()));
    connect(m_exportAsSvgAction, SIGNAL(triggered()), this, SLOT(slotExportAsSvg()));

    //Setup dynamic QListView Width Based on Comment Model Columns...
    connect(m_commentModel, &StoryboardCommentModel::sigCommentListChanged, this, &StoryboardDockerDock::slotUpdateMinimumWidth);
    connect(m_storyboardModel.data(), &StoryboardModel::rowsInserted, this, &StoryboardDockerDock::slotUpdateMinimumWidth);

    connect(m_storyboardModel.data(), &StoryboardModel::rowsInserted, this, &StoryboardDockerDock::slotModelChanged);
    connect(m_storyboardModel.data(), &StoryboardModel::rowsRemoved, this, &StoryboardDockerDock::slotModelChanged);

    m_ui->btnComment->setMenu(m_commentMenu);
    m_ui->btnComment->setPopupMode(QToolButton::InstantPopup);

    m_lockAction = new KisAction(KisIconUtils::loadIcon("unlocked"),
                                i18nc("Freeze keyframe positions and ignore storyboard adjustments", "Freeze Keyframe Data"), m_ui->btnLock);
    m_lockAction->setCheckable(true);
    m_ui->btnLock->setDefaultAction(m_lockAction);
    m_ui->btnLock->setIconSize(QSize(16, 16));
    connect(m_lockAction, SIGNAL(toggled(bool)), this, SLOT(slotLockClicked(bool)));

    m_ui->btnArrange->setMenu(m_arrangeMenu);
    m_ui->btnArrange->setPopupMode(QToolButton::InstantPopup);
    m_ui->btnArrange->setIcon(KisIconUtils::loadIcon("view-choose"));
    m_ui->btnArrange->setAutoRaise(true);
    m_ui->btnArrange->setIconSize(QSize(16, 16));

    m_modeGroup = m_arrangeMenu->getModeGroup();
    m_viewGroup = m_arrangeMenu->getViewGroup();

    connect(m_modeGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(slotModeChanged(QAbstractButton*)));
    connect(m_viewGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(slotViewChanged(QAbstractButton*)));

    m_storyboardDelegate->setView(m_ui->sceneView);
    m_storyboardModel->setView(m_ui->sceneView);
    m_ui->sceneView->setModel(m_storyboardModel.data());
    m_ui->sceneView->setItemDelegate(m_storyboardDelegate);

    m_storyboardModel->setCommentModel(m_commentModel);

    m_modeGroup->button(Mode::Row)->click();
    m_viewGroup->button(View::All)->click();

    {   // Footer section...
        QAction* action = new QAction(i18nc("Add new scene as the last storyboard", "Add Scene"), this);
        connect(action, &QAction::triggered, this, [this](bool){
            if (!m_canvas) return;

            QModelIndex currentSelection = m_ui->sceneView->currentIndex();
            if (currentSelection.parent().isValid()) {
                currentSelection = currentSelection.parent();
            }

            m_storyboardModel->insertItem(currentSelection, true);
        });
        action->setIcon(KisIconUtils::loadIcon("list-add"));
        m_ui->btnCreateScene->setAutoRaise(true);
        m_ui->btnCreateScene->setIconSize(QSize(22,22));
        m_ui->btnCreateScene->setDefaultAction(action);

        action = new QAction(i18nc("Remove current scene from storyboards", "Remove Scene"), this);
        connect(action, &QAction::triggered, this, [this](bool){
            if (!m_canvas) return;

            QModelIndex currentSelection = m_ui->sceneView->currentIndex();
            if (currentSelection.parent().isValid()) {
                currentSelection = currentSelection.parent();
            }

            if (currentSelection.isValid()) {
                int row = currentSelection.row();
                KisRemoveStoryboardCommand *command = new KisRemoveStoryboardCommand(row, m_storyboardModel->getData().at(row), m_storyboardModel.data());

                m_storyboardModel->removeItem(currentSelection, command);
                m_storyboardModel->pushUndoCommand(command);
            }
        });
        action->setIcon(KisIconUtils::loadIcon("edit-delete"));
        m_ui->btnDeleteScene->setAutoRaise(true);
        m_ui->btnDeleteScene->setIconSize(QSize(22,22));
        m_ui->btnDeleteScene->setDefaultAction(action);
    }

    setEnabled(false);
}

StoryboardDockerDock::~StoryboardDockerDock()
{
    delete m_commentModel;
    m_storyboardModel.reset();
    delete m_storyboardDelegate;
}

void StoryboardDockerDock::setCanvas(KoCanvasBase *canvas)
{
    if (m_canvas == canvas) {
        return;
    }

    if (m_canvas) {
        disconnect(m_storyboardModel.data(), SIGNAL(sigStoryboardItemListChanged()), this, SLOT(slotUpdateDocumentList()));
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

        connect(m_storyboardModel.data(), SIGNAL(sigStoryboardItemListChanged()), SLOT(slotUpdateDocumentList()), Qt::UniqueConnection);
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

    slotUpdateMinimumWidth();
    slotModelChanged();
}

void StoryboardDockerDock::unsetCanvas()
{
    setCanvas(0);
}

void StoryboardDockerDock::setViewManager(KisViewManager* kisview)
{
    m_nodeManager = kisview->nodeManager();
    if (m_nodeManager) {
        connect(m_nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)), m_storyboardModel.data(), SLOT(slotSetActiveNode(KisNodeSP)));
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
    QFileInfo fileInfo(m_canvas->imageView()->document()->path());
    const QString imageFileName = fileInfo.baseName();
    const int storyboardCount = m_storyboardModel->rowCount();
    KIS_SAFE_ASSERT_RECOVER_RETURN(storyboardCount > 0);
    DlgExportStoryboard dlg(format, m_storyboardModel);

    if (dlg.exec() == QDialog::Accepted) {
        dlg.hide();
        QApplication::setOverrideCursor(Qt::WaitCursor);

        ExportPage layoutPage;
        QPrinter printer(QPrinter::HighResolution);

        // Setup export parameters...
        QPainter painter;
        QSvgGenerator generator;

        QFont font = painter.font();
        font.setPointSize(dlg.fontSize());

        StoryboardItemList storyboardList = m_storyboardModel->getData();

        // Setup per-element layout details...
        bool layoutSpecifiedBySvg = dlg.layoutSpecifiedBySvgFile();
        if (layoutSpecifiedBySvg) {
            QString svgFileName = dlg.layoutSvgFile();
            layoutPage = getPageLayout(svgFileName, &printer);
        }
        else {
            int rows = dlg.rows();
            int columns = dlg.columns();
            printer.setOutputFileName(dlg.saveFileName());
            printer.setPageSize(dlg.pageSize());
            printer.setPageOrientation(dlg.pageOrientation());
            painter.begin(&printer); // We need to begin the painter temporarily for font metrics.
            painter.setFont(font);
            layoutPage = getPageLayout(rows, columns, QRect(0, 0, m_canvas->image()->width(), m_canvas->image()->height()), printer.pageRect(), painter.fontMetrics());
            painter.end(); // End temporary painter begin.
        }

        KIS_SAFE_ASSERT_RECOVER_RETURN(layoutPage.elements.length() > 0);

        // Get a range of items to render. Used to be configurable in an older version but now simplified...
        QModelIndex firstIndex = m_storyboardModel->index(0,0);
        QModelIndex lastIndex  = m_storyboardModel->index(m_storyboardModel->rowCount() - 1, 0);

        if (!firstIndex.isValid() || !lastIndex.isValid()) {
            QMessageBox::warning((QWidget*)(&dlg), i18nc("@title:window", "Krita"), i18n("Please enter correct range. There are no panels in the range of frames provided."));
            QApplication::restoreOverrideCursor();
            return;
        }

        int firstItemRow = firstIndex.row();
        int lastItemRow = lastIndex.row();

        int numBoards = lastItemRow - firstItemRow + 1;
        if (numBoards <= 0) {
            QMessageBox::warning((QWidget*)(&dlg), i18nc("@title:window", "Krita"), i18n("Please enter correct range. There are no panels in the range of frames provided."));
            QApplication::restoreOverrideCursor();
            return;
        }

        if (dlg.format() == ExportFormat::SVG) {
            generator.setFileName(dlg.saveFileName() + "/" + imageFileName + "0.svg");
            QSize sz = printer.pageRect().size();
            generator.setSize(sz);
            generator.setViewBox(QRect(0, 0, sz.width(), sz.height()));
            generator.setResolution(printer.resolution());
            painter.begin(&generator);
            painter.setBrush(QBrush(QColor(255,255,255)));
            painter.drawRect(QRect(0,0, sz.width(), sz.height()));
        }
        else {
            printer.setOutputFileName(dlg.saveFileName());
            printer.setOutputFormat(QPrinter::PdfFormat);
            painter.begin(&printer);
            painter.setFont(font);
            painter.setBackgroundMode(Qt::BGMode::OpaqueMode);
        }

        // Paint boards
        int pageNumber = 1;
        int pageDurationInFrames = 0;

        for (int currentBoard = 0; currentBoard < numBoards; currentBoard++) {
            if (currentBoard % layoutPage.elements.length() == 0) {
                if (dlg.format() == ExportFormat::SVG) {
                    if (currentBoard != 0) {
                        painter.end();
                        painter.eraseRect(printer.pageRect());
                        generator.setFileName(dlg.saveFileName() + "/" + imageFileName + QString::number(currentBoard / layoutPage.elements.length()) + ".svg");
                        QSize sz = printer.pageRect().size();
                        generator.setSize(sz);
                        generator.setViewBox(QRect(0, 0, sz.width(), sz.height()));
                        generator.setResolution(printer.resolution());
                        painter.begin(&generator);
                    }
                }
                else {
                    if (currentBoard != 0 ) { // New page!
                        printer.newPage();

                        pageNumber++;
                        pageDurationInFrames = 0;
                    }

                    if(layoutPage.svg) {
                        QMap<QString, QDomNode> groups = rootItemsInSvg(layoutPage.svg.value());
                        if (groups.contains("overlay")) {
                            QMapIterator<QString, QDomNode> iter(groups);
                            while(iter.hasNext()) {
                                iter.next();
                                if (iter.key() == "overlay" || iter.key() == "layout") {
                                    iter.value().toElement().setAttribute("display","none");
                                } else {
                                    iter.value().toElement().setAttribute("display","inline");
                                }
                            }
                        }
                        QSvgRenderer renderer(layoutPage.svg->toByteArray());
                        renderer.render(&painter);
                    }
                }
            }

            ThumbnailData data = qvariant_cast<ThumbnailData>(storyboardList.at(currentBoard + firstItemRow)->child(StoryboardItem::FrameNumber)->data());
            QPixmap pxmp = qvariant_cast<QPixmap>(data.pixmap);
            QVector<StoryboardComment> comments = m_commentModel->getData();
            const int numComments = comments.size();

            const ExportPageShot* const layoutShot = &layoutPage.elements[currentBoard % layoutPage.elements.length()];

            QPen pen(QColor(1, 0, 0));
            pen.setWidth(5);
            painter.setPen(pen);

            // Draw image
            if (layoutShot->cutImageRect.has_value()) {
                QRectF imgRect = layoutShot->cutImageRect.value();
                QSizeF resizedImage = QSizeF(pxmp.size()).scaled(layoutShot->cutImageRect->size(), Qt::KeepAspectRatio);
                const int MARGIN = -2;
                resizedImage = QSize(resizedImage.width() + MARGIN * 2, resizedImage.height() + MARGIN * 2);
                imgRect.setSize(resizedImage);
                imgRect.translate((layoutShot->cutImageRect.value().width() - imgRect.size().width()) / 2 - MARGIN,
                                  (layoutShot->cutImageRect->height() - imgRect.size().height()) / 2 - MARGIN);
                painter.drawPixmap(imgRect, pxmp, pxmp.rect());
                painter.drawRect(layoutShot->cutImageRect.value());
            }

            { // Insert shot text elements...
                painter.save();
                painter.setBackgroundMode(Qt::TransparentMode);

                // Draw shot name
                if (layoutShot->cutNameRect.has_value()) {
                    QString str = storyboardList.at(currentBoard + firstItemRow)->child(StoryboardItem::ItemName)->data().toString();
                    painter.drawText(layoutShot->cutNameRect.value().translated(painter.fontMetrics().averageCharWidth() / 2, 0), Qt::AlignLeft | Qt::AlignVCenter, str);

                    if (!layoutPage.svg) {
                        painter.drawRect(layoutShot->cutNameRect.value());
                    }
                }

                // Draw shot number
                if (layoutShot->cutNumberRect.has_value()) {
                    painter.drawText(layoutShot->cutNumberRect.value(), Qt::AlignCenter, QString::number(currentBoard + firstItemRow));

                    if (!layoutPage.svg) {
                        painter.drawRect(layoutShot->cutNumberRect.value());
                    }
                }

                QModelIndex boardIndex = m_storyboardModel->index(currentBoard + firstItemRow, 0);

                const int boardDurationFrames = m_storyboardModel->data(boardIndex, StoryboardModel::TotalSceneDurationInFrames).toInt();
                pageDurationInFrames += boardDurationFrames;

                // Draw shot duration
                if (layoutShot->cutDurationRect.has_value()) {
                    // Split shot duration into tuple of seconds + frames (remainder)..
                    int durationSecondsPart = boardDurationFrames / m_storyboardModel->getFramesPerSecond();
                    int durationFramesPart = boardDurationFrames % m_storyboardModel->getFramesPerSecond();

                    painter.drawText(layoutShot->cutDurationRect.value(), Qt::AlignCenter,
                                     buildDurationString(durationSecondsPart, durationFramesPart));

                    if (!layoutPage.svg) {
                        painter.drawRect(layoutShot->cutDurationRect.value());
                    }
                }

                painter.restore();
            }


            // Draw shot comments
            for (int commentIndex = 0; commentIndex < numComments; commentIndex++) {
                if (!layoutShot->commentRects.contains(comments[commentIndex].name))
                    continue;

                const QString& commentName = comments[commentIndex].name;

                QTextDocument doc;
                doc.setDocumentMargin(0);
                doc.setDefaultFont(painter.font());
                QString comment;
                comment += "<p><b>" + commentName + "</b></p>"; // if arrange options are used check for visibility
                QString originalCommentText = qvariant_cast<CommentBox>(storyboardList.at(currentBoard + firstItemRow)->child(StoryboardItem::Comments + commentIndex)->data()).content.toString();
                originalCommentText = originalCommentText.replace('\n', "</p><p>");
                comment += "<p>&nbsp;" + originalCommentText + "</p>";
                const int MARGIN = painter.fontMetrics().averageCharWidth() / 2;

                doc.setHtml(comment);
                doc.setTextWidth(layoutShot->commentRects[commentName].width() - MARGIN * 2);

                QAbstractTextDocumentLayout::PaintContext ctx;
                ctx.palette.setColor(QPalette::Text, painter.pen().color());

                //draw the comments
                painter.save();
                painter.translate(layoutShot->commentRects[commentName].topLeft() + QPoint(MARGIN, MARGIN));
                painter.setClipRegion(QRegion(0,0,layoutShot->commentRects[commentName].width(), layoutShot->commentRects[commentName].height() - painter.fontMetrics().height()));
                painter.setBackgroundMode(Qt::TransparentMode);
                doc.documentLayout()->draw(&painter, ctx);
                painter.restore();

                painter.drawRect(layoutShot->commentRects[commentName]);
            }

            // Draw overlays after drawing last element on page or after drawing last element in general.
            if ((currentBoard % layoutPage.elements.length()) == (layoutPage.elements.length() - 1) || (currentBoard == numBoards - 1)) {

                painter.save();
                painter.setBackgroundMode(Qt::TransparentMode);

                if (layoutPage.pageNumberRect) {
                    painter.drawText(layoutPage.pageNumberRect.value(), Qt::AlignCenter, QString::number(pageNumber));
                }

                if (layoutPage.pageTimeRect) {
                    int pageDurationSecondsPart = pageDurationInFrames / m_storyboardModel->getFramesPerSecond();
                    int pageDurationFramesPart = pageDurationInFrames % m_storyboardModel->getFramesPerSecond();

                    painter.drawText(layoutPage.pageTimeRect.value(), Qt::AlignCenter, buildDurationString(pageDurationSecondsPart, pageDurationFramesPart));
                }

                if (layoutPage.svg) {
                    QMap<QString, QDomNode> groups = rootItemsInSvg(layoutPage.svg.value());
                    if (groups.contains("overlay")) {
                        QMapIterator<QString, QDomNode> iter(groups);
                        while(iter.hasNext()) {
                            iter.next();
                            if (iter.key() == "overlay") {
                                iter.value().toElement().setAttribute("display","inline");
                            } else {
                                iter.value().toElement().setAttribute("display","none");
                            }
                        }
                    }
                    QSvgRenderer renderer(layoutPage.svg->toByteArray());
                    renderer.render(&painter);
                }

                painter.restore();
            }
        }
        painter.end();
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
        m_ui->sceneView->setFlow(QListView::LeftToRight);
        m_ui->sceneView->setWrapping(false);
        m_ui->sceneView->setItemOrientation(Qt::Vertical);
        m_viewGroup->button(View::CommentsOnly)->setEnabled(true);
    }
    else if (mode == Mode::Row) {
        m_ui->sceneView->setFlow(QListView::TopToBottom);
        m_ui->sceneView->setWrapping(false);
        m_ui->sceneView->setItemOrientation(Qt::Horizontal);
        m_viewGroup->button(View::CommentsOnly)->setEnabled(false);           //disable the comments only view
    }
    else if (mode == Mode::Grid) {
        m_ui->sceneView->setFlow(QListView::LeftToRight);
        m_ui->sceneView->setWrapping(true);
        m_ui->sceneView->setItemOrientation(Qt::Vertical);
        m_viewGroup->button(View::CommentsOnly)->setEnabled(true);
    }
    m_storyboardModel->layoutChanged();
}

void StoryboardDockerDock::slotViewChanged(QAbstractButton* button)
{
    int view = m_viewGroup->id(button);
    if (view == View::All) {
        m_ui->sceneView->setCommentVisibility(true);
        m_ui->sceneView->setThumbnailVisibility(true);
        m_modeGroup->button(Mode::Row)->setEnabled(true);
    }
    else if (view == View::ThumbnailsOnly) {
        m_ui->sceneView->setCommentVisibility(false);
        m_ui->sceneView->setThumbnailVisibility(true);
        m_modeGroup->button(Mode::Row)->setEnabled(true);
    }

    else if (view == View::CommentsOnly) {
        m_ui->sceneView->setCommentVisibility(true);
        m_ui->sceneView->setThumbnailVisibility(false);
        m_modeGroup->button(Mode::Row)->setEnabled(false);               //disable the row mode
    }
    m_storyboardModel->layoutChanged();
}

void StoryboardDockerDock::slotUpdateMinimumWidth()
{
    m_ui->sceneView->setMinimumSize(m_ui->sceneView->sizeHint());
}

void StoryboardDockerDock::slotModelChanged()
{
    if (m_storyboardModel) {
        m_ui->btnExport->setDisabled(m_storyboardModel->rowCount() == 0);
    }
}

StoryboardDockerDock::ExportPage StoryboardDockerDock::getPageLayout(int rows, int columns, const QRect& imageSize, const QRect& pageRect, const QFontMetrics& fontMetrics)
{
    QSizeF pageSize = pageRect.size();
    QRectF border = pageRect;
    QSizeF cellSize(pageSize.width() / columns, pageSize.height() / rows);
    QVector<QRectF> rects;

#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
    int numericFontWidth = fontMetrics.horizontalAdvance("0");
#else
    int numericFontWidth = fontMetrics.width("0");
#endif

    for (int row = 0; row < rows; row++) {

        QRectF cellRect = border;
        cellRect.moveTop(border.top() + row * cellSize.height());
        cellRect.setSize(cellSize - QSize(200,200));
        for (int column = 0; column < columns; column++) {
            cellRect.moveLeft(border.left() + column * cellSize.width());
            cellRect.setSize(cellSize * 0.9);
            rects.push_back(cellRect);
        }
    }

    QVector<ExportPageShot> elements;

    for (int i = 0; i < rects.length(); i++) {
        QRectF& cellRect = rects[i];

        const bool horizontal = cellRect.width() > cellRect.height(); // Determine general image / text flow orientation.
        ExportPageShot layout;

        QVector<StoryboardComment> comments = m_commentModel->getData();
        const int numComments = comments.size();

        if (horizontal) {
            QRectF sourceRect = cellRect;
            layout.cutDurationRect = kisTrimTop(fontMetrics.height() * 1.5, sourceRect);
            layout.cutNameRect = kisTrimLeft(layout.cutDurationRect.value().width() - numericFontWidth * 6, layout.cutDurationRect.value());

            const int imageWidth = sourceRect.height() * static_cast<qreal>(imageSize.width()) / static_cast<qreal>(imageSize.height());
            layout.cutImageRect = kisTrimLeft(imageWidth, sourceRect);
            const float commentWidth = sourceRect.width() / numComments;
            if (commentWidth > 100) {
                for (int i = 0; i < numComments; i++) {
                    QRectF rect = kisTrimLeft(commentWidth, sourceRect);
                    layout.commentRects.insert(comments[i].name, rect);
                }
            }
        } else {
            QRectF sourceRect = cellRect;
            layout.cutDurationRect = kisTrimTop(fontMetrics.height() * 1.5, sourceRect);
            layout.cutNameRect = kisTrimLeft(layout.cutDurationRect.value().width() - numericFontWidth * 6, layout.cutDurationRect.value());

            const int imageHeight = sourceRect.width() * static_cast<qreal>(imageSize.height()) / static_cast<qreal>(imageSize.width());
            layout.cutImageRect = kisTrimTop(imageHeight, sourceRect);
            const float commentHeight = sourceRect.height() / numComments;
            if (commentHeight > 200) {
                for (int i = 0; i < numComments; i++) {
                    QRectF rect = kisTrimTop(commentHeight, sourceRect);
                    layout.commentRects.insert(comments[i].name, rect);
                }
            }
        }

        elements.push_back(layout);
    }

    ExportPage layout;
    layout.elements = elements;
    return layout;
}

StoryboardDockerDock::ExportPage StoryboardDockerDock::getPageLayout(QString layoutSvgFileName, QPrinter *printer)
{
    QDomDocument svgDoc;
    ExportPage page;
    QVector<ExportPageShot> elements;

    // Load DOM from file...
    QFile f(layoutSvgFileName);
    if (!f.open(QIODevice::ReadOnly ))
    {
        qDebug()<<"svg layout file didn't open";
        return page;
    }
    svgDoc.setContent(&f);
    f.close();

    QDomElement eroot = svgDoc.documentElement();

    QStringList lst = eroot.attribute("viewBox").split(" ");
    QSizeF sizeMM(lst.at(2).toDouble(), lst.at(3).toDouble());
    printer->setPageSizeMM(sizeMM);
    QSizeF size = printer->pageRect().size();
    QSizeF scaling = QSizeF( size.width() / sizeMM.width(), size.height() / sizeMM.height());

    QVector<QString> commentLayers;
    Q_FOREACH( StoryboardComment channel, m_commentModel->getData()) {
        commentLayers.push_back(channel.name);
    }

    QMap<int, ExportPageShot> elementMap;

    { // Go through all root-level svg data and preconfigure...
        QMap<QString, QDomNode> groupsMap = rootItemsInSvg(svgDoc);

        KIS_ASSERT_RECOVER_RETURN_VALUE(groupsMap.contains("layout"), page);

        groupsMap["layout"].toElement().setAttribute("display", "none");
        if (groupsMap.contains("overlay")) {
            groupsMap["overlay"].toElement().setAttribute("display", "nonde");
        }

        QDomNodeList  nodeList = groupsMap["layout"].toElement().elementsByTagName("rect");
        for(int i = 0; i < nodeList.size(); i++) {
            QDomNode node = nodeList.at(i);
            QDomNamedNodeMap attrMap = node.attributes();

            for (int j = 0; j < attrMap.length(); j++) {
                QDomAttr attribute = attrMap.item(j).toAttr();
                QString afterNamespace = attribute.name().split(":").last();

                auto isValidLabel = [&](QString label, int& index) -> bool {
                    ENTER_FUNCTION() << ppVar(label) << ppVar(attribute.value());
                    if (attribute.value().startsWith(label)) {
                        if (attribute.value() == label) {
                            index = 0;
                            return true;
                        }

                        QString indexString = attribute.value().remove(0, label.length());
                        bool ok = false;
                        index = indexString.toInt(&ok);
                        if (ok) {
                            if (!elementMap.contains(index))
                                elementMap.insert(index, ExportPageShot());
                            return true;
                        }
                    }
                    return false;
                };

                auto extractRect = [&](boost::optional<QRectF>& to) {
                    double x = scaling.width() * attrMap.namedItem("x").nodeValue().toDouble();
                    double y = scaling.height() * attrMap.namedItem("y").nodeValue().toDouble();
                    double width = scaling.width() * attrMap.namedItem("width").nodeValue().toDouble();
                    double height = scaling.height() * attrMap.namedItem("height").nodeValue().toDouble();
                    to = QRectF(x,y, width, height);
                };

                if (afterNamespace == "label") {
                    int index = 0;
                    if (isValidLabel("image", index)) {
                        extractRect(elementMap[index].cutImageRect);
                    } else if (isValidLabel("time",index)) {
                        extractRect(elementMap[index].cutDurationRect);
                    } else if (isValidLabel("name", index)) {
                        extractRect(elementMap[index].cutNameRect);
                    } else if (isValidLabel("shot", index)) {
                        extractRect(elementMap[index].cutNumberRect);
                    } else if (isValidLabel("page-time", index)) {
                        ENTER_FUNCTION();
                        extractRect(page.pageTimeRect);
                    } else if (isValidLabel("page-number", index)) {
                        extractRect(page.pageNumberRect);
                    } else {
                        for(int commentIndex = 0; commentIndex < commentLayers.length(); commentIndex++) {
                            const QString& comment = commentLayers[commentIndex];
                            if (isValidLabel(comment.toLower(), index)) {
                                boost::optional<QRectF> rect;
                                extractRect(rect);
                                if (rect) {
                                    elementMap[index].commentRects.insert(comment, rect.value());
                                }
                            }
                        }
                    }
                }
            }

        }
    }

    // Sort fetched elements and push to array to return...
    QList<int> indices = elementMap.keys();
    std::sort(indices.begin(), indices.end(), [](const int& a, const int& b){
        return a < b;
    });

    Q_FOREACH(const int& index, indices){
        elements.push_back(elementMap[index]);
    }

    page.svg = svgDoc;
    page.elements = elements;
    return page;
}

QString StoryboardDockerDock::buildDurationString(int seconds, int frames)
{
    QString durationString = QString::number(seconds);
    durationString += i18nc("suffix in spin box in storyboard that means 'seconds'", "s");
    durationString += "+";
    durationString += QString::number(frames);
    durationString += i18nc("suffix in spin box in storyboard that means 'frames'", "f");
    return durationString;
}

#include "StoryboardDockerDock.moc"

