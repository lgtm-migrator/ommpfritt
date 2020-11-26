#include "objects/line.h"
#include "properties/boolproperty.h"
#include "properties/floatproperty.h"
#include <QObject>

namespace omm
{
class Style;

Line::Line(Scene* scene) : Object(scene)
{
  static const auto category = QObject::tr("Line");
  create_property<FloatProperty>(LENGTH_PROPERTY_KEY, 100.0)
      .set_label(QObject::tr("length"))
      .set_category(category);
  create_property<FloatProperty>(ANGLE_PROPERTY_KEY, 0)
      .set_multiplier(M_180_PI)
      .set_label(QObject::tr("angle"))
      .set_category(category);
  create_property<BoolProperty>(CENTER_PROPERTY_KEY, true)
      .set_label(QObject::tr("center"))
      .set_category(category);
  update();
}

QString Line::type() const
{
  return TYPE;
}

Flag Line::flags() const
{
  return Object::flags() | Flag::Convertible;
}

Geom::PathVector Line::paths() const
{
  const auto length = property(LENGTH_PROPERTY_KEY)->value<double>();
  const auto angle = property(ANGLE_PROPERTY_KEY)->value<double>();
  const auto centered = property(CENTER_PROPERTY_KEY)->value<bool>();
  const PolarCoordinates a(angle, centered ? -length / 2.0 : 0.0);
  const PolarCoordinates b(angle, centered ? length / 2.0 : length);
  std::vector<Point> segment = {Point(a.to_cartesian()), Point(b.to_cartesian())};
  return segments_to_path_vector({segment}, false);
}

void Line::on_property_value_changed(Property* property)
{
  if (property == this->property(LENGTH_PROPERTY_KEY)
      || property == this->property(ANGLE_PROPERTY_KEY)
      || property == this->property(CENTER_PROPERTY_KEY)) {
    update();
  } else {
    Object::on_property_value_changed(property);
  }
}

}  // namespace omm
