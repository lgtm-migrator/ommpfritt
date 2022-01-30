#pragma once

#include "nodesystem/node.h"

namespace omm::nodes
{

template<PortType> class OrdinaryPort;

class DecomposeColorNode : public Node
{
  Q_OBJECT
public:
  explicit DecomposeColorNode(NodeModel& model);
  static constexpr auto INPUT_PROPERTY_KEY = "in";
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "DecomposeColorNode");

  [[nodiscard]] QString output_data_type(const OutputPort& port) const override;
  [[nodiscard]] QString title() const override;
  [[nodiscard]] bool accepts_input_data_type(const QString& type, const InputPort& port) const override;
  [[nodiscard]] QString type() const override;
  static const Detail detail;
};

}  // namespace omm::nodes
