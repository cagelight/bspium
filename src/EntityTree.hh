#pragma once

#include <libbsp.hh>

#include <QAbstractItemModel>

#include <vector>

struct EntityTreeItem {
	EntityTreeItem * parent = nullptr;
	virtual ~EntityTreeItem() = default;
	int child_index = 0;
	std::vector<std::unique_ptr<EntityTreeItem>> children;
	
	virtual QVariant get_data(int role, int idx) = 0;
	virtual Qt::ItemFlags get_flags(int) { 
		return Qt::ItemIsEnabled;
	}
};

struct EntityTreeRoot : public EntityTreeItem {
	std::shared_ptr<BSPI::EntityArray> source;
	
	inline QVariant get_data(int, int) override { return {}; }
};

struct EntityTreeEnt : public EntityTreeItem {
	BSPI::Entity * data_parent;
	uint32_t entity_index;
	
	inline QVariant get_data(int role, int col) override {
		switch (role) {
			default: return {};
			case Qt::DisplayRole:
			case Qt::EditRole:
				switch (col) {
					case 0: return entity_index;
					case 1: {
						auto classname = data_parent->find("classname");
						return (classname == data_parent->end()) ?
							QVariant {} :
							QString::fromStdString( meadow::i2s(classname->second) )
						;
					}
					case 2: {
						auto targetname = data_parent->find("targetname");
						return (targetname == data_parent->end()) ?
							QVariant {} :
							QString::fromStdString( meadow::i2s(targetname->second) )
						;
					}
					default: return {};
				}
			case Qt::UserRole: {
				if (col != 2) return {};
				QString sort_str;
				for (auto const & kvp : *data_parent) {
					sort_str.append(kvp.first.data());
					sort_str.append(" ");
					sort_str.append(kvp.second.data());
					sort_str.append(" ");
				}
				return sort_str;
			}
		}
	}
	Qt::ItemFlags get_flags(int col) override {
		if (!col) return Qt::ItemIsEnabled;
		else return Qt::NoItemFlags;
	}
};

struct EntityTreeField : public EntityTreeItem {
	BSPI::Entity * data_parent;
	meadow::istring_view key;
	
	inline QVariant get_data(int role, int col) override {
		switch (role) {
			default: return {};
			case Qt::DisplayRole:
			case Qt::EditRole:
				switch (col) {
					case 0: return QString::fromStdString( meadow::i2s(key) );
					case 1: return QString::fromStdString( meadow::i2s(data_parent->at(meadow::istring{key})) );
					default: return {};
				}
			case Qt::UserRole: {
				if (col != 2) return {};
				QString sort_str;
				for (auto const & kvp : *data_parent) {
					sort_str.append(kvp.first.data());
					sort_str.append(" ");
					sort_str.append(kvp.second.data());
					sort_str.append(" ");
				}
				return sort_str;
			}
		}
	}
	Qt::ItemFlags get_flags(int col) override { 
		switch (col) {
			case 0:
			case 1: return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEditable;
			default: return Qt::ItemNeverHasChildren;
		}
	}
};

class EntityTreeModel : public QAbstractItemModel {
	Q_OBJECT
	
public:
	EntityTreeModel(BSP::Reader::EntityArray const & ents);
	~EntityTreeModel() = default;
	
	BSP::LumpProviderPtr generate_provider();
	
	// QAbstractItemModel implementations
	QModelIndex index(int row, int column, QModelIndex const & parent = {}) const override;
	QModelIndex parent(QModelIndex const & index) const override;
	int rowCount(QModelIndex const & parent) const override;
	int columnCount(QModelIndex const & parent) const override;
	QVariant data(QModelIndex const & index, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(QModelIndex const &) const override;
	bool setData(QModelIndex const &, QVariant const &, int role = Qt::EditRole) override;
	QVariant headerData(int, Qt::Orientation, int) const override { return {}; }
	
private:
	using Entity = QMap<QString, QString>;
	using EntityArray = QList<Entity>;
	std::unique_ptr<EntityTreeRoot> m_root;
	std::shared_ptr<BSPI::EntityArray> m_data;
	
	EntityTreeItem * get_item(QModelIndex const &) const;
};
