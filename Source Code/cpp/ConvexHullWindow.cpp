#include "ConvexHullWindow.h"
#include "LoggerConfig.h"
#include "MYGIS.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QToolButton>
#include <QPushButton>
#include <ogrsf_frmts.h>  // 包含 GDAL OGR 函数

ConvexHullWindow::ConvexHullWindow(QWidget* parent, MYGIS* myGIS)
	: QDialog(parent), mpMYGIS(myGIS), ui(new Ui::ConvexHullWindowClass)
{
	ui->setupUi(this); // UI 初始化

	if (mpMYGIS) {
		const auto& layers = mpMYGIS->getVectorLayers();
		for (const auto& layerPair : layers) {
			ui->comboBox->addItem(layerPair.first); // 添加图层名称到 comboBox
		}
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("MYGIS pointer is null!"));
	}

	connect(ui->startpushButton, &QPushButton::clicked, this, &ConvexHullWindow::startConvexHullCalculate);
	connect(ui->cancelpushButton, &QPushButton::clicked, this, &ConvexHullWindow::close);

	// 连接 toolButton 点击事件到槽函数
	connect(ui->toolButton, &QToolButton::clicked, this, &ConvexHullWindow::selectOutputFilePath);
}

ConvexHullWindow::~ConvexHullWindow() {
	delete ui;
}

void ConvexHullWindow::selectOutputFilePath() {
	QString outputFileName = QFileDialog::getSaveFileName(this, tr("Save Convex Hull"), "", tr("Shapefile (*.shp)"));
	if (!outputFileName.isEmpty()) {
		ui->lineEdit->setText(outputFileName); // 将选择的文件路径设置到 lineEdit 中
	}
}

//包络矩形转面
void ConvexHullWindow::startConvexHullCalculate() {
	ui->progressBar->setValue(0);  // 更新进度条
	QString inputLayerName = ui->comboBox->currentText();
	QString outputFileName = ui->lineEdit->text(); // 从 lineEdit 获取输出文件路径

	if (inputLayerName.isEmpty() || outputFileName.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("Input layer or output file not specified."));
		logger.warn("startConvexHullCalculate: Input layer or output file not specified.");
		return;
	}

	auto it = mpMYGIS->getVectorLayers().find(inputLayerName);
	if (it == mpMYGIS->getVectorLayers().end()) {
		QMessageBox::critical(this, tr("Error"), tr("Selected layer not found."));
		logger.error("startConvexHullCalculate: Selected layer %s not found.", inputLayerName.toStdString().c_str());
		return;
	}
	ui->progressBar->setValue(10);  // 更新进度条
	const VectorLayerData& layerData = it->second;
	std::vector<OGRGeometry*> groupedGeometries;

	// 将每个要素的 GeometryData 转换为 OGRGeometry 并加入分组
	for (const auto& feature : layerData.getFeatures()) {
		OGRGeometry* poGeometry = mpMYGIS->createOGRGeometryFromGeometryData(feature.getGeometry());  // 使用整合后的函数
		if (poGeometry) {
			groupedGeometries.push_back(poGeometry);
		}
	}
	ui->progressBar->setValue(20);  // 更新进度条
	// 计算分组要素的包围矩形
	std::vector<OGRGeometry*> boundingBoxes;
	for (auto& geometry : groupedGeometries) {
		if (geometry) {
			OGREnvelope envelope;
			geometry->getEnvelope(&envelope);

			// 构建包围矩形
			OGRPolygon* boundingBox = new OGRPolygon();
			OGRLinearRing* ring = new OGRLinearRing();
			ring->addPoint(envelope.MinX, envelope.MinY);
			ring->addPoint(envelope.MaxX, envelope.MinY);
			ring->addPoint(envelope.MaxX, envelope.MaxY);
			ring->addPoint(envelope.MinX, envelope.MaxY);
			ring->addPoint(envelope.MinX, envelope.MinY);  // 封闭环
			boundingBox->addRing(ring);

			boundingBoxes.push_back(boundingBox);
		}
	}
	ui->progressBar->setValue(40);  // 更新进度条
	// 创建输出 Shapefile
	const char* pszDriverName = "ESRI Shapefile";
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
	if (poDriver == nullptr) {
		QMessageBox::critical(this, tr("Error"), tr("ESRI Shapefile driver not available."));
		logger.error("startConvexHullCalculate: ESRI Shapefile driver not available.");
		return;
	}

	GDALDataset* poDS = poDriver->Create(outputFileName.toStdString().c_str(), 0, 0, 0, GDT_Unknown, nullptr);
	if (poDS == nullptr) {
		QMessageBox::critical(this, tr("Error"), tr("Failed to create Shapefile."));
		logger.error("startConvexHullCalculate: Failed to create Shapefile %s.", outputFileName.toStdString().c_str());
		return;
	}
	ui->progressBar->setValue(50);  // 更新进度条
	OGRLayer* poLayer = poDS->CreateLayer("BoundingBox", nullptr, wkbPolygon, nullptr);
	if (poLayer == nullptr) {
		QMessageBox::critical(this, tr("Error"), tr("Layer creation failed."));
		logger.error("startConvexHullCalculate: Layer creation failed in Shapefile %s.", outputFileName.toStdString().c_str());
		GDALClose(poDS);
		return;
	}
	ui->progressBar->setValue(70);  // 更新进度条
	// 输出包围矩形到 Shapefile
	for (auto& boundingBox : boundingBoxes) {
		OGRFeature* poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
		poFeature->SetGeometry(boundingBox);
		if (poLayer->CreateFeature(poFeature) != OGRERR_NONE) {
			QMessageBox::critical(this, tr("Error"), tr("Failed to create feature in Shapefile."));
			logger.error("startConvexHullCalculate: Failed to create feature in Shapefile %s.", outputFileName.toStdString().c_str());
			OGRFeature::DestroyFeature(poFeature);
			GDALClose(poDS);
			return;
		}
		OGRFeature::DestroyFeature(poFeature);
		delete boundingBox;  // 清理内存
	}
	ui->progressBar->setValue(80);  // 更新进度条
	GDALClose(poDS);

	mpMYGIS->addVectorLayer(outputFileName);  // 将生成的 Shapefile 添加到项目中
	logger.info("startConvexHullCalculate: Convex hull calculation completed and added to project. Output file: %s.", outputFileName.toStdString().c_str());

	ui->progressBar->setValue(100);  // 更新进度条
	QMessageBox::information(this, tr("Success"), tr("凸包运算已完成，图层已加入当前视图."));
}









