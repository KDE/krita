#ifndef TABLEOFCONTENTSPREVIEW_H
#define TABLEOFCONTENTSPREVIEW_H

#include <KoZoomHandler.h>
#include <KoInlineTextObjectManager.h>

#include <QFrame>

class TextShape;
class KoTableOfContentsGeneratorInfo;
class KoStyleManager;

class TableOfContentsPreview : public QFrame
{
    Q_OBJECT
public:
    explicit TableOfContentsPreview(QWidget *parent = 0);
    void setStyleManager(KoStyleManager *styleManager);

protected:
    void paintEvent(QPaintEvent *event);

signals:

public slots:
    void updatePreview(KoTableOfContentsGeneratorInfo *info);

private slots:
    void finishedPreviewLayout();

private:
    TextShape *m_textShape;
    QPixmap *m_pm;
    KoZoomHandler m_zoomHandler;
    KoStyleManager *m_styleManager;
    KoInlineTextObjectManager m_itom;

};

#endif // TABLEOFCONTENTSPREVIEW_H
