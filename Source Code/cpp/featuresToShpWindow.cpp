#include "featuresToShpWindow.h"
#include "LoggerConfig.h"
#include "MYGIS.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QToolButton>
#include <QPushButton>

featuresToShpWindow::featuresToShpWindow(QWidget* parent, MYGIS* myGIS)
	: QDialog(parent), mpMYGIS(myGIS), ui(new Ui::featuresToShpWindowClass)
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
	connect(ui->toolButton, &QToolButton::clicked, this, &featuresToShpWindow::selectSavePath);

	// ���� savepushButton �ĵ���¼����ۺ���
	connect(ui->savepushButton, &QPushButton::clicked, this, &featuresToShpWindow::saveLayerToShapefile);

	// ���� cancelpushButton �ĵ���¼��Թرմ���
	connect(ui->cancelpushButton, &QPushButton::clicked, this, &featuresToShpWindow::close);
}

featuresToShpWindow::~featuresToShpWindow()
{
	delete ui;
}

//����·������
void featuresToShpWindow::selectSavePath()
{
	QString filePath = QFileDialog::getSaveFileName(this, tr("Save Shapefile"), "", tr("Shapefile (*.shp)"));
	if (!filePath.isEmpty()) {
		ui->lineEdit->setText(filePath);
	}
}

//Ҫ��תshapefile
void featuresToShpWindow::saveLayerToShapefile()
{
	ui->progressBar->setValue(0);  // ���½�����
	QString layerName = ui->comboBox->currentText();
	QString savePath = ui->lineEdit->text();

	if (layerName.isEmpty() || savePath.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("Please select a layer and specify a save path."));
		logger.warn("saveLayerToShapefile: No layer selected or save path not specified.");
		return;
	}
	ui->progressBar->setValue(20);  // ���½�����

	// ���� GIS ϵͳ�ĺ�������ͼ��Ϊ Shapefile
	bool success = mpMYGIS->saveVectorLayerAsShapefile(layerName, savePath);
	ui->progressBar->setValue(100);  // ���½�����

	if (success) {
		QMessageBox::information(this, tr("Success"), tr("�ɹ�����Ϊshapefile!"));
		logger.info("saveLayerToShapefile: Layer %s successfully saved as Shapefile at %s.", layerName.toStdString().c_str(), savePath.toStdString().c_str());
		close();
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("Failed to save the layer."));
		logger.error("saveLayerToShapefile: Failed to save layer %s as Shapefile.", layerName.toStdString().c_str());
	}
}

