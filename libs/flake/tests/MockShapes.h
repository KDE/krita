#include <KoShapeGroup.h>
#include <KoCanvasBase.h>

class MockShape : public KoShape {
    void paint(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
};

class MockContainer : public KoShapeContainer {
    void paintComponent(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
};

class MockGroup : public KoShapeGroup {
    void paintComponent(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
};

class KCommand;
class KoToolProxy;

class MockCanvas : public KoCanvasBase {
public:
    MockCanvas() {}
    ~MockCanvas() {}

    void gridSize(double *, double *) const {}
    bool snapToGrid() const  { return false; }
    void addCommand(KCommand *, bool = true) { }
    KoShapeManager *shapeManager() const  { return 0; }
    void updateCanvas(const QRectF& )  {};
    KoToolProxy * toolProxy() { return 0; }
    KoViewConverter *viewConverter() { return 0; }
    QWidget* canvasWidget() { return 0; }
    KoUnit::Unit unit() { return KoUnit::U_MM; }
};
