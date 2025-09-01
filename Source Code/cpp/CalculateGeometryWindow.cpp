#include "CalculateGeometryWindow.h"
#include "LoggerConfig.h"
#include "MYGIS.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QToolButton>
#include <QPushButton>

CalculateGeometryWindow::CalculateGeometryWindow(QWidget* parent, MYGIS* myGIS)
	: QDialog(parent), mpMYGIS(myGIS), ui(new Ui::CalculateGeometryWindowClass)
{
	ui->setupUi(this); // UI 组件初始化

	// 设置默认值
	ui->shpLinelineEdit->setText("shp_line");
	ui->shpArealineEdit->setText("shp_area");

	if (mpMYGIS) { // 确保 mpMYGIS 不为空
		const auto& layers = mpMYGIS->getVectorLayers();
		for (const auto& layerPair : layers) {
			ui->comboBox->addItem(layerPair.first); // 添加图层名称到 comboBox
		}
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("MYGIS pointer is null!"));
	}

	// 连接 startpushButton 和 cancelpushButton 到相应的槽函数
	connect(ui->startpushButton, &QPushButton::clicked, this, &CalculateGeometryWindow::startcalculateGeometry);
	connect(ui->cancelpushButton, &QPushButton::clicked, this, &CalculateGeometryWindow::close);
}

CalculateGeometryWindow::~CalculateGeometryWindow()
{
	delete ui;
}

void CalculateGeometryWindow::startcalculateGeometry()
{
	ui->progressBar->setValue(0);  // 更新进度条
	QString selectedLayer = ui->comboBox->currentText();
	QString lineField = ui->shpLinelineEdit->text();
	QString areaField = ui->shpArealineEdit->text();

	if (selectedLayer.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("No layer selected!"));
		// 日志记录未选择图层的情况
		logger.warn("startcalculateGeometry: No layer selected.");
		return;
	}
	ui->progressBar->setValue(20);  // 更新进度条
	// 调用 MYGIS 的方法来计算选定图层的几何信息
	if (mpMYGIS) {
		ui->progressBar->setValue(30);  // 更新进度条
		bool success = mpMYGIS->calculateGeometry(selectedLayer, lineField, areaField);
		ui->progressBar->setValue(100);  // 更新进度条
		if (success) {
			QMessageBox::information(this, tr("Success"), tr("计算几何成功!"));
			// 日志记录成功计算几何
			logger.info("startcalculateGeometry: Geometry calculation succeeded for layer %s.", selectedLayer.toStdString().c_str());
		}
		else {
			QMessageBox::critical(this, tr("Error"), tr("计算几何失败!"));
			// 日志记录计算几何失败
			logger.error("startcalculateGeometry: Geometry calculation failed for layer %s.", selectedLayer.toStdString().c_str());
		}
	}
}


