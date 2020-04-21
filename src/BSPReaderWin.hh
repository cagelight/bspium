#pragma once

#include <QMainWindow>
#include <QFileInfo>

class BSPReaderWindow : public QMainWindow {
	Q_OBJECT
public:
	
	BSPReaderWindow();
	~BSPReaderWindow();
	
public slots:
	void close();
	void open(QFileInfo file);
	void refresh_bsp_info();
	
private:
	struct PrivateData;
	std::unique_ptr<PrivateData> m_data;
};
