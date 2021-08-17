#ifndef KISCOLLAPSIBLEBUTTONGROUP_H
#define KISCOLLAPSIBLEBUTTONGROUP_H

#include <QObject>
#include <QWidget>
#include <QScopedPointer>

#include "kritaui_export.h"

class QToolButton;

class KRITAUI_EXPORT KisCollapsibleButtonGroup : public QWidget
{
    Q_OBJECT
public:
    KisCollapsibleButtonGroup(QWidget *parent = nullptr);
    ~KisCollapsibleButtonGroup() {}

    void setAutoRaise(bool autoRaise);
    bool autoRaise() const;
    void setIconSize(const QSize& size);
    QSize iconSize() const;

    /* setAutoCollapse:
     * Set whether we want to try to automatically collapse based on the size
     * of the current widget. This can be disabled for manual collapsing.
     */
    void setAutoCollapse(bool autoCollapse);
    void setCollapsed(bool collapse);
    bool collapsed() const;

    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;

    QToolButton* addAction(QAction* action);

protected:
    virtual void resizeEvent(class QResizeEvent *event) override;

private:
    struct Private;
    Private* m_d;
};

#endif // KISCOLLAPSIBLEBUTTONGROUP_H
