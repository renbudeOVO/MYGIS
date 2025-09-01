#include "featuresToShpWindow.h"
#include "LoggerConfig.h"
#include "MYGIS.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QToolButton>
#include <QPushButton>

featuresToShpWindow::featuresToShpWindow(QWidget* parent, MYGIS* myGIS)
	: QDialog(parent), mpMYGIS(myGIS), ui(new Ui::featuresToShpWindowClass)
{
	ui->setupUi(this); //  UI 组件初始化

	if (mpMYGIS) { // 确保 mpMYGIS 不为空
		const auto& layers = mpMYGIS->getVectorLayers();
		for (const auto& layerPair : layers) {
			ui->comboBox->addItem(layerPair.first); // 添加图层名称到 comboBox
		}
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("MYGIS pointer is null!"));
	}

	// 连接 toolButton 的点击事件到槽函数
	connect(ui->toolButton, &QToolButton::clicked, this, &featuresToShpWindow::selectSavePath);

	// 连接 savepushButton 的点击事件到槽函数
	connect(ui->savepushButton, &QPushButton::clicked, this, &featuresToShpWindow::saveLayerToShapefile);

	// 连接 cancelpushButton 的点击事件以关闭窗口
	connect(ui->cancelpushButton, &QPushButton::clicked, this, &featuresToShpWindow::close);
}

featuresToShpWindow::~featuresToShpWindow()
{
	delete ui;
}

//保存路径函数
void featuresToShpWindow::selectSavePath()
{
	QString filePath = QFileDialog::getSaveFileName(this, tr("Save Shapefile"), "", tr("Shapefile (*.shp)"));
	if (!filePath.isEmpty()) {
		ui->lineEdit->setText(filePath);
	}
}

//要素转shapefile
void featuresToShpWindow::saveLayerToShapefile()
{
	ui->progressBar->setValue(0);  // 更新进度条
	QString layerName = ui->comboBox->currentText();
	QString savePath = ui->lineEdit->text();

	if (layerName.isEmpty() || savePath.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("Please select a layer and specify a save path."));
		logger.warn("saveLayerToShapefile: No layer selected or save path not specified.");
		return;
	}
	ui->progressBar->setValue(20);  // 更新进度条

	// 调用 GIS 系统的函数保存图层为 Shapefile
	bool success = mpMYGIS->saveVectorLayerAsShapefile(layerName, savePath);
	ui->progressBar->setValue(100);  // 更新进度条

	if (success) {
		QMessageBox::information(this, tr("Success"), tr("成功保存为shapefile!"));
		logger.info("saveLayerToShapefile: Layer %s successfully saved as Shapefile at %s.", layerName.toStdString().c_str(), savePath.toStdString().c_str());
		close();
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("Failed to save the layer."));
		logger.error("saveLayerToShapefile: Failed to save layer %s as Shapefile.", layerName.toStdString().c_str());
	}
}

