#include <KoShapeGroup.h>

class MockShape : public KoShape {
    void paint(QPainter &painter, KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
};

class MockContainer : public KoShapeContainer {
    void paintComponent(QPainter &painter, KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
};

class MockGroup : public KoShapeGroup {
    void paintComponent(QPainter &painter, KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
};
