#include "FileBrowserWidget.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QCoreApplication>

// ���캯������ʼ�� FileBrowserWidget ��
FileBrowserWidget::FileBrowserWidget(QWidget* parent)
	: QWidget(parent) // ���ø��๹�캯��
{
	// �����ļ�ϵͳģ�ͣ����ڹ������ʾ�ļ�ϵͳ�е�����
	mpFileSystemModel = new QFileSystemModel(this);
	// ����ģ�͵ĸ�Ŀ¼Ϊϵͳ�ĸ�·��
	mpFileSystemModel->setRootPath(QDir::rootPath());

	// ����������ͼ��������ʾ�ļ����ļ��еĲ�νṹ
	mpTreeView = new QTreeView(this);
	// ����������ͼ��ģ��Ϊ�ļ�ϵͳģ��
	mpTreeView->setModel(mpFileSystemModel);

	// ��ȡ��ǰ����Ŀ¼������һ��Ŀ¼
	QDir currentDir = QDir(QCoreApplication::applicationDirPath());
	currentDir.cdUp(); // ��һ��Ŀ¼
	currentDir.cdUp(); // ����һ��Ŀ¼

	// ����������ͼ�ĸ�����Ϊ����һ��Ŀ¼
	mpTreeView->setRootIndex(mpFileSystemModel->index(currentDir.absolutePath()));

	// ������ֱ���֣����ڹ���������ͼ�ڴ����еĲ���
	QVBoxLayout* layout = new QVBoxLayout(this);
	// ��������ͼ��ӵ�������
	layout->addWidget(mpTreeView);
	// ����������Ϊ��ǰ���ڵ�������
	setLayout(layout);

	// ����������ͼ��˫���źŵ��ۺ��� onFileDoubleClicked
	connect(mpTreeView, &QTreeView::doubleClicked, this, &FileBrowserWidget::onFileDoubleClicked);
}

// �����ļ�˫���¼�
void FileBrowserWidget::onFileDoubleClicked(const QModelIndex& index)
{
	// ��ȡ��˫���ļ���·��
	QString filePath = mpFileSystemModel->filePath(index);
	// �����ļ�˫���źţ��������ļ�·��
	emit fileDoubleClicked(filePath);
	// ������Ϣ����ʾѡ���ļ���·��
	QMessageBox::information(this, tr("File Selected"), tr("File Path: %1").arg(filePath));
}
