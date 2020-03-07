#include "managers/nodemanager/nodes/spynode.h"
#include "managers/nodemanager/nodemodel.h"
#include "managers/nodemanager/ordinaryport.h"

namespace omm
{

const Node::Detail SpyNode::detail {
  {
    { AbstractNodeCompiler::Language::Python, "" }
  },
  {
    QT_TRANSLATE_NOOP("NodeMenuPath", "General"),
  },
};

SpyNode::SpyNode(NodeModel& model)
  : Node(model)
{
  m_port = &add_port<OrdinaryPort<PortType::Input>>(tr("value"));
}

bool SpyNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  Q_UNUSED(port)
  return type != NodeCompilerTypes::INVALID_TYPE;
}

void SpyNode::set_text(const QString& text)
{
  m_port->set_label(text);
}

}  // namespace
