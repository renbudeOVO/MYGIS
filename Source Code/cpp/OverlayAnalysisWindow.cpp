#include "OverlayAnalysisWindow.h"
#include "LoggerConfig.h"
#include "MYGIS.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QToolButton>
#include <QPushButton>

OverlayAnalysisWindow::OverlayAnalysisWindow(QWidget *parent, MYGIS* myGIS)
	: QDialog(parent), mpMYGIS(myGIS), ui(new Ui::OverlayAnalysisWindowClass)
{
	ui->setupUi(this); // UI ��ʼ��

	// ���Ĭ��ѡ�� "��������" �� analysecomboBox
	ui->AttributecomboBox->addItem(tr("��������"));

	// �� MYGIS ��ȡͼ������е�ͼ�����ƣ�����ӵ� ComboBox ��
	if (mpMYGIS) {
		const auto& layers = mpMYGIS->getVectorLayers();
		for (const auto& layerPair : layers) {
			ui->inputcomboBox->addItem(layerPair.first);
			ui->analysecomboBox->addItem(layerPair.first);
		}
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("MYGIS pointer is null!"));
	}
	// ���� startpushButton �� cancelpushButton ����Ӧ�Ĳۺ���
	connect(ui->toolButton, &QToolButton::clicked, this, &OverlayAnalysisWindow::selectOutputFilePath);
	connect(ui->startpushButton, &QPushButton::clicked, this, &OverlayAnalysisWindow::startOverlayAnalysisCalculate);
	connect(ui->cancelpushButton, &QPushButton::clicked, this, &OverlayAnalysisWindow::close);
}

OverlayAnalysisWindow::~OverlayAnalysisWindow()
{
	delete ui;
}

void OverlayAnalysisWindow::selectOutputFilePath() {
	QString outputFileName = QFileDialog::getSaveFileName(this, tr("Save Overlay Result"), "", tr("Shapefile (*.shp)"));
	if (!outputFileName.isEmpty()) {
		ui->lineEdit->setText(outputFileName); // ��ѡ����ļ�·�����õ� lineEdit ��
	}
}

//��ʼ��������
void OverlayAnalysisWindow::startOverlayAnalysisCalculate() {
	ui->progressBar->setValue(0);  // ���½�����
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "YES"); // ֧������·��
	CPLSetConfigOption("SHAPE_ENCODING", "GBK"); // ���� Shapefile ����Ϊ GBK

	QString inputLayerName = ui->inputcomboBox->currentText();
	QString analyseLayerName = ui->analysecomboBox->currentText();
	QString outputFileName = ui->lineEdit->text();
	ui->progressBar->setValue(10);  // ���½�����

	// ����Ƿ�ѡ��������ͼ�㡢����ͼ�������ļ�
	if (inputLayerName.isEmpty() || analyseLayerName.isEmpty() || outputFileName.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("Input layer, analyse layer or output file not specified."));
		logger.warn("startOverlayAnalysisCalculate: Input layer, analyse layer or output file not specified.");
		return;
	}
	ui->progressBar->setValue(20);  // ���½�����

	// ��ȡ�ļ���������Ƿ���������ַ�
	QString _fileName = QFileInfo(outputFileName).fileName();
	for (int i = 0; i < _fileName.length(); ++i) {
		if (_fileName[i].unicode() > 127) { // Unicodeֵ����127��ʾ��ASCII�ַ�
			QMessageBox::warning(nullptr, tr("Error"), tr("�ļ������������ַ�����ʹ��Ӣ������������."));
			logger.error("startOverlayAnalysisCalculate: Output file name contains non-ASCII characters.");
			return;
		}
	}

	auto inputLayerIt = mpMYGIS->getVectorLayers().find(inputLayerName);
	auto analyseLayerIt = mpMYGIS->getVectorLayers().find(analyseLayerName);
	ui->progressBar->setValue(30);  // ���½�����

	// �������ͷ���ͼ���Ƿ����
	if (inputLayerIt == mpMYGIS->getVectorLayers().end() || analyseLayerIt == mpMYGIS->getVectorLayers().end()) {
		QMessageBox::critical(this, tr("Error"), tr("Selected layer not found."));
		logger.error("startOverlayAnalysisCalculate: Selected layer not found in vector layers.");
		return;
	}
	ui->progressBar->setValue(40);  // ���½�����

	const VectorLayerData& inputLayerData = inputLayerIt->second;
	const VectorLayerData& analyseLayerData = analyseLayerIt->second;
	ui->progressBar->setValue(50);  // ���½�����

	// ������� Shapefile
	const char* pszDriverName = "ESRI Shapefile";
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
	if (poDriver == nullptr) {
		QMessageBox::critical(this, tr("Error"), tr("ESRI Shapefile driver not available."));
		logger.error("startOverlayAnalysisCalculate: ESRI Shapefile driver not available.");
		return;
	}

	GDALDataset* poDS = poDriver->Create(outputFileName.toStdString().c_str(), 0, 0, 0, GDT_Unknown, nullptr);
	if (poDS == nullptr) {
		QMessageBox::critical(this, tr("Error"), tr("Failed to create Shapefile."));
		logger.error("startOverlayAnalysisCalculate: Failed to create Shapefile %s.", outputFileName.toStdString().c_str());
		return;
	}
	ui->progressBar->setValue(60);  // ���½�����

	OGRSpatialReference oSRS;
	oSRS.SetWellKnownGeogCS("WGS84");

	OGRLayer* poLayer = poDS->CreateLayer("OverlayResult", &oSRS, wkbUnknown, nullptr);
	if (poLayer == nullptr) {
		QMessageBox::critical(this, tr("Error"), tr("Layer creation failed."));
		logger.error("startOverlayAnalysisCalculate: Layer creation failed in Shapefile %s.", outputFileName.toStdString().c_str());
		GDALClose(poDS);
		return;
	}
	ui->progressBar->setValue(70);  // ���½�����

	// �������ͼ����ֶε���ͼ��
	auto addFieldsFromLayer = [&](const VectorLayerData& layerData, const std::string& suffix = "") {
		for (const auto& feature : layerData.getFeatures()) {
			const AttributeData& attributes = feature.getAttributes();
			for (const QString& key : attributes.getAttributeKeys()) {
				std::string fieldName = key.toStdString() + suffix;
				// ȷ���ֶ������Ȳ�����10���ַ�
				if (fieldName.length() > 10) {
					fieldName = fieldName.substr(0, 10);
				}
				OGRFieldDefn oField(fieldName.c_str(), OFTString);  // ���ݾ�����������ֶ�����
				poLayer->CreateField(&oField);
			}
			break; // ����һ��ѭ�����ֶ�������
		}
		};
	addFieldsFromLayer(inputLayerData);
	addFieldsFromLayer(analyseLayerData, "_analyse");

	ui->progressBar->setValue(80);  // ���½�����

	// ���㽻������������
	for (const auto& inputFeature : inputLayerData.getFeatures()) {
		OGRGeometry* inputGeometry = mpMYGIS->createOGRGeometryFromGeometryData(inputFeature.getGeometry());
		for (const auto& analyseFeature : analyseLayerData.getFeatures()) {
			OGRGeometry* analyseGeometry = mpMYGIS->createOGRGeometryFromGeometryData(analyseFeature.getGeometry());
			OGRGeometry* intersection = inputGeometry->Intersection(analyseGeometry);
			if (intersection && !intersection->IsEmpty()) {
				OGRFeature* poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());

				// ���ü���
				poFeature->SetGeometry(intersection);

				// ��������ͼ�������
				int fieldIndex = 0;
				const AttributeData& inputAttributes = inputFeature.getAttributes();
				auto setFieldValue = [&](const auto& value) {
					QString valueString;
					if constexpr (std::is_same_v<std::decay_t<decltype(value)>, int>) {
						valueString = QString::number(value);
					}
					else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, double>) {
						valueString = QString::number(value);
					}
					else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, QString>) {
						valueString = value;
					}
					poFeature->SetField(fieldIndex, valueString.toStdString().c_str());
					fieldIndex++;
					};
				for (const QString& key : inputAttributes.getAttributeKeys()) {
					std::visit(setFieldValue, inputAttributes.getAttribute(key));
				}

				// ���÷���ͼ�������
				const AttributeData& analyseAttributes = analyseFeature.getAttributes();
				for (const QString& key : analyseAttributes.getAttributeKeys()) {
					std::visit(setFieldValue, analyseAttributes.getAttribute(key));
				}

				// ����Ҫ��
				if (poLayer->CreateFeature(poFeature) != OGRERR_NONE) {
					QMessageBox::critical(this, tr("Error"), tr("Failed to create feature in Shapefile."));
					logger.error("startOverlayAnalysisCalculate: Failed to create feature in Shapefile %s.", outputFileName.toStdString().c_str());
					OGRFeature::DestroyFeature(poFeature);
					GDALClose(poDS);
					return;
				}
				OGRFeature::DestroyFeature(poFeature);
				delete intersection;  // �����ڴ�
			}
		}
	}

	GDALClose(poDS);
	ui->progressBar->setValue(90);  // ���½�����
	mpMYGIS->addVectorLayer(outputFileName);

	ui->progressBar->setValue(100);  // ���½�����

	QMessageBox::information(this, tr("Success"), tr("���ӷ����ɹ�������ͼ������ӵ���ͼ��."));
	logger.info("startOverlayAnalysisCalculate: Overlay analysis completed successfully, output layer %s added to view.", outputFileName.toStdString().c_str());

	// �ڱ����� Shapefile �����ñ�������
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", nullptr); // ��ԭ��Ĭ��ֵ
	CPLSetConfigOption("SHAPE_ENCODING", nullptr); // ��ԭ��Ĭ��ֵ
}









