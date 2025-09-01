#ifndef MYGIS_H
#define MYGIS_H

#include "VectorLayerData.h" // ��������Ƶ�VectorLayerData���ݽṹ

#include <QtWidgets/QMainWindow>
#include "ui_MYGIS.h"
#include <ogrsf_frmts.h>
#include <map>
#include <QString>
#include "GDALRasterRead.h"

//��ֹqt������������
#ifdef WIN32
#pragma  execution_character_set("utf-8")
#endif

//ǰ������
class MyGraphicsView;
class QTableWidgetItem;
class GeometryItem;//ʸ���ײ��ͼitem
class featuresToShpWindow;//Ҫ��תshapefile����
class CalculateGeometryWindow;//ͳ�Ƽ��δ���
class ConvexHullWindow;//͹�����㴰��
class OverlayAnalysisWindow;//���ӷ�������
class TextSaveVector;//ͼ�㱣��Ϊ�ı���ʽ����
class ProjectManager;//���̹���
class ProgramStatementWindow;//����˵������

class RasterItem;//դ��ײ��ͼitem
class RasterFalseColorWindow;//դ���ɫ����ʾ����
class GrayscaleHistogramWindow;//�Ҷ�ֱ��ͼ����
class EqualizeRasterWindow;//���⻯դ����ʾ����

class MYGIS : public QMainWindow
{
	Q_OBJECT

public:
	 MYGIS(QWidget* parent = nullptr);
	~MYGIS();

	//���ر༭״̬
	bool isEditingMode() const {
		return mEditingMode;
	}

	// ��ȡ mpGraphicsView �ķ���
	 MyGraphicsView* getmpGraphicsView() const {
		return mpGraphicsView;
	}

	// ��ȡ mvSelectedItems �ķ���
	QList<GeometryItem*>& getSelectedItems() {
		return mvSelectedItems;
	}

	// ���һ�����з����Ի�ȡʸ��ͼ����б�
	const std::map<QString, VectorLayerData>& getVectorLayers() const {
		return mvVectorLayer; 
	}

	// ���һ�����з����Ի�ȡդ��ͼ����б�
	const std::map<QString, std::vector<RasterItem*>>& getmvRasterItems() const {
		return mvRasterItems;
	}

	//��ȡ�ض�ͼ��� RasterData ����
	RasterData* getRasterData(const QString& layerName) {
		auto it = mvRasterLayers.find(layerName);
		if (it != mvRasterLayers.end()) {
			return it->second->getRasterData()[layerName];
		}
		return nullptr;
	}

	//��ȡmvRasterLayers
	const std::map<QString, std::shared_ptr<GDALRasterRead>>& getmvRasterLayers() const {
		return mvRasterLayers;
	}

	//����ͼ�����ƴ� mvLayerColors �л�ȡ��ǰͼ�����ɫ��
	QColor getLayerColor(const QString& layerName) const {
		auto it = mvLayerColors.find(layerName);
		if (it != mvLayerColors.end()) {
			return it->second; // ʹ�� it->second ��ȡֵ
		}
		return QColor(); // ����Ҳ�����Ӧ��ͼ����ɫ������һ��Ĭ����ɫ
	}

	
	void UpdateAttributeTables();//�������Ա�
	void updateLayerIcon(QTreeWidgetItem* layerItem, const QString& layerName);//����ͼ��
	QTreeWidgetItem* findLayerItemByName(const QString& layerName);//ͼ�����Ʋ��Ҷ�Ӧ�� QTreeWidgetItem
	bool saveVectorLayerAsShapefile(const QString& layerName, const QString& outputPath);//�ṩһ��Ҫ��תshapefile�Ľӿ�
	//���㼸��
	bool calculateGeometry(const QString& layerName, const QString& lineField, const QString& areaField);
	OGRGeometry* createOGRGeometryFromGeometryData(const GeometryData& geomData);//�� GeometryData ת��Ϊ OGRGeometry
	//��·�����ͼ��
	void addVectorLayer(const QString& filePath);
	// ��ȡ��ǰ��ͼ�е�ͼ����ɫ�� Z ��ֵ
	void storeLayerPropertiesFromView();

	void loadVectorLayerFromProperties(const QString& filePath, int zIndex, const QColor& layerColor);//�򿪹����ļ�����ʸ��
	void loadRasterLayerFromProperties(const QString& filePath, int zIndex);//�򿪹����ļ�����դ��

	//���̱���
	ProjectManager* mpProjectManager;

	QPointF mapRasterToViewCoordinates(QGraphicsScene* scene, double x, double y);//����դ��Y�ᷭת
	QImage createImageFromRasterData(RasterData* rasterData);//��դ������תΪimage
	void addRasterToManage(const QPixmap& pixmap, const QString& layerName, const double* geoTransform, int rows, int cols);//��ӷ������դ����й���
	
private slots:
	void saveProject();//���湤��
	void openProject();//�򿪹���


	void showFeaturesToShpWindow();//��Ҫ��תshapefile����
	void showCalculateGeometryWindow();//ͳ�Ƽ��δ���
	void showConvexHullWindow();//͹�����㴰��
	void showOverlayAnalysisWindow();//���ӷ�������
	void showTextSaveVector();//ͼ�㱣��Ϊ�ı���ʽ����
	void showRasterFalseColorWindow();//դ���ɫ����ʾ����
	void showGrayscaleHistogramWindow();//�Ҷ�ֱ��ͼ����
	void showEqualizeRasterWindow();//���⻯դ����ʾ����
	void showProgramStatementWindow();//����˵������

	void openVectorLayer(); // ���ڴ����ʸ��ͼ��Ĳۺ���
	void viewCoordinateChanged(QPointF scenePos);//����ƶ���ʾ��γ��

	void showLayerContextMenu(const QPoint& pos); // ��ʾͼ�������Ĳ˵��Ĳۺ���
	void layerDisplays(QTreeWidgetItem* item, int column);//��ѡ��Ӱ�غ���ʾͼ��
	void zoomToLayer(QTreeWidgetItem* item); // ���ŵ�ͼ��Ĳۺ���
	void removeLayer(QTreeWidgetItem* item); // �Ƴ�ͼ��Ĳۺ���
	void showLayerAttributeData(QTreeWidgetItem* item);//ͼ�����Ա�ۺ���
	void onAttributeItemSelected(QTableWidgetItem* item, const QString& layerName);   // �����µļ�����
	void layerMoveToTop(QTreeWidgetItem* item); // ͼ�����Ƶ�����
	void layerMoveToBottom(QTreeWidgetItem* item); // ͼ�����Ƶ��ײ�
	void layerColorSystem();//ͼ����ɫ
	void updateLayerFeatureCount(QTreeWidgetItem* item);	// ����ͼ��Ҫ��������ʾ


	void toggleEditingMode(); // �л��༭ģʽ�Ĳۺ���
	void deleteSelectedItems(); // ɾ����ѡҪ�صĲۺ���
	void withdrawDelete();// ����ɾ�������Ĳۺ���
	void saveEdit();// ����༭�����Ĳۺ���
	void endEdit(); // �˳��༭�����Ĳۺ���

	void resetView();//���ó�����ͼ��λ��
	void startPreviewMode();//��ʼԤ��ģʽ
	void endPreviewMode();//����Ԥ��ģʽ

	
	void openRasterLayer();//�򿪲���ʾդ��
	void openBigTif();//�򿪳���դ��
	
	void showAuthorStatementMessage();//��������
private:
	Ui::MYGISClass ui;

	featuresToShpWindow* mpfeaturesToShpWindow;//Ҫ��תshapefile����
	CalculateGeometryWindow* mpCalculateGeometryWindow;//ͳ�Ƽ��δ���
	ConvexHullWindow* mpConvexHullWindow;//͹�����㴰��
	OverlayAnalysisWindow* mpOverlayAnalysisWindow;//���ӷ�������
	TextSaveVector* mpTextSaveVector;//ͼ�㱣��Ϊ�ı���ʽ����
	RasterFalseColorWindow* mpRasterFalseColorWindow;//դ���ɫ����ʾ����
	GrayscaleHistogramWindow* mpGrayscaleHistogramWindow;//�Ҷ�ֱ��ͼ����
	EqualizeRasterWindow* mpEqualizeRasterWindow;//���⻯դ����ʾ����
	ProgramStatementWindow* mpProgramStatementWindow;//����˵������

	void InitializeMapView();//��ʼ����ͼ��ͼ
	
	QGraphicsScene* mpGraphicsScene; // ��������
	MyGraphicsView* mpGraphicsView; // ʹ���Զ���� MyGraphicsView

	std::map<QString, QColor> mvLayerColors;//ͼ����ɫ
	std::map<QString, VectorLayerData> mvVectorLayer; // ���ڴ洢���ʸ��ͼ��
	void readVectorLayer();//��ȡʸ��

	QTreeWidget* mpLayerTreeWidget; // ����ָ�� LayertreeWidget
	void addLayerToTreeWidget(const QString& layerName, bool isRasterLayer);//���ͼ�㵽ͼ����

	bool mEditingMode = false; // �Ƿ��ڱ༭ģʽ
	bool mIsSaved = false; // ���ڱ�ʶ�༭�����Ƿ��Ѿ�����
	QList<GeometryItem*> mvSelectedItems;  // �洢��ѡ�е�Ҫ��
	QList<FeatureData> mvDeletedFeatures;  // ���ڴ洢��ɾ���� FeatureData ����
	QList<GeometryItem*> mvDeletedItems; // ��ʱ��ű�ɾ����Ҫ�أ����ڳ���


	// �� MYGIS �������һ�� std::map ������դ��ͼ��
	std::map<QString, std::shared_ptr<GDALRasterRead>> mvRasterLayers;
	// �� MYGIS.h ����� mRasterItems ������
	std::map<QString, std::vector<RasterItem*>> mvRasterItems;


};

#endif