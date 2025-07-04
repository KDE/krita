#ifndef _KIS_SELECTION_ASSISTANTS_DECORATION_H_
#define _KIS_SELECTION_ASSISTANTS_DECORATION_H_

#include <QPointF>
#include <QColor>

#include "KoPointerEvent.h"
#include "KoSnapGuide.h"
#include "kis_icon_utils.h"
#include "canvas/kis_canvas_decoration.h"
#include "kis_painting_assistant.h"
#include <kritaui_export.h>

class KisView;

class KisSelectionAssistantsDecoration;
typedef KisSharedPtr<KisSelectionAssistantsDecoration> KisSelectionAssistantsDecorationSP;

class KRITAUI_EXPORT KisSelectionAssistantsDecoration
{
    //Q_OBJECT
public:
    KisSelectionAssistantsDecoration();
    ~KisSelectionAssistantsDecoration();
    void drawDecoration(QPainter& gc, const KisCoordinatesConverter *converter);

private:
    struct Private;
    Private* const d;
};

#endif
