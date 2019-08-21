#pragma once

#include <QAbstractItemModel>
#include "scene/structure.h"
#include "objects/object.h"
#include "scene/tree.h"
#include "scene/list.h"

namespace omm
{

class Scene;

template<typename StructureT, typename ItemModel>
class ItemModelAdapter : public ItemModel
{
  static_assert( std::is_base_of<QAbstractItemModel, ItemModel>::value,
                 "ItemModel must be derived from QAbstractItemModel" );
public:
  using structure_type = StructureT;
  explicit ItemModelAdapter(Scene& scene, StructureT& structure);
  virtual ~ItemModelAdapter() = default;
  Qt::DropActions supportedDragActions() const override;
  Qt::DropActions supportedDropActions() const override;
  bool canDropMimeData( const QMimeData *data, Qt::DropAction action,
                        int row, int column, const QModelIndex &parent ) const override;
  bool dropMimeData( const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent ) override;
  QStringList mimeTypes() const override;
  QMimeData* mimeData(const QModelIndexList &indexes) const override;
  virtual typename structure_type::item_type& item_at(const QModelIndex& index) const = 0;
  virtual QModelIndex index_of(typename structure_type::item_type& item) const = 0;
  Scene& scene;
  StructureT& structure;

private:
};

}  // namespace omm
