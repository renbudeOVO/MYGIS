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

	if (mpMYGIS) { // ȷ�� mpMYGIS ��Ϊ��
		const auto& rasterLayers = mpMYGIS->getmvRasterItems();
		for (const auto& layerPair : rasterLayers) {
			ui->comboBox->addItem(layerPair.first); // ���դ��ͼ�����Ƶ� comboBox
		}
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("MYGIS pointer is null!"));
	}

	connect(ui->startpushButton, &QPushButton::clicked, this, &GrayscaleHistogramWindow::startGrayscaleHistogram);
	connect(ui->cancelpushButton, &QPushButton::clicked, this, &GrayscaleHistogramWindow::close);

	// ���� toolButton ����¼����ۺ���
	connect(ui->toolButton, &QToolButton::clicked, this, &GrayscaleHistogramWindow::selectOutputFilePath);
}

GrayscaleHistogramWindow::~GrayscaleHistogramWindow()
{
	delete ui;
}

//����·��
void GrayscaleHistogramWindow::selectOutputFilePath() {
	QString outputFileName = QFileDialog::getSaveFileName(this, tr("Save Histogram"), "", tr("PNG Files (*.png)"));
	if (!outputFileName.isEmpty()) {
		ui->lineEdit->setText(outputFileName); // ��ѡ����ļ�·�����õ� lineEdit ��
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

	// ��ȡѡ�е�դ��ͼ������
	auto rasterLayer = mpMYGIS->getRasterData(selectedLayer);
	if (!rasterLayer) {
		QMessageBox::warning(this, tr("Warning"), tr("Failed to get raster data!"));
		return;
	}

	// ��ȡÿ�����ε�ֱ��ͼ������
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
	ui->progressBar->setValue(0);  // ���½�����

	// ���� QCustomPlot ����
	QCustomPlot customPlot;

	// ���� x ��� y ������
	QVector<double> x(histogram.size()), y(histogram.size());
	for (int i = 0; i < histogram.size(); ++i) {
		x[i] = i; // �Ҷ�ֵ
		y[i] = histogram[i]; // Ƶ��
	}

	// �����״ͼ
	QCPBars* bars = new QCPBars(customPlot.xAxis, customPlot.yAxis);
	bars->setData(x, y);
	ui->progressBar->setValue(20);  // ���½�����
	// �������ǩ
	customPlot.xAxis->setLabel("�Ҷ�ֵ");
	customPlot.yAxis->setLabel("Ƶ��");
	ui->progressBar->setValue(40);  // ���½�����
	// �����᷶Χ
	customPlot.xAxis->setRange(0, histogram.size() - 1);
	customPlot.yAxis->setRange(0, *std::max_element(histogram.begin(), histogram.end()));
	ui->progressBar->setValue(60);  // ���½�����
	// ����ֱ��ͼ
	customPlot.replot();
	ui->progressBar->setValue(80);  // ���½�����
	// ��ͼ�α���Ϊ PNG �ļ�
	customPlot.savePng(outputFilePath, 800, 600);
	ui->progressBar->setValue(100);  // ���½�����
	QMessageBox::information(this, tr("Success"), tr("ֱ��ͼ %1 ����ɹ�!").arg(bandName));
}
