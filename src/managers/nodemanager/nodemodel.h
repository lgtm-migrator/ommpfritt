#pragma once

#include "common.h"
#include "aspects/serializable.h"
#include <QObject>
#include <set>
#include <memory>
#include "managers/nodemanager/port.h"

namespace omm
{

class Node;
class AbstractPort;
class OutputPort;
class InputPort;
class Scene;

class NodeModel : public QObject, public Serializable
{
  Q_OBJECT
public:
  explicit NodeModel(Scene* scene);
  NodeModel(const NodeModel& other);
  ~NodeModel();

  Node& add_node(std::unique_ptr<Node> node);
  std::unique_ptr<Node> extract_node(Node& node);
  std::set<Node*> nodes() const;
  bool can_connect(const AbstractPort& a, const AbstractPort& b) const;
  bool can_connect(const OutputPort& a, const InputPort& b) const;
  using QObject::connect;

  void serialize(AbstractSerializer&, const Pointer&) const override;
  void deserialize(AbstractDeserializer&deserializer, const Pointer&ptr) override;

  static constexpr auto NODES_POINTER = "nodes";
  static constexpr auto TYPE_POINTER = "type";

  bool find_path(const Node& start, const Node& end, std::list<const Node*>& path) const;
  bool find_path(std::list<const Node*>& path, const Node& end) const;
  bool find_path(const Node& start, const Node& end) const;
  bool types_compatible(const QString& from, const QString& to) const;

  std::set<AbstractPort*> ports() const;
  template<typename PortT> std::set<PortT*> ports() const
  {
    static const auto pred = [](AbstractPort* p) { return p->port_type == PortT::PORT_TYPE; };
    static const auto conv = [](AbstractPort* p) { return static_cast<PortT*>(p); };
    return ::transform<PortT*>(::filter_if(ports(), pred), conv);
  }

  Scene* scene() const { return m_scene; }
  void notify_appearance_changed();
  void notify_topology_changed();

Q_SIGNALS:
  void appearance_changed();
  void topology_changed();

private:
  std::set<std::unique_ptr<Node>> m_nodes;
  Scene* m_scene;
};


}  // namespace omm
