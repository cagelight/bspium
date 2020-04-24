#include "EntityTree.hh"

#include <QDebug>
#include <QSize>
#include <QMessageBox>

EntityTreeModel::EntityTreeModel(BSP::Reader::EntityArray const & ents) {
	
	m_data = std::make_shared<BSPI::EntityArray>(ents);
	
	m_root.reset( new EntityTreeRoot );
	m_root->source = m_data;
		
	size_t entity_idx = 0;
	for (auto entity_iter = ents.begin(); entity_iter != ents.end(); entity_iter++, entity_idx++) {
		auto & ent_item_p = m_root->children.emplace_back( new EntityTreeEnt );
		EntityTreeEnt * ent_item = static_cast<EntityTreeEnt *>(ent_item_p.get());
		ent_item->parent = m_root.get();
		ent_item->child_index = entity_idx;
		ent_item->entity_index = entity_idx;
		ent_item->data_parent = &m_data->at(ent_item->entity_index);
		
		size_t field_idx = 0;
		for (auto field_iter = entity_iter->begin(); field_iter != entity_iter->end(); field_iter++, field_idx++) {
			auto & field_item_p = ent_item->children.emplace_back( new EntityTreeField );
			EntityTreeField * field_item = static_cast<EntityTreeField *>(field_item_p.get());
			field_item->parent = ent_item;
			field_item->child_index = field_idx;
			field_item->data_parent = ent_item->data_parent;
			field_item->key = field_iter->first;
		}
	}
}

BSP::LumpProviderPtr EntityTreeModel::generate_provider() {
	return std::make_shared<BSP::BSPIEntityArrayLumpProvider>(m_data);
}

QModelIndex EntityTreeModel::index(int row, int column, QModelIndex const & parent) const {
	if (parent.isValid() && parent.column()) return {};
	EntityTreeItem * item = get_item(parent);
	if (!item) return {};
	
	if (row >= (int)item->children.size()) return {};
	EntityTreeItem * child = item->children[row].get();
	if (!child) return {};
	return createIndex(row, column, child);
}

QModelIndex EntityTreeModel::parent(QModelIndex const & index) const {
	if (!index.isValid()) return {};
	EntityTreeItem * item = reinterpret_cast<EntityTreeItem *>(index.internalPointer());
	if (!item || !item->parent || item->parent == m_root.get()) return {};
	return createIndex(item->child_index, 0, item->parent);
}

int EntityTreeModel::rowCount(QModelIndex const & parent) const {
	if (!parent.isValid()) return m_root->children.size();
	EntityTreeItem * item = reinterpret_cast<EntityTreeItem *>(parent.internalPointer());
	return item->children.size();
}

int EntityTreeModel::columnCount(QModelIndex const &) const {
	return 3;
}

QVariant EntityTreeModel::data(QModelIndex const & index, int role) const {
	EntityTreeItem * item = reinterpret_cast<EntityTreeItem *>(index.internalPointer());
	if (!index.isValid()) return {};
	return item->get_data(role, index.column());
}

Qt::ItemFlags EntityTreeModel::flags(QModelIndex const & index) const {
	EntityTreeItem * item = get_item(index);
	if (!item) return Qt::NoItemFlags;
	else return item->get_flags(index.column());
}

bool EntityTreeModel::setData(QModelIndex const & index, QVariant const & value, int role) {
	switch (role) {
		default: return QAbstractItemModel::setData(index, value, role);
		case Qt::EditRole:
			break;
	}
	EntityTreeItem * item = get_item(index);
	if (!item) return false;
	
	if (dynamic_cast<EntityTreeRoot *>(item)) return false;
	if (dynamic_cast<EntityTreeEnt *>(item)) return false;
	if (dynamic_cast<EntityTreeField *>(item)) {
		EntityTreeField * f = dynamic_cast<EntityTreeField *>(item);
		meadow::istring new_value = meadow::s2i(value.toString().toStdString());
		if (!new_value.size()) return false;
		switch (index.column()) {
			case 0: {
				if (new_value == f->key) return false;
				if (f->data_parent->find(new_value) != f->data_parent->end()) {
					QMessageBox::critical(nullptr, "Cannot Rename Field", "Cannot rename field, new field value already exists.");
					return false;
				}
				auto iter = f->data_parent->find(f->key);
				meadow::istring old_value = iter->second;
				f->data_parent->erase(iter);
				f->key = (f->data_parent->emplace(new_value, old_value)).first->first;
				emit dataChanged(index, index);
				return true;
			}
			case 1: {
				f->data_parent->at((meadow::istring)f->key) = new_value;
				emit dataChanged(index, index);
				return true;
			}
			default: return false;
		}
	}
	
	return false;
}

EntityTreeItem * EntityTreeModel::get_item(QModelIndex const & index) const {
	if (!index.isValid()) return m_root.get();
	return reinterpret_cast<EntityTreeItem *>(index.internalId());
}
