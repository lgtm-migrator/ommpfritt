#include "managers/nodemanager/node.h"

namespace omm
{

template<PortType> class OrdinaryPort;

class SpyNode : public Node
{
  Q_OBJECT
public:
  explicit SpyNode(Scene* scene);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SpyNode");
  QString type() const override { return TYPE; }

  bool accepts_input_data_type(const QString& type, const InputPort& port) const override;
  void set_text(const QString& text);

private:
  OrdinaryPort<PortType::Input>* m_port;
};

}  // namespace omm
