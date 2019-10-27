#include "properties/referenceproperty.h"
#include "objects/object.h"
#include "tags/tag.h"

namespace omm
{

using Flag = AbstractPropertyOwner::Flag;
using Kind = AbstractPropertyOwner::Kind;

const std::map<AbstractPropertyOwner::Kind, QString> ReferenceProperty::KIND_KEYS {
  { Kind::Tag, "tag" }, { Kind::Tool, "tool" },
  { Kind::Style, "style" }, { Kind::Object, "object" }
};

const std::map<AbstractPropertyOwner::Flag, QString> ReferenceProperty::FLAG_KEYS {
  { Flag::IsView, "is-view" }, { Flag::HasScript, "has-script" },
  { Flag::IsPathLike, "is-pathlike"}, { Flag::Convertable, "convertable" }
};

ReferenceProperty::ReferenceProperty()
  : TypedProperty(nullptr)
{
  set_default_value(nullptr);
}

ReferenceProperty::ReferenceProperty(const ReferenceProperty &other)
  : TypedProperty<AbstractPropertyOwner *>(other)
{
  auto* value = this->value();
  if (value != nullptr) {
    set(value);
  }
}

ReferenceProperty::~ReferenceProperty()
{
  QSignalBlocker blocker(this);
  set(nullptr);
}

void ReferenceProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  // TODO serialize allowed_kinds and required_flags
  TypedProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
}

void ReferenceProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  // TODO deserialize allowed_kinds and required_flags
  TypedProperty::deserialize(deserializer, root);

  // not all objects are restored yet, hence not all pointers are known.
  // remember the property to set `m_value` later.
  const auto ref_pointer = make_pointer(root, TypedPropertyDetail::VALUE_POINTER);
  m_reference_value_id = deserializer.get_size_t(ref_pointer);
  deserializer.register_reference_polisher(*this);
}

void ReferenceProperty::set_default_value(const value_type& value)
{
  assert(value == nullptr);
  TypedProperty::set_default_value(value);
}

ReferenceProperty& ReferenceProperty::set_allowed_kinds(AbstractPropertyOwner::Kind allowed_kinds)
{
  m_allowed_kinds = allowed_kinds;
  return *this;
}

ReferenceProperty&
ReferenceProperty::set_required_flags(AbstractPropertyOwner::Flag required_flags)
{
  m_required_flags = required_flags;
  return *this;
}

AbstractPropertyOwner::Kind ReferenceProperty::allowed_kinds() const { return m_allowed_kinds; }
AbstractPropertyOwner::Flag ReferenceProperty::required_flags() const { return m_required_flags; }

bool ReferenceProperty::is_compatible(const Property& other) const
{
  if (Property::is_compatible(other)) {
    auto& other_reference_property = static_cast<const ReferenceProperty&>(other);
    return other_reference_property.allowed_kinds() == allowed_kinds();
  } else {
    return false;
  }
}

std::unique_ptr<Property> ReferenceProperty::clone() const
{
  return std::make_unique<ReferenceProperty>(*this);
}

void ReferenceProperty
::update_referenes(const std::map<std::size_t, AbstractPropertyOwner *> &references)
{
  if (m_reference_value_id != 0) {
    try {
      set(references.at(m_reference_value_id));
    } catch (const std::out_of_range&) {
      LWARNING << "Failed to restore reference for property " << label();
    }
  }
}

void ReferenceProperty::revise() { set(nullptr); }

void ReferenceProperty::set(AbstractPropertyOwner * const &value)
{
  AbstractPropertyOwner* const old_value = this->value();
  TypedProperty::set(value);
  Q_EMIT reference_changed(old_value, value);
}

}   // namespace omm
