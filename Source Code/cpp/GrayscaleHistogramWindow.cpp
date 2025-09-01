#include "GrayscaleHistogramWindow.h"
#include "MYGIS.h"
#include "qcustomplot.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QToolButton>
#include <QPushButton>
#include <QPixmap>

GrayscaleHistogramWindow::GrayscaleHistogramWindow(QWidget* parent, MYGIS* myGIS)
	: QDialog(parent), mpMYGIS(myGIS), ui(new Ui::GrayscaleHistogramWindowClass)
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

	connect(ui->startpushButton, &QPushButton::clicked, this, &GrayscaleHistogramWindow::startGrayscaleHistogram);
	connect(ui->cancelpushButton, &QPushButton::clicked, this, &GrayscaleHistogramWindow::close);

	// 连接 toolButton 点击事件到槽函数
	connect(ui->toolButton, &QToolButton::clicked, this, &GrayscaleHistogramWindow::selectOutputFilePath);
}

GrayscaleHistogramWindow::~GrayscaleHistogramWindow()
{
	delete ui;
}

//保存路径
void GrayscaleHistogramWindow::selectOutputFilePath() {
	QString outputFileName = QFileDialog::getSaveFileName(this, tr("Save Histogram"), "", tr("PNG Files (*.png)"));
	if (!outputFileName.isEmpty()) {
		ui->lineEdit->setText(outputFileName); // 将选择的文件路径设置到 lineEdit 中
	}
}

void GrayscaleHistogramWindow::startGrayscaleHistogram() {
	QString selectedLayer = ui->comboBox->currentText();
	QString outputFilePath = ui->lineEdit->text();

	if (selectedLayer.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("No raster layer selected!"));
		return;
	}

	if (outputFilePath.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("No output file path selected!"));
		return;
	}

	// 获取选中的栅格图层数据
	auto rasterLayer = mpMYGIS->getRasterData(selectedLayer);
	if (!rasterLayer) {
		QMessageBox::warning(this, tr("Warning"), tr("Failed to get raster data!"));
		return;
	}

	// 获取每个波段的直方图并绘制
	for (size_t bandIndex = 1; bandIndex <= rasterLayer->getBandnum(); ++bandIndex) {
		const std::vector<int>& histogram = rasterLayer->getHistogram(bandIndex);
		if (!histogram.empty()) {
			QString bandName = QString("band%1").arg(bandIndex);
			QString bandFilePath = outputFilePath.left(outputFilePath.lastIndexOf('.')) + "_" + bandName + ".png";
			plotHistogram(histogram, bandName, bandFilePath);
		}
	}
}

void GrayscaleHistogramWindow::plotHistogram(const std::vector<int>& histogram, const QString& bandName, const QString& outputFilePath) {
	ui->progressBar->setValue(0);  // 更新进度条

	// 创建 QCustomPlot 对象
	QCustomPlot customPlot;

	// 生成 x 轴和 y 轴数据
	QVector<double> x(histogram.size()), y(histogram.size());
	for (int i = 0; i < histogram.size(); ++i) {
		x[i] = i; // 灰度值
		y[i] = histogram[i]; // 频率
	}

	// 添加柱状图
	QCPBars* bars = new QCPBars(customPlot.xAxis, customPlot.yAxis);
	bars->setData(x, y);
	ui->progressBar->setValue(20);  // 更新进度条
	// 设置轴标签
	customPlot.xAxis->setLabel("灰度值");
	customPlot.yAxis->setLabel("频率");
	ui->progressBar->setValue(40);  // 更新进度条
	// 设置轴范围
	customPlot.xAxis->setRange(0, histogram.size() - 1);
	customPlot.yAxis->setRange(0, *std::max_element(histogram.begin(), histogram.end()));
	ui->progressBar->setValue(60);  // 更新进度条
	// 绘制直方图
	customPlot.replot();
	ui->progressBar->setValue(80);  // 更新进度条
	// 将图形保存为 PNG 文件
	customPlot.savePng(outputFilePath, 800, 600);
	ui->progressBar->setValue(100);  // 更新进度条
	QMessageBox::information(this, tr("Success"), tr("直方图 %1 保存成功!").arg(bandName));
}
