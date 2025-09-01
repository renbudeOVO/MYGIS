#include "FileBrowserWidget.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QCoreApplication>

// 构造函数：初始化 FileBrowserWidget 类
FileBrowserWidget::FileBrowserWidget(QWidget* parent)
	: QWidget(parent) // 调用父类构造函数
{
	// 创建文件系统模型，用于管理和显示文件系统中的数据
	mpFileSystemModel = new QFileSystemModel(this);
	// 设置模型的根目录为系统的根路径
	mpFileSystemModel->setRootPath(QDir::rootPath());

	// 创建树形视图，用于显示文件和文件夹的层次结构
	mpTreeView = new QTreeView(this);
	// 设置树形视图的模型为文件系统模型
	mpTreeView->setModel(mpFileSystemModel);

	// 获取当前程序目录的上上一级目录
	QDir currentDir = QDir(QCoreApplication::applicationDirPath());
	currentDir.cdUp(); // 上一级目录
	currentDir.cdUp(); // 上上一级目录

	// 设置树形视图的根索引为上上一级目录
	mpTreeView->setRootIndex(mpFileSystemModel->index(currentDir.absolutePath()));

	// 创建垂直布局，用于管理树形视图在窗口中的布局
	QVBoxLayout* layout = new QVBoxLayout(this);
	// 将树形视图添加到布局中
	layout->addWidget(mpTreeView);
	// 将布局设置为当前窗口的主布局
	setLayout(layout);

	// 连接树形视图的双击信号到槽函数 onFileDoubleClicked
	connect(mpTreeView, &QTreeView::doubleClicked, this, &FileBrowserWidget::onFileDoubleClicked);
}

// 处理文件双击事件
void FileBrowserWidget::onFileDoubleClicked(const QModelIndex& index)
{
	// 获取被双击文件的路径
	QString filePath = mpFileSystemModel->filePath(index);
	// 发出文件双击信号，并传递文件路径
	emit fileDoubleClicked(filePath);
	// 弹出消息框，显示选中文件的路径
	QMessageBox::information(this, tr("File Selected"), tr("File Path: %1").arg(filePath));
}
