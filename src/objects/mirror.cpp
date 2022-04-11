#include "objects/mirror.h"

#include "geometry/vec2.h"
#include "objects/empty.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "path/lib2geomadapter.h"
#include "objects/pathobject.h"
#include "properties/boolproperty.h"
#include "properties/floatproperty.h"
#include "properties/optionproperty.h"
#include "renderers/painter.h"
#include "renderers/painteroptions.h"
#include "scene/disjointpathpointsetforest.h"
#include "scene/scene.h"
#include <QObject>
#include "path/path.h"
#include "geometry/objecttransformation.h"

namespace
{

using namespace omm;






ObjectTransformation get_mirror_t(Mirror::Direction direction)
{
  switch (direction) {
  case Mirror::Direction::Horizontal:
    return ObjectTransformation().scaled(Vec2f(-1.0, 1.0));
  case Mirror::Direction::Vertical:
    return ObjectTransformation().scaled(Vec2f(1.0, -1.0));
  case Mirror::Direction::Both:
    return ObjectTransformation().scaled(Vec2f(-1.0, -1.0));
  default:
    Q_UNREACHABLE();
    return ObjectTransformation();
  }
}

}  // namespace

namespace omm
{

Mirror::Mirror(Scene* scene) : Object(scene)
{
  static constexpr double TOLERANCE_STEP = 0.1;
  static const auto category = QObject::tr("Mirror");
  create_property<OptionProperty>(DIRECTION_PROPERTY_KEY)
      .set_options({QObject::tr("Horizontal"), QObject::tr("Vertical"), QObject::tr("Both")})
      .set_label(QObject::tr("Direction"))
      .set_category(category);
  auto& mode_property = create_property<OptionProperty>(AS_PATH_PROPERTY_KEY);
  mode_property.set_options({QObject::tr("Object"), QObject::tr("Path")})
      .set_label(QObject::tr("Mode"))
      .set_category(category);
  create_property<FloatProperty>(TOLERANCE_PROPERTY_KEY)
      .set_range(0.0, std::numeric_limits<double>::max())
      .set_step(TOLERANCE_STEP)
      .set_label(QObject::tr("Snap tolerance"))
      .set_category(category);
  polish();
}

Mirror::Mirror(const Mirror& other)
    : Object(other), m_reflection(other.m_reflection ? other.m_reflection->clone() : nullptr)
{
  polish();
}

void Mirror::polish()
{
  listen_to_children_changes();
  Mirror::update();
}

void Mirror::draw_object(Painter& renderer,
                         const Style& style,
                         const PainterOptions& options) const
{
  assert(&renderer.scene == scene());
  if (is_active()) {
    auto options_copy = options;
    options_copy.default_style = &style;
    if (m_reflection) {
      m_reflection->draw_recursive(renderer, options_copy);
    }
  } else {
    Object::draw_object(renderer, style, options);
  }
}

BoundingBox Mirror::bounding_box(const ObjectTransformation& transformation) const
{
  if (is_active() && m_reflection) {
    const ObjectTransformation t = transformation.apply(m_reflection->transformation());
    switch (property(AS_PATH_PROPERTY_KEY)->value<Mode>()) {
    case Mode::Path:
      [[fallthrough]];
    case Mode::Object:
      return m_reflection->recursive_bounding_box(t);
    default:
      Q_UNREACHABLE();
    }
  } else {
    return BoundingBox{};
  }
}

QString Mirror::type() const
{
  return TYPE;
}

std::unique_ptr<Object> Mirror::convert(bool& keep_children) const
{
  if (m_draw_children) {
    std::unique_ptr<Object> converted = std::make_unique<Empty>(scene());
    auto reflection = m_reflection->clone();
    reflection->update();
    converted->adopt(std::move(reflection));
    keep_children = true;
    return converted;
  } else {
    keep_children = false;
    auto clone = m_reflection->clone();
    return clone;
  }
}

PathVector Mirror::compute_path_vector() const
{
  if (!is_active()) {
    return {};
  }
  switch (property(AS_PATH_PROPERTY_KEY)->value<Mode>()) {
  case Mode::Path:
    return type_cast<PathObject&>(*m_reflection).geometry();
  case Mode::Object:
    return PathVector{m_reflection->path_vector(), nullptr};
  default:
    Q_UNREACHABLE();
  }
}

void Mirror::update_object_mode()
{
  const auto n_children = this->n_children();
  if (n_children > 0) {
    const auto direction = property(DIRECTION_PROPERTY_KEY)->value<Mirror::Direction>();
    const auto make_reflection = [this](auto* const parent, const Direction direction) {
      auto reflection = this->tree_children().front()->clone();
      reflection->set_virtual_parent(parent);
      reflection->set_transformation(get_mirror_t(direction).apply(reflection->transformation()));
      reflection->update();
      return reflection;
    };
    if (direction == Direction::Both) {
      m_reflection = std::make_unique<Empty>(this->scene());
      m_reflection->adopt(make_reflection(m_reflection.get(), Direction::Horizontal));
      m_reflection->adopt(make_reflection(m_reflection.get(), Direction::Vertical));
      m_reflection->adopt(make_reflection(m_reflection.get(), Direction::Both));
    } else {
      m_reflection = make_reflection(this, direction);
    }
  } else {
    m_reflection.reset();
  }
}

void Mirror::update_path_mode()
{

}

void Mirror::update_property_visibility()
{
  const auto mode = property(AS_PATH_PROPERTY_KEY)->value<Mode>();
  property(TOLERANCE_PROPERTY_KEY)->set_enabledness(mode == Mode::Path);
}

void Mirror::update()
{
  if (is_active()) {
    switch (property(AS_PATH_PROPERTY_KEY)->value<Mode>()) {
    case Mode::Path:
      update_path_mode();
      m_draw_children = false;
      break;
    case Mode::Object:
      update_object_mode();
      m_draw_children = true;
      break;
    default:
      Q_UNREACHABLE();
    }
  } else {
    m_draw_children = true;
  }
  Object::update();
}

void Mirror::on_property_value_changed(Property* property)
{
  if (pmatch(property, {DIRECTION_PROPERTY_KEY, TOLERANCE_PROPERTY_KEY})) {
    update();
  } else if (pmatch(property, {AS_PATH_PROPERTY_KEY})) {
    update_property_visibility();
    update();
  } else {
    Object::on_property_value_changed(property);
  }
}

void Mirror::on_child_added(Object& child)
{
  Object::on_child_added(child);
  update();
}

void Mirror::on_child_removed(Object& child)
{
  Object::on_child_removed(child);
  update();
}

}  // namespace omm
