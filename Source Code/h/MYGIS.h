#ifndef MYGIS_H
#define MYGIS_H

#include "VectorLayerData.h" // 引入你设计的VectorLayerData数据结构

#include <QtWidgets/QMainWindow>
#include "ui_MYGIS.h"
#include <ogrsf_frmts.h>
#include <map>
#include <QString>
#include "GDALRasterRead.h"

//防止qt出现中文乱码
#ifdef WIN32
#pragma  execution_character_set("utf-8")
#endif

//前向声明
class MyGraphicsView;
class QTableWidgetItem;
class GeometryItem;//矢量底层绘图item
class featuresToShpWindow;//要素转shapefile窗口
class CalculateGeometryWindow;//统计几何窗口
class ConvexHullWindow;//凸包计算窗口
class OverlayAnalysisWindow;//叠加分析窗口
class TextSaveVector;//图层保存为文本格式窗口
class ProjectManager;//工程管理
class ProgramStatementWindow;//程序说明窗口

class RasterItem;//栅格底层绘图item
class RasterFalseColorWindow;//栅格假色彩显示窗口
class GrayscaleHistogramWindow;//灰度直方图窗口
class EqualizeRasterWindow;//均衡化栅格显示窗口

class MYGIS : public QMainWindow
{
	Q_OBJECT

public:
	 MYGIS(QWidget* parent = nullptr);
	~MYGIS();

	//返回编辑状态
	bool isEditingMode() const {
		return mEditingMode;
	}

	// 获取 mpGraphicsView 的方法
	 MyGraphicsView* getmpGraphicsView() const {
		return mpGraphicsView;
	}

	// 获取 mvSelectedItems 的方法
	QList<GeometryItem*>& getSelectedItems() {
		return mvSelectedItems;
	}

	// 添加一个公有方法以获取矢量图层的列表
	const std::map<QString, VectorLayerData>& getVectorLayers() const {
		return mvVectorLayer; 
	}

	// 添加一个公有方法以获取栅格图层的列表
	const std::map<QString, std::vector<RasterItem*>>& getmvRasterItems() const {
		return mvRasterItems;
	}

	//获取特定图层的 RasterData 对象
	RasterData* getRasterData(const QString& layerName) {
		auto it = mvRasterLayers.find(layerName);
		if (it != mvRasterLayers.end()) {
			return it->second->getRasterData()[layerName];
		}
		return nullptr;
	}

	//获取mvRasterLayers
	const std::map<QString, std::shared_ptr<GDALRasterRead>>& getmvRasterLayers() const {
		return mvRasterLayers;
	}

	//根据图层名称从 mvLayerColors 中获取当前图层的颜色。
	QColor getLayerColor(const QString& layerName) const {
		auto it = mvLayerColors.find(layerName);
		if (it != mvLayerColors.end()) {
			return it->second; // 使用 it->second 获取值
		}
		return QColor(); // 如果找不到对应的图层颜色，返回一个默认颜色
	}

	
	void UpdateAttributeTables();//更新属性表
	void updateLayerIcon(QTreeWidgetItem* layerItem, const QString& layerName);//更新图标
	QTreeWidgetItem* findLayerItemByName(const QString& layerName);//图层名称查找对应的 QTreeWidgetItem
	bool saveVectorLayerAsShapefile(const QString& layerName, const QString& outputPath);//提供一个要素转shapefile的接口
	//计算几何
	bool calculateGeometry(const QString& layerName, const QString& lineField, const QString& areaField);
	OGRGeometry* createOGRGeometryFromGeometryData(const GeometryData& geomData);//将 GeometryData 转换为 OGRGeometry
	//按路径添加图层
	void addVectorLayer(const QString& filePath);
	// 获取当前视图中的图层颜色和 Z 轴值
	void storeLayerPropertiesFromView();

	void loadVectorLayerFromProperties(const QString& filePath, int zIndex, const QColor& layerColor);//打开工程文件绘制矢量
	void loadRasterLayerFromProperties(const QString& filePath, int zIndex);//打开工程文件绘制栅格

	//工程保存
	ProjectManager* mpProjectManager;

	QPointF mapRasterToViewCoordinates(QGraphicsScene* scene, double x, double y);//关于栅格Y轴翻转
	QImage createImageFromRasterData(RasterData* rasterData);//对栅格数据转为image
	void addRasterToManage(const QPixmap& pixmap, const QString& layerName, const double* geoTransform, int rows, int cols);//添加分析后的栅格进行管理
	
private slots:
	void saveProject();//保存工程
	void openProject();//打开工程


	void showFeaturesToShpWindow();//打开要素转shapefile窗口
	void showCalculateGeometryWindow();//统计几何窗口
	void showConvexHullWindow();//凸包计算窗口
	void showOverlayAnalysisWindow();//叠加分析窗口
	void showTextSaveVector();//图层保存为文本格式窗口
	void showRasterFalseColorWindow();//栅格假色彩显示窗口
	void showGrayscaleHistogramWindow();//灰度直方图窗口
	void showEqualizeRasterWindow();//均衡化栅格显示窗口
	void showProgramStatementWindow();//程序说明窗口

	void openVectorLayer(); // 用于处理打开矢量图层的槽函数
	void viewCoordinateChanged(QPointF scenePos);//鼠标移动显示经纬度

	void showLayerContextMenu(const QPoint& pos); // 显示图层上下文菜单的槽函数
	void layerDisplays(QTreeWidgetItem* item, int column);//复选框影藏和显示图层
	void zoomToLayer(QTreeWidgetItem* item); // 缩放到图层的槽函数
	void removeLayer(QTreeWidgetItem* item); // 移除图层的槽函数
	void showLayerAttributeData(QTreeWidgetItem* item);//图层属性表槽函数
	void onAttributeItemSelected(QTableWidgetItem* item, const QString& layerName);   // 高亮新的几何体
	void layerMoveToTop(QTreeWidgetItem* item); // 图层上移到顶部
	void layerMoveToBottom(QTreeWidgetItem* item); // 图层下移到底部
	void layerColorSystem();//图层颜色
	void updateLayerFeatureCount(QTreeWidgetItem* item);	// 更新图层要素总数显示


	void toggleEditingMode(); // 切换编辑模式的槽函数
	void deleteSelectedItems(); // 删除所选要素的槽函数
	void withdrawDelete();// 撤销删除操作的槽函数
	void saveEdit();// 保存编辑操作的槽函数
	void endEdit(); // 退出编辑操作的槽函数

	void resetView();//重置程序视图的位置
	void startPreviewMode();//开始预览模式
	void endPreviewMode();//结束预览模式

	
	void openRasterLayer();//打开并显示栅格
	void openBigTif();//打开超大栅格
	
	void showAuthorStatementMessage();//作者声明
private:
	Ui::MYGISClass ui;

	featuresToShpWindow* mpfeaturesToShpWindow;//要素转shapefile窗口
	CalculateGeometryWindow* mpCalculateGeometryWindow;//统计几何窗口
	ConvexHullWindow* mpConvexHullWindow;//凸包计算窗口
	OverlayAnalysisWindow* mpOverlayAnalysisWindow;//叠加分析窗口
	TextSaveVector* mpTextSaveVector;//图层保存为文本格式窗口
	RasterFalseColorWindow* mpRasterFalseColorWindow;//栅格假色彩显示窗口
	GrayscaleHistogramWindow* mpGrayscaleHistogramWindow;//灰度直方图窗口
	EqualizeRasterWindow* mpEqualizeRasterWindow;//均衡化栅格显示窗口
	ProgramStatementWindow* mpProgramStatementWindow;//程序说明窗口

	void InitializeMapView();//初始化地图视图
	
	QGraphicsScene* mpGraphicsScene; // 场景对象
	MyGraphicsView* mpGraphicsView; // 使用自定义的 MyGraphicsView

	std::map<QString, QColor> mvLayerColors;//图层颜色
	std::map<QString, VectorLayerData> mvVectorLayer; // 用于存储多个矢量图层
	void readVectorLayer();//读取矢量

	QTreeWidget* mpLayerTreeWidget; // 用于指向 LayertreeWidget
	void addLayerToTreeWidget(const QString& layerName, bool isRasterLayer);//添加图层到图层树

	bool mEditingMode = false; // 是否处于编辑模式
	bool mIsSaved = false; // 用于标识编辑操作是否已经保存
	QList<GeometryItem*> mvSelectedItems;  // 存储被选中的要素
	QList<FeatureData> mvDeletedFeatures;  // 用于存储被删除的 FeatureData 对象
	QList<GeometryItem*> mvDeletedItems; // 临时存放被删除的要素，便于撤回


	// 在 MYGIS 类中添加一个 std::map 来管理栅格图层
	std::map<QString, std::shared_ptr<GDALRasterRead>> mvRasterLayers;
	// 在 MYGIS.h 中添加 mRasterItems 的声明
	std::map<QString, std::vector<RasterItem*>> mvRasterItems;


};

#endif