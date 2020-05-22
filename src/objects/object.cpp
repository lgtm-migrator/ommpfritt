#include "objects/object.h"

#include <cassert>
#include <algorithm>
#include <map>
#include <functional>
#include <QObject>

#include "scene/objecttree.h"
#include "tags/tag.h"
#include "properties/stringproperty.h"
#include "properties/integerproperty.h"
#include "properties/floatproperty.h"
#include "properties/referenceproperty.h"
#include "common.h"
#include "serializers/jsonserializer.h"
#include "renderers/style.h"
#include "tags/styletag.h"
#include "scene/contextes.h"
#include "properties/boolproperty.h"
#include "properties/optionproperty.h"
#include "properties/integervectorproperty.h"
#include "properties/floatvectorproperty.h"
#include "logging.h"
#include "objects/path.h"
#include "scene/scene.h"
#include "scene/messagebox.h"

namespace
{

static constexpr auto CHILDREN_POINTER = "children";
static constexpr auto TAGS_POINTER = "tags";
static constexpr auto TYPE_POINTER = "type";

QPen make_bounding_box_pen()
{
  QPen pen;
  pen.setCosmetic(true);
  pen.setColor(Qt::black);
  pen.setWidth(1.0);
  return pen;
}

}  // namespace

namespace omm
{

QPen Object::m_bounding_box_pen = make_bounding_box_pen();
QBrush Object::m_bounding_box_brush = Qt::NoBrush;

Object::Object(Scene* scene) : PropertyOwner(scene), tags(*this)
{
  static const auto category = QObject::tr("basic");
  create_property<OptionProperty>(VIEWPORT_VISIBILITY_PROPERTY_KEY, 0)
    .set_options({ QObject::tr("default"), QObject::tr("hidden"),
      QObject::tr("visible") })
    .set_label(QObject::tr("visibility (viewport)"))
    .set_category(category);

  create_property<OptionProperty>(VISIBILITY_PROPERTY_KEY, 0)
    .set_options({ QObject::tr("default"), QObject::tr("hidden"),
      QObject::tr("visible") })
    .set_label(QObject::tr("visibility"))
    .set_category(category);

  create_property<BoolProperty>(IS_ACTIVE_PROPERTY_KEY, true)
    .set_label(QObject::tr("active"))
    .set_category(category);

  create_property<StringProperty>(NAME_PROPERTY_KEY, QObject::tr("<unnamed object>"))
    .set_label(QObject::tr("Name"))
    .set_category(category);

  create_property<FloatVectorProperty>(POSITION_PROPERTY_KEY, Vec2f(0.0, 0.0))
    .set_label(QObject::tr("pos"))
    .set_category(category);

  create_property<FloatVectorProperty>(SCALE_PROPERTY_KEY, Vec2f(1.0, 1.0))
    .set_step(Vec2f(0.1, 0.1))
    .set_label(QObject::tr("scale"))
    .set_category(category);

  create_property<FloatProperty>(ROTATION_PROPERTY_KEY, 0.0)
    .set_multiplier(180.0 / M_PI)
    .set_label(QObject::tr("rotation"))
    .set_category(category);

  create_property<FloatProperty>(SHEAR_PROPERTY_KEY, 0.0)
    .set_step(0.01)
    .set_label(QObject::tr("shear"))
    .set_category(category);
}

Object::Object(const Object& other)
  : PropertyOwner(other)
  , TreeElement(other)
  , tags(other.tags, *this)
  , m_draw_children(other.m_draw_children)
  , m_object_tree(other.m_object_tree)
{
  for (Tag* tag : tags.items()) {
    tag->owner = this;
  }
}

Object::~Object()
{
  if (const Scene* scene = this->scene(); scene != nullptr) {
    assert(!::contains(scene->selection(), this));
  }
}

ObjectTransformation Object::transformation() const
{
  return ObjectTransformation(
    property(POSITION_PROPERTY_KEY)->value<Vec2f>(),
    property(SCALE_PROPERTY_KEY)->value<Vec2f>(),
    property(ROTATION_PROPERTY_KEY)->value<double>(),
    property(SHEAR_PROPERTY_KEY)->value<double>()
  );
}

ObjectTransformation Object::global_transformation(Space space) const
{
  if (m_virtual_parent != nullptr) {
    return m_virtual_parent->global_transformation(space).apply(transformation());
  } else if (is_root()) {
    return transformation();
  } else if (space == Space::Scene && tree_parent().is_root()) {
    return transformation();
  } else {
    // TODO caching could gain some speed
    //  invalidate cache if local transformation is set or parent changes
    return tree_parent().global_transformation(space).apply(transformation());
  }
}

void Object::set_transformation(const ObjectTransformation& transformation)
{
  property(POSITION_PROPERTY_KEY)->set(transformation.translation());
  property(SCALE_PROPERTY_KEY)->set(transformation.scaling());
  property(ROTATION_PROPERTY_KEY)->set(transformation.rotation());
  property(SHEAR_PROPERTY_KEY)->set(transformation.shearing());
}

void Object::
set_global_transformation(const ObjectTransformation& global_transformation, Space space)
{
  ObjectTransformation local_transformation;
  if (is_root() || (space == Space::Scene && tree_parent().is_root())) {
    local_transformation = global_transformation;
  } else {
    try {
      local_transformation =
        tree_parent().global_transformation(space).inverted().apply(global_transformation);
    } catch (const std::runtime_error&) {
      assert(false);
    }
  }
  set_transformation(local_transformation);
}


void Object::set_global_axis_transformation( const ObjectTransformation& global_transformation,
                                             Space space )
{
  const auto get_glob_trans = [space](const auto* c) {
    return c->global_transformation(space);
  };
  const auto child_transformations = ::transform<ObjectTransformation>(tree_children(),
                                                                       get_glob_trans);
  set_global_transformation(global_transformation, space);
  for (std::size_t i = 0; i < child_transformations.size(); ++i) {
    tree_children()[i]->set_global_transformation(child_transformations[i], space);
  }
}

void Object::transform(const ObjectTransformation& transformation)
{
  set_transformation(transformation.apply(this->transformation()));
}

void Object::set_virtual_parent(const Object* parent)
{
  m_virtual_parent = parent;
}

std::ostream& operator<<(std::ostream& ostream, const Object& object)
{
  ostream << object.type() << "[" << object.name() << "]";
  return ostream;
}

std::ostream &operator<<(std::ostream &ostream, const Object *object)
{
  if (object != nullptr) {
    ostream << *object;
  } else {
    ostream << "null-Object";
  }
  return ostream;
}

void Object::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  PropertyOwner::serialize(serializer, root);

  const auto children_pointer = make_pointer(root, CHILDREN_POINTER);
  serializer.start_array(n_children(), children_pointer);
  for (size_t i = 0; i < n_children(); ++i) {
    const auto& child = this->tree_child(i);
    const auto child_pointer = make_pointer(children_pointer, i);
    serializer.set_value(child.type(), make_pointer(child_pointer, TYPE_POINTER));
    child.serialize(serializer, child_pointer);
  }
  serializer.end_array();

  const auto tags_pointer = make_pointer(root, TAGS_POINTER);
  serializer.start_array(tags.size(), tags_pointer);
  for (size_t i = 0; i < tags.size(); ++i) {
    const auto& tag = tags.item(i);
    const auto tag_pointer = make_pointer(tags_pointer, i);
    serializer.set_value(tag.type(), make_pointer(tag_pointer, TYPE_POINTER));
    tag.serialize(serializer, tag_pointer);
  }
  serializer.end_array();
}

void Object::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  PropertyOwner::deserialize(deserializer, root);

  const auto children_pointer = make_pointer(root, CHILDREN_POINTER);
  size_t n_children = deserializer.array_size(children_pointer);
  for (size_t i = 0; i < n_children; ++i) {
    const auto child_pointer = make_pointer(children_pointer, i);
    const auto child_type = deserializer.get_string(make_pointer(child_pointer, TYPE_POINTER));
    try {
      auto child = Object::make(child_type, static_cast<Scene*>(scene()));
      child->set_object_tree(scene()->object_tree());
      child->deserialize(deserializer, child_pointer);

      // TODO adopt sets the global transformation which is reverted by setting the local
      //  transformation immediately afterwards. That can be optimized.
      const auto t = child->transformation();
      adopt(std::move(child)).set_transformation(t);
    } catch (std::out_of_range&) {
      const auto message = QObject::tr("Failed to retrieve object type '%1'.") .arg(child_type);
      LERROR << message;
      throw AbstractDeserializer::DeserializeError(message.toStdString());
    }
  }

  const auto tags_pointer = make_pointer(root, TAGS_POINTER);
  size_t n_tags = deserializer.array_size(tags_pointer);
  std::vector<std::unique_ptr<Tag>> tags;
  tags.reserve(n_tags);
  for (size_t i = 0; i < n_tags; ++i) {
    const auto tag_pointer = make_pointer(tags_pointer, i);
    const auto tag_type = deserializer.get_string(make_pointer(tag_pointer, TYPE_POINTER));
    auto tag = Tag::make(tag_type, *this);
    tag->deserialize(deserializer, tag_pointer);
    tags.push_back(std::move(tag));
  }
  this->tags.set(std::move(tags));
}

void Object::draw_recursive(Painter& renderer, Painter::Options options) const
{
  renderer.push_transformation(transformation());
  const bool is_enabled = !!(renderer.category_filter & Painter::Category::Objects);
  if (is_enabled && is_visible(options.device_is_viewport)) {
    // TODO options.styles is overriden before being used. Why not use a local variable instead?
    // Remove the styles field from Painter::Options
    options.styles = find_styles();
    for (auto* style : options.styles) {
      draw_object(renderer, *style, options);
    }
    if (options.styles.size() == 0) {
      draw_object(renderer, *options.default_style, options);
    }

    if (!!(renderer.category_filter & Painter::Category::BoundingBox)) {
      renderer.painter->save();
      renderer.painter->setBrush(Qt::NoBrush);
      renderer.painter->setPen(m_bounding_box_pen);
      renderer.painter->drawRect(bounding_box(ObjectTransformation()));
      renderer.painter->restore();
    }

    if (!!(renderer.category_filter & Painter::Category::Handles)) {
      draw_handles(renderer);
    }
  }

  if (m_draw_children) {
    for (const auto& child : tree_children()) {
      child->draw_recursive(renderer, options);
    }
  }
  renderer.pop_transformation();
}

BoundingBox Object::recursive_bounding_box(const ObjectTransformation& transformation) const
{
  BoundingBox bounding_box;
  if (is_active()) {
    bounding_box = this->bounding_box(transformation);
  }
  for (const auto& child : tree_children()) {
    bounding_box |= child->recursive_bounding_box(transformation.apply(child->transformation()));
  }
  return bounding_box;
}

std::unique_ptr<Object> Object::repudiate(Object &repudiatee)
{
  const auto global_transformation = repudiatee.global_transformation(Space::Scene);
  auto o = TreeElement<Object>::repudiate(repudiatee);
  repudiatee.set_global_transformation(global_transformation, Space::Scene);
  return o;
}

Object &Object::adopt(std::unique_ptr<Object> adoptee, const size_t pos)
{
  const auto global_transformation = adoptee->global_transformation(Space::Scene);
  Object& o = TreeElement<Object>::adopt(std::move(adoptee), pos);
  o.set_global_transformation(global_transformation, Space::Scene);
  return o;
}

std::unique_ptr<Object> Object::convert() const { return clone(); }
Flag Object::flags() const { return Flag::None; }

void Object::copy_tags(Object& other) const
{
  for (const Tag* tag : tags.ordered_items()) {
    ListOwningContext<Tag> context(tag->clone(other), other.tags);
    other.tags.insert(context);
  }
}

void Object::on_property_value_changed(Property *property)
{
  const auto object_tree_data_changed = [this](int column) {
    if (m_object_tree != nullptr) {
      const auto index = m_object_tree->index_of(*this).siblingAtColumn(column);
      Q_EMIT m_object_tree->dataChanged(index, index);
    }
  };

  if (   property == this->property(POSITION_PROPERTY_KEY)
      || property == this->property(ROTATION_PROPERTY_KEY)
      || property == this->property(SHEAR_PROPERTY_KEY)
      || property == this->property(SCALE_PROPERTY_KEY) )
  {
    Q_EMIT scene()->message_box().transformation_changed(*this);
  } else if (property == this->property(IS_ACTIVE_PROPERTY_KEY)) {
    object_tree_data_changed(ObjectTree::VISIBILITY_COLUMN);
    for (Object* c : all_descendants()) {
      c->m_visibility_cache_is_dirty = true;
    }
    update();
  } else if (property == this->property(NAME_PROPERTY_KEY)) {
    object_tree_data_changed(ObjectTree::OBJECT_COLUMN);
  } else if (property == this->property(VIEWPORT_VISIBILITY_PROPERTY_KEY)) {
    object_tree_data_changed(ObjectTree::VISIBILITY_COLUMN);
    if (is_root()) {
      Q_EMIT scene()->message_box().appearance_changed();
    } else {
      Q_EMIT scene()->message_box().appearance_changed(tree_parent());
    }
  } else if (property == this->property(VISIBILITY_PROPERTY_KEY)) {
    object_tree_data_changed(ObjectTree::VISIBILITY_COLUMN);
  }
}

void Object::post_create_hook() { }

void Object::update()
{
  if (Scene* scene = this->scene(); scene != nullptr) {
    Q_EMIT scene->message_box().appearance_changed(*this);
  }
}

double Object::apply_border(double t, Border border)
{
  switch (border) {
  case Border::Clamp: return std::min(1.0, std::max(0.0, t));
  case Border::Wrap: return fmod(fmod(t, 1.0) + 1.0, 1.0);
  case Border::Hide: return (t >= 0.0 && t <= 1.0) ? t : -1.0;
  case Border::Reflect: {
    const bool flip = int(t / 1.0) % 2 == 1;
    t = apply_border(t, Border::Wrap);
    return flip ? (1.0-t) : t;
  }
  }
  Q_UNREACHABLE();
}

Point Object::evaluate(const double t) const
{
  Q_UNUSED(t)
  return Point();
}

double Object::path_length() const { return -1.0; }
bool Object::is_closed() const { return false; }

void Object::set_position_on_path(AbstractPropertyOwner* path, const bool align, const double t,
                                  Space space)
{
  if (path != nullptr && path->kind == Kind::Object) {
    auto* path_object = static_cast<Object*>(path);
    if (!path_object->is_ancestor_of(*this)) {
      const auto location = path_object->evaluate(std::clamp(t, 0.0, 1.0));
      const auto global_location = path_object->global_transformation(space).apply(location);
      set_oriented_position(global_location, align);
    } else {
      // it wouldn't crash but ux would be really bad. Don't allow cycles.
      LWARNING << "cycle.";
    }
  }
}

void Object::set_oriented_position(const Point& op, const bool align)
{
  auto transformation = global_transformation(Space::Viewport);
  if (align) { transformation.set_rotation(op.rotation()); }
  transformation.set_translation(op.position);
  set_global_transformation(transformation, Space::Viewport);
}


bool Object::is_active() const { return property(IS_ACTIVE_PROPERTY_KEY)->value<bool>(); }

bool Object::is_visible(bool viewport) const
{
  const QString key = viewport ? VIEWPORT_VISIBILITY_PROPERTY_KEY : VISIBILITY_PROPERTY_KEY;
  const auto compute_visibility = [this, key, viewport]() {
    switch (property(key)->value<Visibility>()) {
    case Visibility::Hidden:
      return false;
    case Visibility::Visible:
      return true;
    default:
      if (is_root()) {
        return true;
      } else {
        return tree_parent().is_visible(viewport);
      }
    }
  };

  if (m_visibility_cache_is_dirty) {
    m_visibility_cache_value = compute_visibility();
  }
  return m_visibility_cache_value;
}

std::vector<const omm::Style*> Object::find_styles() const
{
  const auto get_style = [](const omm::Tag* tag) -> const omm::Style* {
    if (tag->type() == omm::StyleTag::TYPE) {
      const auto* property_owner = tag->property(omm::StyleTag::STYLE_REFERENCE_PROPERTY_KEY)
                                       ->value<omm::ReferenceProperty::value_type>();
      assert(  property_owner == nullptr
            || property_owner->kind == omm::Kind::Style );
      return static_cast<const omm::Style*>(property_owner);
    } else {
      return nullptr;
    }
  };

  const auto tags = this->tags.ordered_items();
  return ::filter_if(::transform<const omm::Style*>(tags, get_style), ::is_not_null);
}

bool Object::contains(const Vec2f &point) const
{
  Q_UNUSED(point)
  return false;
}

void Object::update_recursive()
{
  // it's important to first update the children because of the way e.g. Cloner does its caching.
  for (auto* child : tree_children()) {
    child->update_recursive();
  }
  update();
  //  Q_EMIT scene()->message_box.appearance_changed(*this);
}

QString Object::tree_path() const
{
  const QString path = is_root() ? "" : tree_parent().tree_path();
  return path + "/" + name();
}

void Object::draw_object(Painter&, const Style&, Painter::Options) const {}
void Object::draw_handles(Painter&) const {}
std::vector<Point> Object::points() const { return {}; }

void Object::set_object_tree(ObjectTree &object_tree)
{
  if (m_object_tree == nullptr) {
    m_object_tree = &object_tree;
  } else {
    assert(m_object_tree == &object_tree);
  }
}

void Object::on_child_added(Object &child)
{
  TreeElement::on_child_added(child);
  Q_EMIT scene()->message_box().appearance_changed(*this);
}

void Object::on_child_removed(Object &child)
{
  TreeElement::on_child_removed(child);
  Q_EMIT scene()->message_box().appearance_changed(*this);
}

void Object::listen_to_changes(const std::function<Object *()> &get_watched)
{
  connect(&scene()->message_box(), qOverload<Object&>(&MessageBox::appearance_changed), this,
          [get_watched, this](Object& o)
  {
    Object* r = get_watched();
    if (r != nullptr) {
      if (r->is_ancestor_of(*this)) {
        {
          QSignalBlocker blocker(&scene()->message_box());
          update();
        }
        Q_EMIT scene()->message_box().appearance_changed();
      } else if (r->is_ancestor_of(o)) {
        update();
      }
    }
  });
}

void Object::listen_to_children_changes()
{
  const auto on_change = [this](Object& o) {
    if (&o != this && is_ancestor_of(o)) {
      update();
    }
  };
  connect(&scene()->message_box(), &MessageBox::transformation_changed, this, on_change);
  connect(&scene()->message_box(), qOverload<Object&>(&MessageBox::appearance_changed),
          this, on_change);
}

}  // namespace omm
