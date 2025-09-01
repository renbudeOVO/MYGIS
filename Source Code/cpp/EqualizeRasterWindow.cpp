#include "EqualizeRasterWindow.h"
#include "MYGIS.h"
#include "GeometryItem.h"
#include "RasterData.h"
#include "MyGraphicsView.h"
#include "LoggerConfig.h"


#include <QFileDialog>
#include <QMessageBox>
#include <QToolButton>
#include <QPushButton>
#include <QPixmap>

EqualizeRasterWindow::EqualizeRasterWindow(QWidget* parent, MYGIS* myGIS)
	: QDialog(parent), mpMYGIS(myGIS), ui(new Ui::EqualizeRasterWindowClass)
{
	ui->setupUi(this);

	if (mpMYGIS) { // 确保 mpMYGIS 不为空
		const auto& rasterLayers = mpMYGIS->getmvRasterItems();
		for (const auto& layerPair : rasterLayers) {
			ui->comboBox->addItem(layerPair.first); // 添加栅格图层名称到 comboBox
		}
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("MYGIS pointer is null!"));
	}

	connect(ui->startpushButton, &QPushButton::clicked, this, &EqualizeRasterWindow::startEqualizeRaster);

	// 连接 toolButton 点击事件到槽函数
	connect(ui->cancelPushButton, &QToolButton::clicked, this, &EqualizeRasterWindow::close);
}

EqualizeRasterWindow::~EqualizeRasterWindow()
{
	delete ui;
}

//均衡化增强
void EqualizeRasterWindow::startEqualizeRaster() {
	ui->progressBar->setValue(0);  // 更新进度条
	QString selectedLayer = ui->comboBox->currentText();
	if (selectedLayer.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("No raster layer selected!"));
		logger.warn("startEqualizeRaster: No raster layer selected.");
		return;
	}

	// 获取选中的栅格图层
	auto it = mpMYGIS->getmvRasterItems().find(selectedLayer);
	if (it == mpMYGIS->getmvRasterItems().end()) {
		QMessageBox::warning(this, tr("Warning"), tr("Selected raster layer not found!"));
		logger.error("startEqualizeRaster: Selected raster layer %s not found.", selectedLayer.toStdString().c_str());
		return;
	}

	RasterData* rasterData = mpMYGIS->getRasterData(selectedLayer);
	if (!rasterData) {
		QMessageBox::warning(this, tr("Warning"), tr("Failed to get raster data!"));
		logger.error("startEqualizeRaster: Failed to get raster data for layer %s.", selectedLayer.toStdString().c_str());
		return;
	}
	ui->progressBar->setValue(10);  // 更新进度条

	// 获取栅格图层的基本信息
	size_t rows = rasterData->getRows();
	size_t cols = rasterData->getCols();
	size_t bands = rasterData->getBandnum();
	size_t perPixSize = rasterData->getPerPixSize();

	// 初始化用于存储均衡化后的图像
	QImage equalizedImage(cols, rows, bands == 1 ? QImage::Format_Grayscale8 : QImage::Format_RGB888);
	ui->progressBar->setValue(30);  // 更新进度条

	// 对每个波段进行直方图均衡化
	for (size_t band = 1; band <= bands; ++band) {
		const std::vector<int>& histogram = rasterData->getHistogram(band);
		if (histogram.empty()) {
			continue;
		}

		// 计算累积分布函数 (CDF)
		std::vector<int> cdf(histogram.size(), 0);
		cdf[0] = histogram[0];
		for (int i = 1; i < histogram.size(); ++i) {
			cdf[i] = cdf[i - 1] + histogram[i];
		}

		// 对图像每个像素进行均衡化
		int totalPixels = static_cast<int>(rows * cols);
		for (size_t row = 0; row < rows; ++row) {
			for (size_t col = 0; col < cols; ++col) {
				int offset = (row * cols + col) * perPixSize * bands + (band - 1) * perPixSize;
				int pixelValue = static_cast<int>(rasterData->getImgData()[offset]);
				int equalizedValue = (cdf[pixelValue] - cdf[0]) * 255 / (totalPixels - cdf[0]);

				// 更新图像中的像素值
				if (bands == 1) {
					equalizedImage.setPixel(col, row, qRgb(equalizedValue, equalizedValue, equalizedValue));
				}
				else {
					int r = (band == 1) ? equalizedValue : equalizedImage.pixelColor(col, row).red();
					int g = (band == 2) ? equalizedValue : equalizedImage.pixelColor(col, row).green();
					int b = (band == 3) ? equalizedValue : equalizedImage.pixelColor(col, row).blue();
					equalizedImage.setPixel(col, row, qRgb(r, g, b));
				}
			}
		}
	}
	ui->progressBar->setValue(60);  // 更新进度条

	// 生成均衡化后的 QPixmap
	QPixmap equalizedPixmap = QPixmap::fromImage(equalizedImage);

	// 为均衡化后的图层指定新名称
	QString equalizedLayerName = selectedLayer + "_Equalized";

	// 使用 find 获取原始图层的地理信息
	auto rasterLayerIt = mpMYGIS->getmvRasterLayers().find(selectedLayer);
	if (rasterLayerIt == mpMYGIS->getmvRasterLayers().end()) {
		QMessageBox::warning(this, tr("Warning"), tr("Selected raster layer not found!"));
		logger.error("startEqualizeRaster: Selected raster layer %s not found in mvRasterLayers.", selectedLayer.toStdString().c_str());
		return;
	}
	GDALDataset* dataset = rasterLayerIt->second->getGDALDataset();
	ui->progressBar->setValue(70);  // 更新进度条

	// 获取地理变换信息
	double geoTransform[6];
	if (dataset->GetGeoTransform(geoTransform) != CE_None) {
		QMessageBox::warning(this, tr("Warning"), tr("Failed to get geotransform from the selected layer!"));
		logger.error("startEqualizeRaster: Failed to get geotransform for layer %s.", selectedLayer.toStdString().c_str());
		return;
	}
	ui->progressBar->setValue(80);  // 更新进度条

	// 使用 addRasterToManage 将均衡化后的图层添加到管理中
	mpMYGIS->addRasterToManage(equalizedPixmap, equalizedLayerName, geoTransform, rows, cols);
	logger.info("startEqualizeRaster: Added equalized raster layer %s to the project.", equalizedLayerName.toStdString().c_str());

	// 刷新视图
	mpMYGIS->getmpGraphicsView()->update();
	ui->progressBar->setValue(100);  // 更新进度条

	// 显示成功提示框
	QMessageBox::information(this, tr("Success"), tr("直方图均衡化已完成并更新显示!"));
}

