#include "CalculateGeometryWindow.h"
#include "LoggerConfig.h"
#include "MYGIS.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QToolButton>
#include <QPushButton>

CalculateGeometryWindow::CalculateGeometryWindow(QWidget* parent, MYGIS* myGIS)
	: QDialog(parent), mpMYGIS(myGIS), ui(new Ui::CalculateGeometryWindowClass)
{
	ui->setupUi(this); // UI �����ʼ��

	// ����Ĭ��ֵ
	ui->shpLinelineEdit->setText("shp_line");
	ui->shpArealineEdit->setText("shp_area");

	if (mpMYGIS) { // ȷ�� mpMYGIS ��Ϊ��
		const auto& layers = mpMYGIS->getVectorLayers();
		for (const auto& layerPair : layers) {
			ui->comboBox->addItem(layerPair.first); // ���ͼ�����Ƶ� comboBox
		}
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("MYGIS pointer is null!"));
	}

	// ���� startpushButton �� cancelpushButton ����Ӧ�Ĳۺ���
	connect(ui->startpushButton, &QPushButton::clicked, this, &CalculateGeometryWindow::startcalculateGeometry);
	connect(ui->cancelpushButton, &QPushButton::clicked, this, &CalculateGeometryWindow::close);
}

CalculateGeometryWindow::~CalculateGeometryWindow()
{
	delete ui;
}

void CalculateGeometryWindow::startcalculateGeometry()
{
	ui->progressBar->setValue(0);  // ���½�����
	QString selectedLayer = ui->comboBox->currentText();
	QString lineField = ui->shpLinelineEdit->text();
	QString areaField = ui->shpArealineEdit->text();

	if (selectedLayer.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("No layer selected!"));
		// ��־��¼δѡ��ͼ������
		logger.warn("startcalculateGeometry: No layer selected.");
		return;
	}
	ui->progressBar->setValue(20);  // ���½�����
	// ���� MYGIS �ķ���������ѡ��ͼ��ļ�����Ϣ
	if (mpMYGIS) {
		ui->progressBar->setValue(30);  // ���½�����
		bool success = mpMYGIS->calculateGeometry(selectedLayer, lineField, areaField);
		ui->progressBar->setValue(100);  // ���½�����
		if (success) {
			QMessageBox::information(this, tr("Success"), tr("���㼸�γɹ�!"));
			// ��־��¼�ɹ����㼸��
			logger.info("startcalculateGeometry: Geometry calculation succeeded for layer %s.", selectedLayer.toStdString().c_str());
		}
		else {
			QMessageBox::critical(this, tr("Error"), tr("���㼸��ʧ��!"));
			// ��־��¼���㼸��ʧ��
			logger.error("startcalculateGeometry: Geometry calculation failed for layer %s.", selectedLayer.toStdString().c_str());
		}
	}
}


