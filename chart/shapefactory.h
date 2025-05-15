#ifndef SHAPEFACTORY_H
#define SHAPEFACTORY_H

#include <QString>
#include <QMap>
#include <functional>

class Shape; 

class ShapeFactory {
public:
    static ShapeFactory& instance();
    void registerShape(const QString& shapeType, 
                      std::function<Shape*(const int&)> creator);
    Shape* createShape(const QString& shapeType, const int& basis);
    
    // 获取所有已注册的形状类型
    QStringList availableShapes() const;

private:
    ShapeFactory() {}
    QMap<QString, std::function<Shape*(const int&)>> m_creators;
};

#endif // SHAPEFACTORY_H