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

	if (mpMYGIS) { // ȷ�� mpMYGIS ��Ϊ��
	const auto& rasterLayers = mpMYGIS->getmvRasterItems();
	for (const auto& layerPair : rasterLayers) {
		ui->RastercomboBox->addItem(layerPair.first); // ���դ��ͼ�����Ƶ� comboBox
	}
	}
	else {
	QMessageBox::critical(this, tr("Error"), tr("MYGIS pointer is null!"));
	}

	// ���Ĭ��ѡ��
	ui->band1comboBox->addItem("����1");
	ui->band1comboBox->addItem("����2");
	ui->band1comboBox->addItem("����3");

	// ����Ĭ��ѡ���ѡ��Ϊ "����1"
	ui->band1comboBox->setCurrentIndex(0); // ������0��ʼ������2��Ӧ����"����3"

	// ���Ĭ��ѡ��
	ui->band2comboBox->addItem("����1");
	ui->band2comboBox->addItem("����2");
	ui->band2comboBox->addItem("����3");

	// ����Ĭ��ѡ���ѡ��Ϊ "����2"
	ui->band2comboBox->setCurrentIndex(1); // ������0��ʼ������2��Ӧ����"����3"

	// ���Ĭ��ѡ��
	ui->band3comboBox->addItem("����1");
	ui->band3comboBox->addItem("����2");
	ui->band3comboBox->addItem("����3");

	// ����Ĭ��ѡ���ѡ��Ϊ "����3"
	ui->band3comboBox->setCurrentIndex(2); // ������0��ʼ������2��Ӧ����"����3"

	//����
	connect(ui->startpushButton, &QPushButton::clicked, this, &RasterFalseColorWindow::startAnalysis);
	//ȡ��
	connect(ui->cancelpushButton, &QPushButton::clicked, this, &RasterFalseColorWindow::close);


}

RasterFalseColorWindow::~RasterFalseColorWindow()
{
	delete ui;
}


void RasterFalseColorWindow::startAnalysis() {
	ui->progressBar->setValue(0);  // ���½�����
	QString selectedLayer = ui->RastercomboBox->currentText();
	int bandR = ui->band1comboBox->currentIndex() + 1; // ������0��ʼ��������Ҫ+1�Ի�ȡ��ȷ�Ĳ��κ�
	int bandG = ui->band2comboBox->currentIndex() + 1;
	int bandB = ui->band3comboBox->currentIndex() + 1;

	if (selectedLayer.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("No raster layer selected!"));
		return;
	}

	// ��ȡѡ�е�դ��ͼ��
	const auto& rasterItems = mpMYGIS->getmvRasterItems();
	auto it = rasterItems.find(selectedLayer);
	if (it == rasterItems.end()) {
		QMessageBox::warning(this, tr("Warning"), tr("Selected raster layer not found!"));
		return;
	}
	ui->progressBar->setValue(20);  // ���½�����
	// ��ȡͼ��� RasterData ����
	RasterData* rasterData = mpMYGIS->getRasterData(selectedLayer);
	if (!rasterData) {
		QMessageBox::warning(this, tr("Warning"), tr("Failed to get raster data!"));
		return;
	}
	ui->progressBar->setValue(40);  // ���½�����
	// �����µ� QImage ������ѡ���Ĳ�����Ͻ�����ɫӳ��
	QImage image = createImageFromBands(rasterData, bandR, bandG, bandB);

	// ���»��Ʋ�����ͼ����ʾ
	QPixmap pixmap = QPixmap::fromImage(image);
	for (auto* rasterItem : it->second) {
		rasterItem->setPixmap(pixmap);
		rasterItem->update(); // ������ʾ
	}

	ui->progressBar->setValue(100);  // ���½�����

	mpMYGIS->getmpGraphicsView()->update(); // ˢ����ͼ

	// ��ʾ�ɹ���ʾ��
	QMessageBox::information(this, tr("Success"), tr("�ٲ�ɫ��ʾ�ɹ�!"));

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