#ifndef KISPOPUPWIDGETINTERFACE_H
#define KISPOPUPWIDGETINTERFACE_H

#include <QWidget>
#include <QGridLayout>

#include "kis_debug.h"
#include "kis_assert.h"

#include "KoCanvasBase.h"
#include "input/kis_input_manager.h"

/**
 * @brief The PopupWidgetInterface class
 */
struct KisPopupWidgetInterface {
    virtual ~KisPopupWidgetInterface() {}
    virtual void popup(const QPoint& position) = 0;
};


/**
 * @brief The PopupWidget class
 */
class KisPopupWidget : public QWidget, public KisPopupWidgetInterface
{
public:
    KisPopupWidget(QWidget* toPopup, KoCanvasBase* canvas)
        : QWidget(canvas->canvasWidget())
    {
        KIS_ASSERT(toPopup);

        m_toPopup = toPopup;
        m_toPopup->setParent(this);

        setLayout(new QGridLayout());
        layout()->addWidget(m_toPopup);

        setAutoFillBackground(true);
    }

    void popup(const QPoint& position) override {
        setVisible(true);
        adjustPopupLayout(position);
    }

    void adjustPopupLayout(const QPoint& position) {
        if (isVisible() && parentWidget())  {
            const float widgetMargin = -20.0f;
            const QRect fitRect = kisGrowRect(parentWidget()->rect(), widgetMargin);
            const QPoint paletteCenterOffset(sizeHint().width() / 2, sizeHint().height() / 2);

            QRect paletteRect = rect();

            paletteRect.moveTo(position - paletteCenterOffset);

            paletteRect = kisEnsureInRect(paletteRect, fitRect);
            move(paletteRect.topLeft());
        }
    }

    QSize sizeHint() const override {
        KIS_ASSERT(m_toPopup);
        return m_toPopup->sizeHint();
    }

private:
    QWidget* m_toPopup;
};

#endif // KISPOPUPWIDGETINTERFACE_H
