#include "TextSaveVector.h"
#include "MYGIS.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QToolButton>
#include <QPushButton>
#include <QDebug>

TextSaveVector::TextSaveVector(QWidget *parent,MYGIS* myGIS)
	: QDialog(parent), mpMYGIS(myGIS), ui(new Ui::TextSaveVectorClass)
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
	connect(ui->toolButton, &QToolButton::clicked, this, &TextSaveVector::selectSavePath);

	// 连接 savepushButton 的点击事件到槽函数
	connect(ui->savepushButton, &QPushButton::clicked, this, &TextSaveVector::saveLayerToCSV);

	// 连接 cancelpushButton 的点击事件以关闭窗口
	connect(ui->cancelpushButton, &QPushButton::clicked, this, &TextSaveVector::close);
}

TextSaveVector::~TextSaveVector()
{
	delete ui;
}

//保存路径函数
void TextSaveVector::selectSavePath()
{
	QString filePath = QFileDialog::getSaveFileName(this, tr("Save Layer To Text"), "", tr("Shapefile (*.csv)"));
	if (!filePath.isEmpty()) {
		ui->lineEdit->setText(filePath);
	}
}

//图层保存为文本格式
void TextSaveVector::saveLayerToCSV()
{
	ui->progressBar->setValue(0);  // 更新进度条
	QString layerName = ui->comboBox->currentText();
	QString filePath = ui->lineEdit->text();

	if (layerName.isEmpty() || filePath.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("Layer name or save path not specified."));
		return;
	}

	auto it = mpMYGIS->getVectorLayers().find(layerName);
	if (it == mpMYGIS->getVectorLayers().end()) {
		QMessageBox::critical(this, tr("Error"), tr("Selected layer not found."));
		return;
	}
	ui->progressBar->setValue(10);  // 更新进度条
	const VectorLayerData& layerData = it->second;

	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::critical(this, tr("Error"), tr("Unable to open file for writing."));
		return;
	}
	ui->progressBar->setValue(30);  // 更新进度条

	QTextStream out(&file);
	out.setCodec("UTF-8"); // 设置文本流的编码为 UTF-8

	// 写入CSV的标题行
	out << "WKT";
	for (const QString& key : layerData.getFeatures().at(0).getAttributeKeys()) {
		out << "," << key;
	}
	out << "\n";
	ui->progressBar->setValue(50);  // 更新进度条
	// 遍历图层数据中的每个要素
	for (const FeatureData& feature : layerData.getFeatures()) {
		OGRGeometry* ogrGeometry = mpMYGIS->createOGRGeometryFromGeometryData(feature.getGeometry());
		if (ogrGeometry) {
			char* wkt = nullptr;
			ogrGeometry->exportToWkt(&wkt);

			// 写入几何数据（WKT格式）
			out << "\"" << wkt << "\"";  // WKT数据应该作为一个字符串存储

			// 写入属性数据
			const AttributeData& attributes = feature.getAttributes();
			for (const QString& key : attributes.getAttributeKeys()) {
				std::visit([&out](auto&& value) {
					using T = std::decay_t<decltype(value)>;
					if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double>) {
						out << "," << value;
					}
					else if constexpr (std::is_same_v<T, QString>) {
						out << "," << value;
					}
					}, attributes.getAttribute(key));
			}
			out << "\n";

			CPLFree(wkt);
		}
	}
	ui->progressBar->setValue(100);  // 更新进度条
	file.close();
	QMessageBox::information(this, tr("Success"), tr("图层已成功保存为 CSV文件."));
}



