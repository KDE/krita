#ifndef STYLESCOMBOPREVIEW_H
#define STYLESCOMBOPREVIEW_H

#include <QLineEdit>

class QModelIndex;
class QPixmap;
class QPushButton;
class QSize;
class QString;

class StylesComboPreview : public QLineEdit
{
    Q_OBJECT

    Q_PROPERTY( bool showAddButton READ isAddButtonShown WRITE setAddButtonShown )

public:
    explicit StylesComboPreview(QWidget *parent = 0);
    ~StylesComboPreview();

    QSize availableSize() const;
    QSize addButtonUsedSize() const;
    void setAddButtonShown(bool show);
    bool isAddButtonShown() const;

    void setPreview(QPixmap pixmap);

signals:
    void resized();
    void newStyleRequested(QString name);

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual bool event(QEvent *event);
    virtual void paintEvent(QPaintEvent *event);

private slots:
    void updateAddButtonIcon();
    void addNewStyle();

private:
    void init();
    void updateAddButton();

    bool m_clickInAdd;
    bool m_renamingNewStyle;
    bool m_shouldAddNewStyle;
    bool m_wideEnoughForAdd;

    QPixmap m_stylePreview;

    QPushButton *m_addButton;
//    QWeakPointer<KLineEditStyle> style;

};

#endif // STYLESCOMBOPREVIEW_H
