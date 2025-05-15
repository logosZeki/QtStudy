#include "chart/shapefactory.h"
#include "chart/shape.h"
ShapeFactory& ShapeFactory::instance() {
    static ShapeFactory instance;
    return instance;
}
void ShapeFactory::registerShape(const QString& shapeType, 
                               std::function<Shape*(const int&)> creator) {
    m_creators[shapeType] = creator;
}
Shape* ShapeFactory::createShape(const QString& shapeType, const int& basis) {
    if (m_creators.contains(shapeType)) {
        return m_creators[shapeType](basis);
    }
    return nullptr;
}
QStringList ShapeFactory::availableShapes() const {
    return m_creators.keys();
}
