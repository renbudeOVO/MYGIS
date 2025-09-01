#include "RasterFalseColorWindow.h"
#include "GeometryItem.h"
#include "RasterData.h"
#include "MyGraphicsView.h"
#include "MYGIS.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QToolButton>
#include <QPushButton>
#include <QImage>

RasterFalseColorWindow::RasterFalseColorWindow(QWidget *parent, MYGIS* myGIS)
	: QDialog(parent), mpMYGIS(myGIS), ui(new Ui::RasterFalseColorWindowClass)
{
	ui->setupUi(this);

	if (mpMYGIS) { // 确保 mpMYGIS 不为空
	const auto& rasterLayers = mpMYGIS->getmvRasterItems();
	for (const auto& layerPair : rasterLayers) {
		ui->RastercomboBox->addItem(layerPair.first); // 添加栅格图层名称到 comboBox
	}
	}
	else {
	QMessageBox::critical(this, tr("Error"), tr("MYGIS pointer is null!"));
	}

	// 添加默认选项
	ui->band1comboBox->addItem("波段1");
	ui->band1comboBox->addItem("波段2");
	ui->band1comboBox->addItem("波段3");

	// 设置默认选择的选项为 "波段1"
	ui->band1comboBox->setCurrentIndex(0); // 索引从0开始，所以2对应的是"波段3"

	// 添加默认选项
	ui->band2comboBox->addItem("波段1");
	ui->band2comboBox->addItem("波段2");
	ui->band2comboBox->addItem("波段3");

	// 设置默认选择的选项为 "波段2"
	ui->band2comboBox->setCurrentIndex(1); // 索引从0开始，所以2对应的是"波段3"

	// 添加默认选项
	ui->band3comboBox->addItem("波段1");
	ui->band3comboBox->addItem("波段2");
	ui->band3comboBox->addItem("波段3");

	// 设置默认选择的选项为 "波段3"
	ui->band3comboBox->setCurrentIndex(2); // 索引从0开始，所以2对应的是"波段3"

	//运行
	connect(ui->startpushButton, &QPushButton::clicked, this, &RasterFalseColorWindow::startAnalysis);
	//取消
	connect(ui->cancelpushButton, &QPushButton::clicked, this, &RasterFalseColorWindow::close);


}

RasterFalseColorWindow::~RasterFalseColorWindow()
{
	delete ui;
}


void RasterFalseColorWindow::startAnalysis() {
	ui->progressBar->setValue(0);  // 更新进度条
	QString selectedLayer = ui->RastercomboBox->currentText();
	int bandR = ui->band1comboBox->currentIndex() + 1; // 索引从0开始，所以需要+1以获取正确的波段号
	int bandG = ui->band2comboBox->currentIndex() + 1;
	int bandB = ui->band3comboBox->currentIndex() + 1;

	if (selectedLayer.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("No raster layer selected!"));
		return;
	}

	// 获取选中的栅格图层
	const auto& rasterItems = mpMYGIS->getmvRasterItems();
	auto it = rasterItems.find(selectedLayer);
	if (it == rasterItems.end()) {
		QMessageBox::warning(this, tr("Warning"), tr("Selected raster layer not found!"));
		return;
	}
	ui->progressBar->setValue(20);  // 更新进度条
	// 获取图层的 RasterData 对象
	RasterData* rasterData = mpMYGIS->getRasterData(selectedLayer);
	if (!rasterData) {
		QMessageBox::warning(this, tr("Warning"), tr("Failed to get raster data!"));
		return;
	}
	ui->progressBar->setValue(40);  // 更新进度条
	// 创建新的 QImage 并根据选定的波段组合进行颜色映射
	QImage image = createImageFromBands(rasterData, bandR, bandG, bandB);

	// 重新绘制并更新图层显示
	QPixmap pixmap = QPixmap::fromImage(image);
	for (auto* rasterItem : it->second) {
		rasterItem->setPixmap(pixmap);
		rasterItem->update(); // 更新显示
	}

	ui->progressBar->setValue(100);  // 更新进度条

	mpMYGIS->getmpGraphicsView()->update(); // 刷新视图

	// 显示成功提示框
	QMessageBox::information(this, tr("Success"), tr("假彩色显示成功!"));

}


QImage RasterFalseColorWindow::createImageFromBands(RasterData* rasterData, int bandR, int bandG, int bandB) {
	size_t rows = rasterData->getRows();
	size_t cols = rasterData->getCols();
	size_t perPixSize = rasterData->getPerPixSize();

	QImage image(cols, rows, QImage::Format_RGB888);

	for (size_t row = 0; row < rows; ++row) {
		for (size_t col = 0; col < cols; ++col) {
			int offsetR = (row * cols + col) * perPixSize * rasterData->getBandnum() + (bandR - 1) * perPixSize;
			int offsetG = (row * cols + col) * perPixSize * rasterData->getBandnum() + (bandG - 1) * perPixSize;
			int offsetB = (row * cols + col) * perPixSize * rasterData->getBandnum() + (bandB - 1) * perPixSize;

			int r = static_cast<int>(rasterData->getImgData()[offsetR]);
			int g = static_cast<int>(rasterData->getImgData()[offsetG]);
			int b = static_cast<int>(rasterData->getImgData()[offsetB]);

			image.setPixel(col, row, qRgb(r, g, b));
		}
	}

	return image;
}