#pragma once

#include "properties/numericproperty.h"
#include <Qt>

namespace omm
{
class IntegerProperty : public NumericProperty<int>
{
public:
  using NumericProperty::NumericProperty;
  void deserialize(serialization::DeserializerWorker& worker);
  void serialize(serialization::SerializerWorker& worker) const;
  static const PropertyDetail detail;
  IntegerProperty& set_special_value(const QString& label);

public:
  QString special_value_label;
};

}  // namespace omm
