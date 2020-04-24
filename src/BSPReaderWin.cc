#include "BSPReaderWin.hh"
#include "EntityTree.hh"

#include <libbsp.hh>

#include <QAction>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSortFilterProxyModel>
#include <QTabWidget>
#include <QTreeView>

#include <array>

struct BSPReaderWindow::PrivateData {
	QFile * file = nullptr;
	
	// bsp
	BSP::Reader bspr;
	BSP::Reader::EntityArray entities;
	BSP::Assembler bspa;
	
	// info
	std::array<QLabel *, 18> general_info_labels {};
	QScrollArea * general_entities_scrollarea = nullptr;
	
	// ents
	std::unique_ptr<EntityTreeModel> entmodel;
	QScrollArea * ent_scroll = nullptr;
	QLineEdit * ent_filter = nullptr;
	QSortFilterProxyModel * ent_filter_proxy = nullptr;
};

BSPReaderWindow::BSPReaderWindow() : QMainWindow(), m_data { new PrivateData } {
	
	this->setMinimumWidth(600);
	
	auto menu_file = this->menuBar()->addMenu("File");
	auto menu_file_open = menu_file->addAction("Open");
	connect(menu_file_open, &QAction::triggered, this, [this](){
		QFileInfo file_path = QFileDialog::getOpenFileName(this, tr("Open BSP File"), QDir::currentPath(), tr("BSP Files (*.bsp)"));
		if (!file_path.exists() || !file_path.isFile()) return;
		this->open(file_path);
	});
	auto menu_file_save = menu_file->addAction("Save");
	connect(menu_file_save, &QAction::triggered, this, [this](){
		QString file_path = QFileDialog::getSaveFileName(this, tr("Save BSP File"), QDir::currentPath(), tr("BSP Files (*.bsp)"));
		if (file_path.isNull()) return;
		this->save(file_path);
	});
	
	auto main_widget = new QTabWidget { this };
	this->setCentralWidget(main_widget);
	
	// ================================================================
	// GENERAL TAB
	// ================================================================
	{
	
		auto tab = new QWidget { main_widget };
		main_widget->addTab(tab, "General Info");
		
		auto tab_layout = new QGridLayout { tab };
		tab_layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
		
		auto overall_panel = new QGroupBox { "Overall Stats", tab };
		overall_panel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		auto overall_layout = new QGridLayout { overall_panel };
		overall_layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
		tab_layout->addWidget(overall_panel, 0, 0);
		
		auto left_label_gen = [tab](QString str){
			QLabel * lab = new QLabel {str, tab};
			lab->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
			lab->setAlignment(Qt::AlignRight | Qt::AlignTop);
			return lab;
		};
		
		int left_col = 0;
		overall_layout->addWidget( left_label_gen("Entities: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Shaders: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Planes: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Nodes: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Leafs: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Leaf Surfaces: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Leaf Brushes: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Brush Models: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Brushes: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Brush Sides: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Draw Verts: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Draw Indices: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Fogs: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Surfaces: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Lightmaps: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Lightgrid Elements: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Visibility Clusters: "), left_col++, 0 );
		overall_layout->addWidget( left_label_gen("Lightarray Elements: "), left_col++, 0 );
		
		int right_col = 0;
		for (auto & lab : m_data->general_info_labels) {
			lab = new QLabel {"", tab};
			lab->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
			lab->setAlignment(Qt::AlignLeft | Qt::AlignTop);
			lab->setTextInteractionFlags(Qt::TextSelectableByMouse);
			overall_layout->addWidget( lab, right_col++, 1 );
		}
		
		auto ents_panel = new QGroupBox { "Entities", tab };
		tab_layout->addWidget(ents_panel, 0, 1);
		auto ents_panel_layout = new QGridLayout { ents_panel };
		m_data->general_entities_scrollarea = new QScrollArea { ents_panel };
		ents_panel_layout->addWidget(m_data->general_entities_scrollarea, 0, 0);
		ents_panel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		m_data->general_entities_scrollarea->setWidgetResizable(true);
	}
	// ================================================================
	// ENTITY TAB
	// ================================================================
	{
		
		auto tab = new QWidget { main_widget };
		main_widget->addTab(tab, "Entities");
		auto tab_layout = new QGridLayout { tab };
		
		auto filter_widget = new QWidget { tab };
		auto filter_layout = new QHBoxLayout { filter_widget };
		filter_layout->setMargin(0);
		m_data->ent_filter = new QLineEdit { filter_widget };
		filter_layout->addWidget(new QLabel { "Filter: ", filter_widget });
		filter_layout->addWidget(m_data->ent_filter);
		tab_layout->addWidget(filter_widget, 0, 0);
		
		m_data->ent_scroll = new QScrollArea { tab };
		tab_layout->addWidget(m_data->ent_scroll, 1, 0);
		m_data->ent_scroll->setWidgetResizable(true);
	}
	// ================================================================
}

BSPReaderWindow::~BSPReaderWindow() {
	close();
}

void BSPReaderWindow::open(QFileInfo file_info) {
	if (!file_info.exists()) {
		// TODO -- error handling
	}
	close();
	m_data->file = new QFile {file_info.canonicalFilePath(), this};
	m_data->file->open(QIODevice::ReadOnly);
	m_data->bspr.rebase(m_data->file->map(0, file_info.size(), QFileDevice::MapPrivateOption));
	
	init_bsp_info();
}

void BSPReaderWindow::save(QString file_path) {
	QFile f { file_path, this };
	if (!f.open(QIODevice::ReadWrite)) { // test if file is writable before BSP is generated
		QMessageBox::critical(this, "Save Failed", "Unable to create or open specified file for writing.");
		return;
	}
	f.close();
	auto bytes = m_data->bspa.assemble();
	f.open(QIODevice::WriteOnly | QIODevice::Truncate);
	size_t writ = f.write( reinterpret_cast<char const *>(bytes.data()), bytes.size() );
	if (writ != bytes.size()) {
		QMessageBox::critical(this, "Save Failed", "Failed to write complete file, unknown cause.");
		return;
	}
	f.close();
}

void BSPReaderWindow::close() {
	if (!m_data->file) return;
	delete m_data->file;
	m_data->file = nullptr;
}

void BSPReaderWindow::init_bsp_info() {
	
	m_data->entities = m_data->bspr.entities_parsed();
	m_data->general_info_labels[0]-> setText( QString::number(m_data->entities.size()) );
	m_data->general_info_labels[1]-> setText( QString::number(m_data->bspr.shaders().size()) );
	m_data->general_info_labels[2]-> setText( QString::number(m_data->bspr.planes().size()) );
	m_data->general_info_labels[3]-> setText( QString::number(m_data->bspr.nodes().size()) );
	m_data->general_info_labels[4]-> setText( QString::number(m_data->bspr.leafs().size()) );
	m_data->general_info_labels[5]-> setText( QString::number(m_data->bspr.leafsurfaces().size()) );
	m_data->general_info_labels[6]-> setText( QString::number(m_data->bspr.leafbrushes().size()) );
	m_data->general_info_labels[7]-> setText( QString::number(m_data->bspr.models().size()) );
	m_data->general_info_labels[8]-> setText( QString::number(m_data->bspr.brushes().size()) );
	m_data->general_info_labels[9]-> setText( QString::number(m_data->bspr.brushsides().size()) );
	m_data->general_info_labels[10]->setText( QString::number(m_data->bspr.drawverts().size()) );
	m_data->general_info_labels[11]->setText( QString::number(m_data->bspr.drawindices().size()) );
	m_data->general_info_labels[12]->setText( QString::number(m_data->bspr.fogs().size()) );
	m_data->general_info_labels[13]->setText( QString::number(m_data->bspr.surfaces().size()) );
	m_data->general_info_labels[14]->setText( QString::number(m_data->bspr.lightmaps().size()) );
	m_data->general_info_labels[15]->setText( QString::number(m_data->bspr.lightgrids().size()) );
	if (m_data->bspr.has_visibility()) {
		m_data->general_info_labels[16]->setText( QString::number(m_data->bspr.visibility().header.clusters) );
	} else {
		m_data->general_info_labels[16]->setText( "no visibility data" );
	}
	m_data->general_info_labels[17]->setText( QString::number(m_data->bspr.lightarray().size()) );
	
	std::map<meadow::istring_view, std::vector<BSP::Reader::Entity *>> classes;
	for (auto & ent : m_data->entities) {
		auto classname = ent.find("classname");
		if (classname == ent.end())
			classes["<no classname>"].emplace_back(&ent);
		else
			classes[classname->second].emplace_back(&ent);
	}
	
	QWidget * general_ents = new QWidget { m_data->general_entities_scrollarea };
	general_ents->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
	QGridLayout * layout = new QGridLayout { general_ents };
	layout->setSpacing(0);
	layout->setMargin(0);
	int row = 0;
	for (auto const & v : classes) {
		QLabel * classname_label = new QLabel { QString::fromStdString( meadow::i2s(v.first) ) + ": ", general_ents };
		classname_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
		classname_label->setMargin(4);
		classname_label->setStyleSheet((row % 2) ? "background-color:palette(base)" : "background-color:palette(alternate-base)");
		classname_label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
		QLabel * classname_count_label = new QLabel { QString::number(v.second.size()), general_ents };
		classname_count_label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
		classname_count_label->setMargin(4);
		classname_count_label->setStyleSheet((row % 2) ? "background-color:palette(base)" : "background-color:palette(alternate-base)");
		classname_count_label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
		layout->addWidget(classname_label, row, 0);
		layout->addWidget(classname_count_label, row++, 1);
	}
	m_data->general_entities_scrollarea->setWidget(general_ents);
	m_data->general_entities_scrollarea->show();
	
	QTreeView * ent_view = new QTreeView { };
	m_data->entmodel.reset( new EntityTreeModel { m_data->entities } );
	m_data->ent_filter_proxy = new QSortFilterProxyModel { ent_view };
	m_data->ent_filter_proxy->setSourceModel(m_data->entmodel.get());
	ent_view->setModel(m_data->ent_filter_proxy);
	m_data->ent_scroll->setWidget(ent_view);
	m_data->ent_scroll->show();
	ent_view->setColumnWidth(0, 200);
	ent_view->setColumnWidth(1, 200);
	ent_view->setSortingEnabled(true);
	ent_view->sortByColumn(0, Qt::AscendingOrder);
	m_data->ent_filter_proxy->setFilterKeyColumn(2);
	m_data->ent_filter_proxy->setRecursiveFilteringEnabled(false);
	m_data->ent_filter_proxy->setFilterRole(Qt::UserRole);
	connect(m_data->ent_filter, &QLineEdit::textChanged, m_data->ent_filter_proxy, [this](QString const & str){
			m_data->ent_filter_proxy->setFilterRegExp( QRegExp { str, Qt::CaseInsensitive, QRegExp::FixedString } );
	});
	
	BSP::LumpProviderPtr pprov = std::make_shared<BSP::BSPReaderLumpProvider>( m_data->bspr );
	m_data->bspa = BSP::Assembler { pprov };
	m_data->bspa[BSP::LumpIndex::ENTITIES] = m_data->entmodel->generate_provider();
}
