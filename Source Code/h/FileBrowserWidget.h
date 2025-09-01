#ifndef FILEBROWSERWIDGET_H
#define FILEBROWSERWIDGET_H

#include <QWidget>
#include <QTreeView>
#include <QFileSystemModel>

// FileBrowserWidget��������ʾ�ļ�ϵͳ�������ļ���˫���¼�
class FileBrowserWidget : public QWidget {
	Q_OBJECT

public:
	// ���ܸ����ָ����Ϊ����
	explicit FileBrowserWidget(QWidget* parent = nullptr);

signals:
	// �źţ����ļ���˫��ʱ�����������ļ�·��
	void fileDoubleClicked(const QString& filePath);

private slots:
	// �����ļ�˫���¼������ݱ�˫�����ļ����������ź�
	void onFileDoubleClicked(const QModelIndex& index);

private:
	// �ļ�ϵͳ��ͼ��������ʾ�ļ�Ŀ¼�ṹ
	QTreeView* mpTreeView;

	// �ļ�ϵͳģ�ͣ��ṩ�ļ�ϵͳ����
	QFileSystemModel* mpFileSystemModel;
};

#endif // FILEBROWSERWIDGET_H
