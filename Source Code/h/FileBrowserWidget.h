#ifndef FILEBROWSERWIDGET_H
#define FILEBROWSERWIDGET_H

#include <QWidget>
#include <QTreeView>
#include <QFileSystemModel>

// FileBrowserWidget类用于显示文件系统并处理文件的双击事件
class FileBrowserWidget : public QWidget {
	Q_OBJECT

public:
	// 接受父组件指针作为参数
	explicit FileBrowserWidget(QWidget* parent = nullptr);

signals:
	// 信号：当文件被双击时发出，传递文件路径
	void fileDoubleClicked(const QString& filePath);

private slots:
	// 处理文件双击事件，根据被双击的文件索引发出信号
	void onFileDoubleClicked(const QModelIndex& index);

private:
	// 文件系统视图，用于显示文件目录结构
	QTreeView* mpTreeView;

	// 文件系统模型，提供文件系统数据
	QFileSystemModel* mpFileSystemModel;
};

#endif // FILEBROWSERWIDGET_H
