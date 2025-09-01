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
	ui->setupUi(this); //  UI �����ʼ��

	if (mpMYGIS) { // ȷ�� mpMYGIS ��Ϊ��
		const auto& layers = mpMYGIS->getVectorLayers();
		for (const auto& layerPair : layers) {
			ui->comboBox->addItem(layerPair.first); // ���ͼ�����Ƶ� comboBox
		}
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("MYGIS pointer is null!"));
	}

	// ���� toolButton �ĵ���¼����ۺ���
	connect(ui->toolButton, &QToolButton::clicked, this, &TextSaveVector::selectSavePath);

	// ���� savepushButton �ĵ���¼����ۺ���
	connect(ui->savepushButton, &QPushButton::clicked, this, &TextSaveVector::saveLayerToCSV);

	// ���� cancelpushButton �ĵ���¼��Թرմ���
	connect(ui->cancelpushButton, &QPushButton::clicked, this, &TextSaveVector::close);
}

TextSaveVector::~TextSaveVector()
{
	delete ui;
}

//����·������
void TextSaveVector::selectSavePath()
{
	QString filePath = QFileDialog::getSaveFileName(this, tr("Save Layer To Text"), "", tr("Shapefile (*.csv)"));
	if (!filePath.isEmpty()) {
		ui->lineEdit->setText(filePath);
	}
}

//ͼ�㱣��Ϊ�ı���ʽ
void TextSaveVector::saveLayerToCSV()
{
	ui->progressBar->setValue(0);  // ���½�����
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
	ui->progressBar->setValue(10);  // ���½�����
	const VectorLayerData& layerData = it->second;

	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::critical(this, tr("Error"), tr("Unable to open file for writing."));
		return;
	}
	ui->progressBar->setValue(30);  // ���½�����

	QTextStream out(&file);
	out.setCodec("UTF-8"); // �����ı����ı���Ϊ UTF-8

	// д��CSV�ı�����
	out << "WKT";
	for (const QString& key : layerData.getFeatures().at(0).getAttributeKeys()) {
		out << "," << key;
	}
	out << "\n";
	ui->progressBar->setValue(50);  // ���½�����
	// ����ͼ�������е�ÿ��Ҫ��
	for (const FeatureData& feature : layerData.getFeatures()) {
		OGRGeometry* ogrGeometry = mpMYGIS->createOGRGeometryFromGeometryData(feature.getGeometry());
		if (ogrGeometry) {
			char* wkt = nullptr;
			ogrGeometry->exportToWkt(&wkt);

			// д�뼸�����ݣ�WKT��ʽ��
			out << "\"" << wkt << "\"";  // WKT����Ӧ����Ϊһ���ַ����洢

			// д����������
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
	ui->progressBar->setValue(100);  // ���½�����
	file.close();
	QMessageBox::information(this, tr("Success"), tr("ͼ���ѳɹ�����Ϊ CSV�ļ�."));
}



