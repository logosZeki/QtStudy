#ifndef SHAPEFACTORY_H
#define SHAPEFACTORY_H

#include "shape/shape.h"
#include <QString>
#include <QMap>
#include <functional>

class ShapeFactory {
public:
    static ShapeFactory& instance();
    
    // 注册形状类型及其创建函数
    void registerShape(const QString& shapeType, 
                      std::function<Shape*(const int&)> creator);
    
    // 根据类型名创建形状实例
    Shape* createShape(const QString& shapeType, const int& basis);
    
    // 获取所有已注册的形状类型
    QStringList availableShapes() const;

private:
    ShapeFactory() {}
    QMap<QString, std::function<Shape*(const int&)>> m_creators;
};

#endif // SHAPEFACTORY_H