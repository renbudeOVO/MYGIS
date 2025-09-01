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
	ui->setupUi(this); // UI 初始化

	// 添加默认选项 "所有属性" 到 analysecomboBox
	ui->AttributecomboBox->addItem(tr("所有属性"));

	// 从 MYGIS 获取图层管理中的图层名称，并添加到 ComboBox 中
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
	// 连接 startpushButton 和 cancelpushButton 到相应的槽函数
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
		ui->lineEdit->setText(outputFileName); // 将选择的文件路径设置到 lineEdit 中
	}
}

//开始分析操作
void OverlayAnalysisWindow::startOverlayAnalysisCalculate() {
	ui->progressBar->setValue(0);  // 更新进度条
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "YES"); // 支持中文路径
	CPLSetConfigOption("SHAPE_ENCODING", "GBK"); // 设置 Shapefile 编码为 GBK

	QString inputLayerName = ui->inputcomboBox->currentText();
	QString analyseLayerName = ui->analysecomboBox->currentText();
	QString outputFileName = ui->lineEdit->text();
	ui->progressBar->setValue(10);  // 更新进度条

	// 检查是否选择了输入图层、分析图层和输出文件
	if (inputLayerName.isEmpty() || analyseLayerName.isEmpty() || outputFileName.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("Input layer, analyse layer or output file not specified."));
		logger.warn("startOverlayAnalysisCalculate: Input layer, analyse layer or output file not specified.");
		return;
	}
	ui->progressBar->setValue(20);  // 更新进度条

	// 提取文件名并检查是否包含中文字符
	QString _fileName = QFileInfo(outputFileName).fileName();
	for (int i = 0; i < _fileName.length(); ++i) {
		if (_fileName[i].unicode() > 127) { // Unicode值大于127表示非ASCII字符
			QMessageBox::warning(nullptr, tr("Error"), tr("文件名包含中文字符，请使用英文命名并保存."));
			logger.error("startOverlayAnalysisCalculate: Output file name contains non-ASCII characters.");
			return;
		}
	}

	auto inputLayerIt = mpMYGIS->getVectorLayers().find(inputLayerName);
	auto analyseLayerIt = mpMYGIS->getVectorLayers().find(analyseLayerName);
	ui->progressBar->setValue(30);  // 更新进度条

	// 检查输入和分析图层是否存在
	if (inputLayerIt == mpMYGIS->getVectorLayers().end() || analyseLayerIt == mpMYGIS->getVectorLayers().end()) {
		QMessageBox::critical(this, tr("Error"), tr("Selected layer not found."));
		logger.error("startOverlayAnalysisCalculate: Selected layer not found in vector layers.");
		return;
	}
	ui->progressBar->setValue(40);  // 更新进度条

	const VectorLayerData& inputLayerData = inputLayerIt->second;
	const VectorLayerData& analyseLayerData = analyseLayerIt->second;
	ui->progressBar->setValue(50);  // 更新进度条

	// 创建输出 Shapefile
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
	ui->progressBar->setValue(60);  // 更新进度条

	OGRSpatialReference oSRS;
	oSRS.SetWellKnownGeogCS("WGS84");

	OGRLayer* poLayer = poDS->CreateLayer("OverlayResult", &oSRS, wkbUnknown, nullptr);
	if (poLayer == nullptr) {
		QMessageBox::critical(this, tr("Error"), tr("Layer creation failed."));
		logger.error("startOverlayAnalysisCalculate: Layer creation failed in Shapefile %s.", outputFileName.toStdString().c_str());
		GDALClose(poDS);
		return;
	}
	ui->progressBar->setValue(70);  // 更新进度条

	// 添加输入图层的字段到新图层
	auto addFieldsFromLayer = [&](const VectorLayerData& layerData, const std::string& suffix = "") {
		for (const auto& feature : layerData.getFeatures()) {
			const AttributeData& attributes = feature.getAttributes();
			for (const QString& key : attributes.getAttributeKeys()) {
				std::string fieldName = key.toStdString() + suffix;
				// 确保字段名长度不超过10个字符
				if (fieldName.length() > 10) {
					fieldName = fieldName.substr(0, 10);
				}
				OGRFieldDefn oField(fieldName.c_str(), OFTString);  // 根据具体情况调整字段类型
				poLayer->CreateField(&oField);
			}
			break; // 仅需一次循环，字段名不变
		}
		};
	addFieldsFromLayer(inputLayerData);
	addFieldsFromLayer(analyseLayerData, "_analyse");

	ui->progressBar->setValue(80);  // 更新进度条

	// 计算交集并保留属性
	for (const auto& inputFeature : inputLayerData.getFeatures()) {
		OGRGeometry* inputGeometry = mpMYGIS->createOGRGeometryFromGeometryData(inputFeature.getGeometry());
		for (const auto& analyseFeature : analyseLayerData.getFeatures()) {
			OGRGeometry* analyseGeometry = mpMYGIS->createOGRGeometryFromGeometryData(analyseFeature.getGeometry());
			OGRGeometry* intersection = inputGeometry->Intersection(analyseGeometry);
			if (intersection && !intersection->IsEmpty()) {
				OGRFeature* poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());

				// 设置几何
				poFeature->SetGeometry(intersection);

				// 设置输入图层的属性
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

				// 设置分析图层的属性
				const AttributeData& analyseAttributes = analyseFeature.getAttributes();
				for (const QString& key : analyseAttributes.getAttributeKeys()) {
					std::visit(setFieldValue, analyseAttributes.getAttribute(key));
				}

				// 创建要素
				if (poLayer->CreateFeature(poFeature) != OGRERR_NONE) {
					QMessageBox::critical(this, tr("Error"), tr("Failed to create feature in Shapefile."));
					logger.error("startOverlayAnalysisCalculate: Failed to create feature in Shapefile %s.", outputFileName.toStdString().c_str());
					OGRFeature::DestroyFeature(poFeature);
					GDALClose(poDS);
					return;
				}
				OGRFeature::DestroyFeature(poFeature);
				delete intersection;  // 清理内存
			}
		}
	}

	GDALClose(poDS);
	ui->progressBar->setValue(90);  // 更新进度条
	mpMYGIS->addVectorLayer(outputFileName);

	ui->progressBar->setValue(100);  // 更新进度条

	QMessageBox::information(this, tr("Success"), tr("叠加分析成功，生成图层已添加到视图中."));
	logger.info("startOverlayAnalysisCalculate: Overlay analysis completed successfully, output layer %s added to view.", outputFileName.toStdString().c_str());

	// 在保存完 Shapefile 后重置编码配置
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", nullptr); // 还原到默认值
	CPLSetConfigOption("SHAPE_ENCODING", nullptr); // 还原到默认值
}









