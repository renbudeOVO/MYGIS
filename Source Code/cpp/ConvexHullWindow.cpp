#include "ConvexHullWindow.h"
#include "LoggerConfig.h"
#include "MYGIS.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QToolButton>
#include <QPushButton>
#include <ogrsf_frmts.h>  // ���� GDAL OGR ����

ConvexHullWindow::ConvexHullWindow(QWidget* parent, MYGIS* myGIS)
	: QDialog(parent), mpMYGIS(myGIS), ui(new Ui::ConvexHullWindowClass)
{
	ui->setupUi(this); // UI ��ʼ��

	if (mpMYGIS) {
		const auto& layers = mpMYGIS->getVectorLayers();
		for (const auto& layerPair : layers) {
			ui->comboBox->addItem(layerPair.first); // ���ͼ�����Ƶ� comboBox
		}
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("MYGIS pointer is null!"));
	}

	connect(ui->startpushButton, &QPushButton::clicked, this, &ConvexHullWindow::startConvexHullCalculate);
	connect(ui->cancelpushButton, &QPushButton::clicked, this, &ConvexHullWindow::close);

	// ���� toolButton ����¼����ۺ���
	connect(ui->toolButton, &QToolButton::clicked, this, &ConvexHullWindow::selectOutputFilePath);
}

ConvexHullWindow::~ConvexHullWindow() {
	delete ui;
}

void ConvexHullWindow::selectOutputFilePath() {
	QString outputFileName = QFileDialog::getSaveFileName(this, tr("Save Convex Hull"), "", tr("Shapefile (*.shp)"));
	if (!outputFileName.isEmpty()) {
		ui->lineEdit->setText(outputFileName); // ��ѡ����ļ�·�����õ� lineEdit ��
	}
}

//�������ת��
void ConvexHullWindow::startConvexHullCalculate() {
	ui->progressBar->setValue(0);  // ���½�����
	QString inputLayerName = ui->comboBox->currentText();
	QString outputFileName = ui->lineEdit->text(); // �� lineEdit ��ȡ����ļ�·��

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
	ui->progressBar->setValue(10);  // ���½�����
	const VectorLayerData& layerData = it->second;
	std::vector<OGRGeometry*> groupedGeometries;

	// ��ÿ��Ҫ�ص� GeometryData ת��Ϊ OGRGeometry ���������
	for (const auto& feature : layerData.getFeatures()) {
		OGRGeometry* poGeometry = mpMYGIS->createOGRGeometryFromGeometryData(feature.getGeometry());  // ʹ�����Ϻ�ĺ���
		if (poGeometry) {
			groupedGeometries.push_back(poGeometry);
		}
	}
	ui->progressBar->setValue(20);  // ���½�����
	// �������Ҫ�صİ�Χ����
	std::vector<OGRGeometry*> boundingBoxes;
	for (auto& geometry : groupedGeometries) {
		if (geometry) {
			OGREnvelope envelope;
			geometry->getEnvelope(&envelope);

			// ������Χ����
			OGRPolygon* boundingBox = new OGRPolygon();
			OGRLinearRing* ring = new OGRLinearRing();
			ring->addPoint(envelope.MinX, envelope.MinY);
			ring->addPoint(envelope.MaxX, envelope.MinY);
			ring->addPoint(envelope.MaxX, envelope.MaxY);
			ring->addPoint(envelope.MinX, envelope.MaxY);
			ring->addPoint(envelope.MinX, envelope.MinY);  // ��ջ�
			boundingBox->addRing(ring);

			boundingBoxes.push_back(boundingBox);
		}
	}
	ui->progressBar->setValue(40);  // ���½�����
	// ������� Shapefile
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
	ui->progressBar->setValue(50);  // ���½�����
	OGRLayer* poLayer = poDS->CreateLayer("BoundingBox", nullptr, wkbPolygon, nullptr);
	if (poLayer == nullptr) {
		QMessageBox::critical(this, tr("Error"), tr("Layer creation failed."));
		logger.error("startConvexHullCalculate: Layer creation failed in Shapefile %s.", outputFileName.toStdString().c_str());
		GDALClose(poDS);
		return;
	}
	ui->progressBar->setValue(70);  // ���½�����
	// �����Χ���ε� Shapefile
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
		delete boundingBox;  // �����ڴ�
	}
	ui->progressBar->setValue(80);  // ���½�����
	GDALClose(poDS);

	mpMYGIS->addVectorLayer(outputFileName);  // �����ɵ� Shapefile ��ӵ���Ŀ��
	logger.info("startConvexHullCalculate: Convex hull calculation completed and added to project. Output file: %s.", outputFileName.toStdString().c_str());

	ui->progressBar->setValue(100);  // ���½�����
	QMessageBox::information(this, tr("Success"), tr("͹����������ɣ�ͼ���Ѽ��뵱ǰ��ͼ."));
}









