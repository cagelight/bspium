#pragma once

#include <QMainWindow>
#include <QFileInfo>

class BSPReaderWindow : public QMainWindow {
	Q_OBJECT
public:
	
	BSPReaderWindow();
	~BSPReaderWindow();
	
public slots:
	void open(QFileInfo file);
	void save(QString file);
	void close();
	void init_bsp_info();
	
private:
	struct PrivateData;
	std::unique_ptr<PrivateData> m_data;
};
