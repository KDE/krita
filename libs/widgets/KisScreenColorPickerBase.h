#ifndef KISSCREENCOLORPICKERBASE_H
#define KISSCREENCOLORPICKERBASE_H
#include <QWidget>
#include "kritawidgets_export.h"

class KoColor;

class KRITAWIDGETS_EXPORT KisScreenColorPickerBase : public QWidget
{
    Q_OBJECT
public:
    KisScreenColorPickerBase(QWidget *parent = 0) : QWidget(parent) { }
    virtual ~KisScreenColorPickerBase() { }
    /// reloads icon(s) when theme is updated
    virtual void updateIcons() = 0;
Q_SIGNALS:
    void sigNewColorPicked(KoColor);
};

#endif // KISSCREENCOLORPICKERBASE_H
