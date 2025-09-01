//**********************************************************************************************//
//		使用了Qt c++小部件。Qcustomplot，该绘图工具直接添加lib链接库和头文件和cpp就能用			//
//		栅格的读取参考了姚尧老师发在群里的关于栅格读取的方法
//		进行了一部分增删，比如将rasterIO改成bip格式的读取比如增加了经纬度获取，直方图获取等！	//
#include "MYGIS.h"
#include "LoggerConfig.h"
#include "GeometryItem.h"
#include "MapControls.h"
#include "VectorLayerData.h"//矢量底层
#include "GeometryItem.h"
#include "MyGraphicsView.h"
#include "ProjectManager.h"
#include "FileBrowserWidget.h"//右下角浏览器
#include "GDALRasterRead.h"//栅格底层
#include "featuresToShpWindow.h"//要素转shapefile窗口
#include "CalculateGeometryWindow.h"//统计几何窗口
#include "ConvexHullWindow.h"//凸包计算窗口
#include "OverlayAnalysisWindow.h"//叠加分析窗口
#include "TextSaveVector.h"//图层保存为文本格式窗口
#include "RasterFalseColorWindow.h"//栅格假色彩窗口
#include "GrayscaleHistogramWindow.h"//灰度直方图窗口
#include "EqualizeRasterWindow.h"//均衡化栅格显示窗口
#include "ProgramStatementWindow.h"//程序说明窗口


#include "qcustomplot.h" //使用了Qt c++小部件。Qcustomplot
#include <ogr_spatialref.h> // 引入 GDAL 空间参考库
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QRectF>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <variant>
#include <QRandomGenerator>
#include <QMouseEvent>
#include <QTableWidget>
#include<QTableWidgetItem>
#include<QColorDialog>
#include<QToolButton>
#include <QGraphicsPixmapItem>


MYGIS::MYGIS(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	//初始化右下角文件浏览器
	FileBrowserWidget* fileBrowserWidget = new FileBrowserWidget(this);
	ui.BrowserdockWidget->setWidget(fileBrowserWidget);

	mpProjectManager = new ProjectManager();

	//要素转shapefile窗口
	mpfeaturesToShpWindow = new featuresToShpWindow(this,this);
	//统计几何窗口
	mpCalculateGeometryWindow = new CalculateGeometryWindow(this, this);
	//凸包计算窗口
	mpCalculateGeometryWindow = new CalculateGeometryWindow(this, this);
	//叠加分析窗口
	mpOverlayAnalysisWindow = new OverlayAnalysisWindow(this, this);
	//图层保存为文本格式窗口
	mpTextSaveVector = new TextSaveVector(this, this);
	//栅格假色彩显示窗口
	mpRasterFalseColorWindow = new RasterFalseColorWindow(this, this);
	//灰度直方图窗口
	mpGrayscaleHistogramWindow = new GrayscaleHistogramWindow(this, this);
	//均衡化栅格显示窗口
	mpEqualizeRasterWindow = new EqualizeRasterWindow(this, this);
	//程序说明窗口
	mpProgramStatementWindow = new ProgramStatementWindow(this,this);

	// 使用自定义的 MyGraphicsView 代替原有的 QGraphicsView
	mpGraphicsView = new MyGraphicsView(this);
	ui.graphicsView->setViewport(mpGraphicsView);

	// 初始化 LayertreeWidget
	mpLayerTreeWidget = ui.LayertreeWidget;
	mpLayerTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);//设置 QTreeWidget 的上下文菜单策略。
	// 连接 LayertreeWidget 的右键点击信号到槽函数
	connect(mpLayerTreeWidget, &QTreeWidget::customContextMenuRequested, this, &MYGIS::showLayerContextMenu);
	// 连接图层复选框状态改变的信号到槽函数
	connect(mpLayerTreeWidget, &QTreeWidget::itemChanged, this, &MYGIS::layerDisplays);

	//打开要素转shapefile窗口
	connect(ui.actionsaveShp, &QAction::triggered, this, &MYGIS::showFeaturesToShpWindow);
	//图层保存为文本格式窗口
	connect(ui.actionTextSaveVector, &QAction::triggered, this, &MYGIS::showTextSaveVector);
	//统计几何窗口
	connect(ui.actionCalculateGeometry, &QAction::triggered, this, &MYGIS::showCalculateGeometryWindow);
	connect(ui.CalculateGeometrytoolButton, &QToolButton::clicked, this, &MYGIS::showCalculateGeometryWindow);
	//凸包计算窗口
	connect(ui.actionConvexHull, &QAction::triggered, this, &MYGIS::showConvexHullWindow);
	connect(ui.ConvexHulltoolButton, &QToolButton::clicked, this, &MYGIS::showConvexHullWindow);
	//叠加分析窗口
	connect(ui.actionOverlayAnalysis, &QAction::triggered, this, &MYGIS::showOverlayAnalysisWindow);
	connect(ui.OverlayAnalysistoolButton, &QToolButton::clicked, this, &MYGIS::showOverlayAnalysisWindow);
	//栅格假色彩显示窗口
	connect(ui.actionRasterFalseColor, &QAction::triggered, this, &MYGIS::showRasterFalseColorWindow);
	connect(ui.RasterFalseColortoolButton, &QToolButton::clicked, this, &MYGIS::showRasterFalseColorWindow);
	//灰度直方图窗口
	connect(ui.actionGrayscaleHistogram, &QAction::triggered, this, &MYGIS::showGrayscaleHistogramWindow);
	connect(ui.GrayscaleHistogramtoolButton, &QToolButton::clicked, this, &MYGIS::showGrayscaleHistogramWindow);
	//均衡化栅格显示窗口
	connect(ui.actionEqualizeRaster, &QAction::triggered, this, &MYGIS::showEqualizeRasterWindow);
	connect(ui.EqualizeRastertoolButton, &QToolButton::clicked, this, &MYGIS::showEqualizeRasterWindow);
	//程序说明窗口
	connect(ui.actionProgramStatement, &QAction::triggered, this, &MYGIS::showProgramStatementWindow);

	// 初始化地图视图
	InitializeMapView();

	// 在初始化时，mapControls传入QGraphicsView对象
	MapControls* mapControls = new MapControls(mpGraphicsView, this); // 注意这里传入 mpGraphicsView

	// 连接打开矢量图层的槽函数
	connect(ui.actionopenVectorLayer, &QAction::triggered, this, &MYGIS::openVectorLayer);

	// 连接打开栅格图层的槽函数
	connect(ui.actionopenRasterLayer, &QAction::triggered, this, &MYGIS::openRasterLayer);

	//超大栅格
	connect(ui.actionbigTIF, &QAction::triggered, this, &MYGIS::openBigTif);

	// 连接自定义视图的 mouseMoved 信号到槽函数
	connect(mpGraphicsView, &MyGraphicsView::mouseMoved, this, &MYGIS::viewCoordinateChanged);


	//进入编辑
	connect(ui.actionstartEdit, &QAction::triggered, this, &MYGIS::toggleEditingMode);
	// 连接删除操作槽函数
	connect(ui.actiondelete, &QAction::triggered, this, &MYGIS::deleteSelectedItems);
	// 连接撤销操作槽函数
	connect(ui.actionwithdraw, &QAction::triggered, this, &MYGIS::withdrawDelete);
	// 连接保存操作槽函数
	connect(ui.actionsaveEdit, &QAction::triggered, this, &MYGIS::saveEdit);
	// 连接退出编辑操作槽函数
	connect(ui.actionendEdit, &QAction::triggered, this, &MYGIS::endEdit);

	//重置默认视图为默认
	connect(ui.actionResetView, &QAction::triggered, this, &MYGIS::resetView);
	//开始预览模式
	connect(ui.actionPreviewMode, &QAction::triggered, this, &MYGIS::startPreviewMode);
	//结束预览模式
	connect(ui.actionExitPreview, &QAction::triggered, this, &MYGIS::endPreviewMode);

	//保存工程
	connect(ui.actionSaveProject, &QAction::triggered, this, &MYGIS::saveProject);
	//打开工程
	connect(ui.actionOpenProject, &QAction::triggered, this, &MYGIS::openProject);
	//作者声明
	connect(ui.actionAuthorStatement, &QAction::triggered, this, &MYGIS::showAuthorStatementMessage);//帮助选项
}

MYGIS::~MYGIS()
{
	if (mpCalculateGeometryWindow != nullptr)
	{
		delete mpCalculateGeometryWindow;
		mpCalculateGeometryWindow = nullptr;
	}
	/*delete mpConvexHullWindow;*/
	if (mpOverlayAnalysisWindow != nullptr)
	{
		delete mpOverlayAnalysisWindow;
		mpOverlayAnalysisWindow = nullptr;
	}
	if (mpfeaturesToShpWindow != nullptr)
	{
		delete mpfeaturesToShpWindow;
		mpfeaturesToShpWindow = nullptr;
	}
	if (mpGraphicsView != nullptr)
	{
		delete mpGraphicsView;
		mpGraphicsView = nullptr;
	}
	if (mpLayerTreeWidget != nullptr)
	{
		delete mpLayerTreeWidget;
		mpLayerTreeWidget = nullptr;
	}

	// 删除场景中的所有图层
	if (mpGraphicsScene != nullptr)
	{
		QList<QGraphicsItem*> items = mpGraphicsScene->items();
		for (QGraphicsItem* item : items)
		{
			mpGraphicsScene->removeItem(item);
			delete item; // 手动删除QGraphicsItem
		}
		delete mpGraphicsScene;
		mpGraphicsScene = nullptr;
	}
	 // 释放所有栅格图层占用的内存
	

	mvLayerColors.clear();
	mvVectorLayer.clear(); // 清除图层映射
}

//作者声明
void MYGIS::showAuthorStatementMessage() {
	QString message = "这是中国地质大学（武汉）\n"
		"钟新伟的MYGIS程序\n"
		"我的QQ: 59426212\n"
		"我的邮箱: 59426212@qq.com";

	QMessageBox::information(this, "作者声明", message);
}


//**********************************窗口显示//**********************************
//打开要素转shapefile窗口
void MYGIS::showFeaturesToShpWindow(){
	//初始化窗口
	mpfeaturesToShpWindow = new featuresToShpWindow(this, this);
	mpfeaturesToShpWindow->show();
}
//图层保存为文本格式窗口
void MYGIS::showTextSaveVector() {
	//初始化窗口
	mpTextSaveVector = new TextSaveVector(this, this);
	mpTextSaveVector->show();
}

//统计几何窗口
void MYGIS::showCalculateGeometryWindow() {
	//初始化窗口
	mpCalculateGeometryWindow = new CalculateGeometryWindow(this, this);
	mpCalculateGeometryWindow->show();
}

//凸包计算窗口
void MYGIS::showConvexHullWindow() {
	//初始化窗口
	mpConvexHullWindow = new ConvexHullWindow(this, this);
	mpConvexHullWindow->show();
}

//叠加分析窗口
void MYGIS::showOverlayAnalysisWindow() {
	//初始化窗口
	mpOverlayAnalysisWindow = new OverlayAnalysisWindow(this, this);
	mpOverlayAnalysisWindow->show();
}

//栅格假色彩显示窗口
void MYGIS::showRasterFalseColorWindow() {
	//初始化窗口
	mpRasterFalseColorWindow = new RasterFalseColorWindow(this, this);
	mpRasterFalseColorWindow->show();
}

//灰度直方图窗口
void MYGIS::showGrayscaleHistogramWindow() {
	//初始化窗口
	mpGrayscaleHistogramWindow = new GrayscaleHistogramWindow(this, this);
	mpGrayscaleHistogramWindow->show();
}

//均衡化栅格显示窗口
void MYGIS::showEqualizeRasterWindow() {
	//初始化窗口
	mpEqualizeRasterWindow = new EqualizeRasterWindow(this, this);
	mpEqualizeRasterWindow->show();
}

////程序说明窗口
void MYGIS::showProgramStatementWindow() {
	//初始化窗口
	mpProgramStatementWindow = new ProgramStatementWindow(this, this);
	mpProgramStatementWindow->show();
}


//**********************************矢量导入和绘制以及视图初始化//**********************************
// 初始化地图视图
void MYGIS::InitializeMapView()
{
	// 设置视图范围为 WGS 1984 坐标系（经度从 -180 到 180，纬度从 -90 到 90）
	QRectF geographicRect(-180, -90, 360, 180);  // 经纬度范围

	// 设置场景的边界
	mpGraphicsScene = new QGraphicsScene(this);
	mpGraphicsScene->setSceneRect(geographicRect);

	// 配置视图
	mpGraphicsView->setScene(mpGraphicsScene);
	mpGraphicsView->fitInView(geographicRect, Qt::KeepAspectRatio);  // 适应视图

	// 可选: 设置视图的其他属性，例如背景颜色
	mpGraphicsView->setRenderHint(QPainter::Antialiasing);  // 开启抗锯齿
	mpGraphicsView->setRenderHint(QPainter::SmoothPixmapTransform);  // 平滑缩放图像

	// 设置默认缩放
	mpGraphicsView->setDragMode(QGraphicsView::NoDrag);
	mpGraphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	mpGraphicsView->setMouseTracking(true); // 启用 mouseTracking 捕捉鼠标移动
}

//读取矢量数据并存储
void MYGIS::readVectorLayer()
{
	
	// 选择文件
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Vector Layer"), "", tr("Vector Files (*.shp *.geojson *.csv);;All Files (*)"));
	if (fileName.isEmpty())
		return;

	// 打开矢量文件（包括 CSV 格式和geojson）
	GDALDataset* poDS = static_cast<GDALDataset*>(GDALOpenEx(fileName.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL));
	if (poDS == nullptr)
	{
		QMessageBox::critical(this, tr("Error"), tr("Failed to open file."));
		return;
	}

	// 获取文件名（不带路径和扩展名）
	QString baseName = QFileInfo(fileName).baseName();
	QString extension = QFileInfo(fileName).suffix().toLower();

	// 遍历所有图层并读取数据
	for (int i = 0; i < poDS->GetLayerCount(); i++)
	{
		OGRLayer* poLayer = poDS->GetLayer(i);
		if (poLayer == nullptr)
		{
			log4cpp::Category& logger = log4cpp::Category::getRoot();
			logger.warn("Warning: Layer %d is null!", i);
			continue;
		}

		// 创建一个新的VectorLayerData对象来存储当前图层的数据
		VectorLayerData layerData;

		// 读取要素并存储到 VectorLayerData 中
		OGRFeature* poFeature;
		poLayer->ResetReading(); // 重置读取，确保从图层的第一个要素开始
		while ((poFeature = poLayer->GetNextFeature()) != nullptr) // 逐个读取图层中的每个要素
		{
			// 获取要素的 FID 作为 ID，便于后续管理和识别
			int featureId = poFeature->GetFID();

			// 读取要素的几何数据并判断其几何类型
			OGRGeometry* poGeometry = poFeature->GetGeometryRef(); // 获取几何数据引用
			GeometryType geomType = GeometryType::Point;  // 初始化为点类型，后续根据实际类型修改
			std::vector<std::vector<double>> coordinates; // 存储点、线或多边形的坐标
			std::vector<std::vector<std::vector<double>>> multiPolygonCoordinates; // 存储多多边形或多线的坐标集合

			if (poGeometry != nullptr) // 确保几何数据不为空
			{
				const char* geometryName = poGeometry->getGeometryName(); // 获取几何类型的名称，便于调试或日志记录

				// 判断几何类型并将坐标存储到对应的结构中
				if (wkbFlatten(poGeometry->getGeometryType()) == wkbPoint)
				{
					geomType = GeometryType::Point; // 设置几何类型为点
					OGRPoint* poPoint = static_cast<OGRPoint*>(poGeometry); // 将几何数据转换为点类型
					coordinates.push_back({ poPoint->getX(), poPoint->getY() }); // 存储点的坐标
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbLineString)
				{
					geomType = GeometryType::Line; // 设置几何类型为线
					OGRLineString* poLine = static_cast<OGRLineString*>(poGeometry); // 将几何数据转换为线类型
					for (int j = 0; j < poLine->getNumPoints(); j++)
					{
						double x = poLine->getX(j); // 获取线的每个点的 X 坐标
						double y = poLine->getY(j); // 获取线的每个点的 Y 坐标
						coordinates.push_back({ x, y }); // 存储线的坐标
					}
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon)
				{
					geomType = GeometryType::Polygon; // 设置几何类型为多边形
					OGRPolygon* poPolygon = static_cast<OGRPolygon*>(poGeometry); // 将几何数据转换为多边形类型
					OGRLinearRing* poRing = poPolygon->getExteriorRing(); // 获取多边形的外环
					for (int j = 0; j < poRing->getNumPoints(); j++)
					{
						double x = poRing->getX(j); // 获取多边形外环每个点的 X 坐标
						double y = poRing->getY(j); // 获取多边形外环每个点的 Y 坐标
						coordinates.push_back({ x, y }); // 存储多边形外环的坐标
					}
				}
				// 处理多点、多线、多多边形和几何集合类型的逻辑
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPoint)
				{
					geomType = GeometryType::MultiPoint; // 设置几何类型为多点
					OGRMultiPoint* poMultiPoint = static_cast<OGRMultiPoint*>(poGeometry); // 将几何数据转换为多点类型
					for (int j = 0; j < poMultiPoint->getNumGeometries(); j++)
					{
						OGRGeometry* poPointGeometry = poMultiPoint->getGeometryRef(j); // 获取多点中的每个点
						OGRPoint* poPoint = static_cast<OGRPoint*>(poPointGeometry);
						coordinates.push_back({ poPoint->getX(), poPoint->getY() }); // 存储多点的坐标
					}
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbMultiLineString)
				{
					geomType = GeometryType::MultiLineString; // 设置几何类型为多线
					OGRMultiLineString* poMultiLine = static_cast<OGRMultiLineString*>(poGeometry); // 将几何数据转换为多线类型
					for (int j = 0; j < poMultiLine->getNumGeometries(); j++)
					{
						OGRGeometry* poLineGeometry = poMultiLine->getGeometryRef(j); // 获取多线中的每条线
						OGRLineString* poLine = static_cast<OGRLineString*>(poLineGeometry);
						std::vector<std::vector<double>> lineCoordinates; // 用于存储每条线的坐标
						for (int k = 0; k < poLine->getNumPoints(); k++)
						{
							double x = poLine->getX(k); // 获取线的每个点的 X 坐标
							double y = poLine->getY(k); // 获取线的每个点的 Y 坐标
							lineCoordinates.push_back({ x, y }); // 存储线的坐标
						}
						multiPolygonCoordinates.push_back(lineCoordinates); // 存储所有线的坐标集合
					}
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon)
				{
					geomType = GeometryType::MultiPolygon; // 设置几何类型为多多边形
					OGRMultiPolygon* poMultiPolygon = static_cast<OGRMultiPolygon*>(poGeometry); // 将几何数据转换为多多边形类型
					for (int k = 0; k < poMultiPolygon->getNumGeometries(); k++)
					{
						OGRGeometry* poSubGeometry = poMultiPolygon->getGeometryRef(k); // 获取多多边形中的每个子多边形
						if (poSubGeometry != nullptr && wkbFlatten(poSubGeometry->getGeometryType()) == wkbPolygon)
						{
							OGRPolygon* poPolygon = static_cast<OGRPolygon*>(poSubGeometry);
							OGRLinearRing* poRing = poPolygon->getExteriorRing(); // 获取子多边形的外环

							std::vector<std::vector<double>> subPolygonCoordinates; // 用于存储子多边形的坐标
							for (int j = 0; j < poRing->getNumPoints(); j++)
							{
								double x = poRing->getX(j); // 获取子多边形外环每个点的 X 坐标
								double y = poRing->getY(j); // 获取子多边形外环每个点的 Y 坐标
								subPolygonCoordinates.push_back({ x, y }); // 存储子多边形外环的坐标
							}

							multiPolygonCoordinates.push_back(subPolygonCoordinates); // 存储所有子多边形的坐标集合
						}
					}
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbGeometryCollection)
				{
					geomType = GeometryType::GeometryCollection; // 设置几何类型为几何集合
					OGRGeometryCollection* poGeomCollection = static_cast<OGRGeometryCollection*>(poGeometry); // 将几何数据转换为几何集合类型
					for (int j = 0; j < poGeomCollection->getNumGeometries(); j++)
					{
						OGRGeometry* poSubGeometry = poGeomCollection->getGeometryRef(j); // 获取几何集合中的每个几何体
						// 递归调用相同的逻辑来处理集合中的几何体
						// 可以将上面的逻辑提取为函数，然后在此调用
					}
				}
			}

			// 创建 GeometryData 对象并存储几何数据
			GeometryData geomData;
			if (geomType == GeometryType::MultiPolygon || geomType == GeometryType::MultiLineString) {
				geomData = GeometryData(geomType, multiPolygonCoordinates); // 如果几何类型为多多边形或多线，存储对应的坐标集合
			}
			else {
				geomData = GeometryData(geomType, coordinates); // 如果几何类型为点、线或单一多边形，存储对应的坐标
			}

			// 读取要素的属性数据
			AttributeData attrData;
			for (int j = 0; j < poFeature->GetFieldCount(); j++) // 遍历所有属性字段
			{
				OGRFieldDefn* poFieldDefn = poFeature->GetFieldDefnRef(j); // 获取属性字段定义
				const char* fieldName = poFieldDefn->GetNameRef(); // 获取属性字段的名称
				if (poFeature->IsFieldSet(j)) // 检查字段是否已设置值
				{
					// 根据字段类型存储属性值
					if (poFieldDefn->GetType() == OFTInteger)
					{
						int intValue = poFeature->GetFieldAsInteger(j); // 获取整数类型的属性值
						attrData.addAttribute(QString::fromUtf8(fieldName), intValue); // 将属性名和值添加到 AttributeData 中
					}
					else if (poFieldDefn->GetType() == OFTReal)
					{
						double doubleValue = poFeature->GetFieldAsDouble(j); // 获取浮点数类型的属性值
						attrData.addAttribute(QString::fromUtf8(fieldName), doubleValue); // 将属性名和值添加到 AttributeData 中
					}
					else if (poFieldDefn->GetType() == OFTString)
					{
						QString stringValue = QString::fromUtf8(poFeature->GetFieldAsString(j)); // 获取字符串类型的属性值
						attrData.addAttribute(QString::fromUtf8(fieldName), stringValue); // 将属性名和值添加到 AttributeData 中
					}
				}
			}

			// 创建 FeatureData 对象，将几何数据和属性数据组合起来
			FeatureData featureData(featureId, geomData, attrData);
			layerData.addFeature(featureData); // 将 FeatureData 添加到图层数据中

			// 释放当前要素以避免内存泄漏
			OGRFeature::DestroyFeature(poFeature);
}

		// 判断是否为 CSV 文件，并设置图层名称
		//QString layerName = (extension == "csv") ? (baseName + "_WKTLayer") : QString::fromUtf8(poLayer->GetName());
		// 判断是否为 geojson 文件，并设置图层名称
		//QString layerName = (extension == "geojson") ? (baseName + "_geojsonLayer") : QString::fromUtf8(poLayer->GetName());
		QString layerName;
		if (extension == "csv") {
			layerName = baseName + "_WKTLayer";
		}// 判断是否为 CSV 文件，并设置图层名称
		else if (extension == "geojson") {
			layerName = baseName + "_geojsonLayer";
		}// 判断是否为 geojson 文件，并设置图层名称
		else {
			const char* rawLayerName = poLayer->GetName();
			if (rawLayerName && strlen(rawLayerName) > 0) {
				layerName = QString::fromUtf8(rawLayerName);
			}
			else {
				layerName = baseName; // 默认使用基础名称
			}
		}
		mvVectorLayer[layerName] = layerData;

		// 存储文件路径和图层名称到 ProjectManager
		mpProjectManager->addLayer(fileName, layerName);

		// 使用log4cpp记录成功读取的信息
		logger.info("Layer %s successfully read with %d features.", layerName.toStdString().c_str(), layerData.getFeatures().size());
	}

	// 关闭数据集
	GDALClose(poDS);
	// 使用log4cpp记录文件读取完成的信息
	logger.info("Finished reading vector file: %s", fileName.toStdString().c_str());
}

// 在openVectorLayer() 打开和绘制图层
void MYGIS::openVectorLayer()
{
	readVectorLayer();

	QRectF totalBoundingRect;  // 用于存储所有导入图层的总边界

	// 清空 LayertreeWidget 中的内容
	// 遍历 LayertreeWidget 中的项，并移除所有矢量图层的项
	for (int i = 0; i < mpLayerTreeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem* item = mpLayerTreeWidget->topLevelItem(i);
		if (item->text(0).startsWith("[Vector] ")) {
			delete mpLayerTreeWidget->takeTopLevelItem(i);
			--i;  // 调整索引以反映删除的项
		}
	}

	int baseZValue = 0;  // 初始化基础 Z 值

	// 遍历读取的矢量图层数据
	for (const auto& layerPair : mvVectorLayer)
	{
		const QString& layerName = layerPair.first;
		const VectorLayerData& layerData = layerPair.second;

		// 为图层生成颜色，如果该图层已存在颜色，则使用原有颜色
		QColor layerColor;
		if (mvLayerColors.find(layerName) == mvLayerColors.end())
		{
			do {
				// 如果图层没有颜色，生成一个新颜色
				layerColor = QColor::fromRgb(QRandomGenerator::global()->generate());
			} while (layerColor == Qt::yellow);  // 检查颜色是否与 Qt::yellow 冲突

			// 将颜色存储在 mvLayerColors 中
			mvLayerColors[layerName] = layerColor;
		}
		else
		{
			// 如果图层已有颜色，复用原有颜色
			layerColor = mvLayerColors[layerName];
		}

		// 为图层中的每个要素设置唯一的Z轴值
		int featureIndex = 0;
		for (const auto& feature : layerData.getFeatures())
		{
			int id = feature.getId();
			// 计算每个要素的唯一Z轴值
			int zIndex = baseZValue + featureIndex;

			//qDebug() << "Creating GeometryItem with ID:" << id;

			// 传递 id 和图层名称到 GeometryItem 构造函数
			GeometryItem* item = new GeometryItem(id, feature.getGeometry(), layerColor, zIndex, layerName, this, nullptr);
			mpGraphicsScene->addItem(item);

			totalBoundingRect = totalBoundingRect.united(item->boundingRect());
			featureIndex++;  // 增加索引以确保Z轴值唯一
		}

		for (const auto& layerPair : mvVectorLayer) {
			const QString& layerName = layerPair.first;
			addLayerToTreeWidget(layerName, false); // 矢量图层
		}

		baseZValue += 100;  // 每个图层分配一个新的基础 Z 值范围
	}

	// 调整视图以适应总边界
	if (!totalBoundingRect.isNull())
	{
		mpGraphicsView->fitInView(totalBoundingRect, Qt::KeepAspectRatio);
	}
	else
	{
		mpGraphicsView->fitInView(mpGraphicsScene->sceneRect(), Qt::KeepAspectRatio);
	}

	mpGraphicsView->update(); // 更新视图
}



//**********************************栅格绘制和导入//**********************************
//栅格显示
void MYGIS::openRasterLayer() {
	// 使用 QFileDialog 打开栅格文件
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open Raster Layer"), "", tr("Raster Files (*.tif *.tiff)"));

	if (filePath.isEmpty()) {
		qDebug() << "openRasterLayer: No file selected.";
		logger.warn("openRasterLayer: No file selected.");
		return;
	}

	QString layerName = QFileInfo(filePath).fileName();
	auto rasterLayer = std::make_shared<GDALRasterRead>();

	bool success = rasterLayer->loadFromGDAL(filePath.toStdString().c_str(), layerName);

	if (success) {
		qDebug() << "openRasterLayer: Raster layer" << layerName << "loaded successfully.";
		logger.info("openRasterLayer: Raster layer %s loaded successfully.", layerName.toStdString().c_str());

		// 将加载的栅格图层保存到管理结构中
		mvRasterLayers[layerName] = rasterLayer;
		addLayerToTreeWidget(layerName, true); // 仅调用一次，避免重复

		RasterData* rasterData = rasterLayer->getRasterData()[layerName];
		if (rasterData && rasterData->getBandnum() >= 1) { // 现在支持单波段和多波段
			QImage image = createImageFromRasterData(rasterData);
			QPixmap pixmap = QPixmap::fromImage(image);

			double geoTransform[6];
			GDALDataset* dataset = rasterLayer->getGDALDataset();
			if (dataset->GetGeoTransform(geoTransform) == CE_None) {
				QPointF topLeft = mapRasterToViewCoordinates(mpGraphicsScene, geoTransform[0], geoTransform[3]);
				QPointF bottomRight = mapRasterToViewCoordinates(mpGraphicsScene, geoTransform[0] + geoTransform[1] * rasterData->getCols(), geoTransform[3] + geoTransform[5] * rasterData->getRows());
				QRectF imageRect(topLeft, bottomRight);

				// 创建 RasterItem 并添加到场景中
				RasterItem* item = new RasterItem(pixmap, layerName);
				item->setPos(imageRect.topLeft());
				item->setScale(imageRect.width() / pixmap.width());
				mpGraphicsScene->addItem(item);

				// 在 std::map 中存储 RasterItem 指针集合以便管理
				mvRasterItems[layerName].push_back(item);

				// 适应视图范围
				mpGraphicsView->fitInView(imageRect, Qt::KeepAspectRatio);

				qDebug() << "openRasterLayer: Raster image displayed successfully.";
				logger.info("openRasterLayer: Raster image %s displayed successfully.", layerName.toStdString().c_str());
				mpProjectManager->addLayer(filePath, layerName);//存储导入的栅格
			}
		}
		else {
			qDebug() << "openRasterLayer: Raster does not have enough bands for RGB or grayscale.";
			logger.warn("openRasterLayer: Raster layer %s does not have enough bands for RGB or grayscale.", layerName.toStdString().c_str());
		}
	}
	else {
		qDebug() << "openRasterLayer: Failed to load raster layer" << layerName;
		logger.error("openRasterLayer: Failed to load raster layer %s.", layerName.toStdString().c_str());
	}
}


// 定义 createImageFromRasterData 函数创建栅格
QImage MYGIS::createImageFromRasterData(RasterData* rasterData) {
	size_t rows = rasterData->getRows();
	size_t cols = rasterData->getCols();
	size_t bands = rasterData->getBandnum();
	size_t perPixSize = rasterData->getPerPixSize();

	QImage image(cols, rows, bands == 1 ? QImage::Format_Grayscale8 : QImage::Format_RGB888);

	if (bands == 1) {
		// 单波段图像，转换为灰度图像
		for (size_t row = 0; row < rows; ++row) {
			for (size_t col = 0; col < cols; ++col) {
				int offset = (row * cols + col) * perPixSize;
				int gray = static_cast<int>(rasterData->getImgData()[offset]);
				image.setPixel(col, row, qRgb(gray, gray, gray));
			}
		}
	}
	else if (bands >= 3) {
		// 多波段图像，假设为RGB格式
		for (size_t row = 0; row < rows; ++row) {
			for (size_t col = 0; col < cols; ++col) {
				int offset = (row * cols + col) * bands * perPixSize;
				int r = static_cast<int>(rasterData->getImgData()[offset]);
				int g = static_cast<int>(rasterData->getImgData()[offset + perPixSize]);
				int b = static_cast<int>(rasterData->getImgData()[offset + 2 * perPixSize]);
				image.setPixel(col, row, qRgb(r, g, b));
			}
		}
	}
	else {
		qDebug() << "Unsupported band configuration.";
	}

	return image;
}

//绘制超大栅格
void MYGIS::openBigTif() {
	// 使用 QFileDialog 打开栅格文件
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open Raster Layer"), "", tr("Raster Files (*.tif *.tiff)"));
	if (filePath.isEmpty()) {
		qDebug() << "openBigTif: No file selected.";
		return;
	}

	GDALDataset* dataset = static_cast<GDALDataset*>(GDALOpen(filePath.toStdString().c_str(), GA_ReadOnly));
	if (dataset == nullptr) {
		qDebug() << "openBigTif: Failed to open file.";
		return;
	}

	int rasterXSize = dataset->GetRasterXSize();
	int rasterYSize = dataset->GetRasterYSize();
	int bandCount = dataset->GetRasterCount();

	if (bandCount < 3) {
		qDebug() << "openBigTif: Raster does not have enough bands for RGB.";
		GDALClose(dataset);
		return;
	}

	double geoTransform[6];
	if (dataset->GetGeoTransform(geoTransform) != CE_None) {
		qDebug() << "openBigTif: Failed to get geotransform.";
		GDALClose(dataset);
		return;
	}

	QString layerName = QFileInfo(filePath).fileName();
	int blockSize = 1024; // 可以根据需要调整块大小
	QRectF sceneRect;

	// 预先声明 item 变量
	RasterItem* item = nullptr;

	std::vector<RasterItem*> items; // 用于存储所有的 RasterItem

	QImage image(blockSize, blockSize, QImage::Format_RGB888);  // 重复使用 QImage 对象
	QPixmap pixmap;

	// 创建进度条
	QProgressDialog progressDialog(tr("加载超大栅格中..."), tr("Cancel"), 0, (rasterXSize / blockSize) * (rasterYSize / blockSize), this);
	progressDialog.setWindowModality(Qt::WindowModal);
	progressDialog.show();

	int progressCounter = 0;

	for (int yBlock = 0; yBlock < rasterYSize; yBlock += blockSize) {
		for (int xBlock = 0; xBlock < rasterXSize; xBlock += blockSize) {
			int xBlockSize = std::min(blockSize, rasterXSize - xBlock);
			int yBlockSize = std::min(blockSize, rasterYSize - yBlock);

			std::vector<unsigned char> buffer(xBlockSize * yBlockSize * bandCount);

			CPLErr err = dataset->RasterIO(GF_Read, xBlock, yBlock, xBlockSize, yBlockSize,
				buffer.data(), xBlockSize, yBlockSize, GDT_Byte, bandCount, nullptr,
				bandCount, bandCount * xBlockSize, 1);

			if (err != CE_None) {
				qDebug() << "openBigTif: Error during RasterIO.";
				GDALClose(dataset);
				return;
			}

			for (int y = 0; y < yBlockSize; ++y) {
				for (int x = 0; x < xBlockSize; ++x) {
					int pixelIndex = (y * xBlockSize + x) * bandCount;
					int r = buffer[pixelIndex];
					int g = buffer[pixelIndex + 1];
					int b = buffer[pixelIndex + 2];
					image.setPixel(x, y, qRgb(r, g, b));
				}
			}

			pixmap = QPixmap::fromImage(image.copy(0, 0, xBlockSize, yBlockSize));
			double minX = geoTransform[0] + xBlock * geoTransform[1];
			double maxY = geoTransform[3] + yBlock * geoTransform[5];
			double maxX = minX + xBlockSize * geoTransform[1];
			double minY = maxY + yBlockSize * geoTransform[5];
			QPointF topLeft = mapRasterToViewCoordinates(mpGraphicsScene, minX, maxY);
			QPointF bottomRight = mapRasterToViewCoordinates(mpGraphicsScene, maxX, minY);
			QRectF imageRect(topLeft, bottomRight);

			item = new RasterItem(pixmap, layerName);
			item->setPos(imageRect.topLeft());
			item->setScale(imageRect.width() / pixmap.width());
			mpGraphicsScene->addItem(item);

			items.push_back(item); // 将每个 RasterItem 添加到集合中

			sceneRect = sceneRect.united(imageRect);

			// 更新进度条
			progressDialog.setValue(++progressCounter);
			if (progressDialog.wasCanceled()) {
				// 移除已添加到场景中的 RasterItem 项
				for (RasterItem* item : items) {
					mpGraphicsScene->removeItem(item);
					delete item; // 删除动态分配的内存
				}
				GDALClose(dataset);
				return;
			}
		}
	}

	mpGraphicsView->fitInView(sceneRect, Qt::KeepAspectRatio);

	// 在 std::map 中存储 RasterItem 集合以便管理
	mvRasterItems[layerName] = items;

	// 将图层添加到图层管理结构中
	addLayerToTreeWidget(layerName, true);

	GDALClose(dataset);

	QMessageBox::information(this, tr("Success"), tr("超大栅格绘制成功."));

}


//**********************************底部bar显示经纬度//**********************************
//下方状态栏更新
void MYGIS::viewCoordinateChanged(QPointF scenePos)
{
	//qDebug() << "viewCoordinateChanged called with scenePos:" << scenePos;

	qreal scaleFactor = mpGraphicsView->transform().m11();
	double lon = scenePos.x();
	double lat = -scenePos.y();  // 翻转 Y 轴
	QString coordinateText = QString("经度: %1, 纬度: %2").arg(lon).arg(lat);
	QString zoomText = QString("缩放比例: 1:%1").arg(scaleFactor);
	statusBar()->showMessage(coordinateText + "  " + zoomText);
}


//**********************************图层管理//**********************************
//图层管理添加
//图标
void MYGIS::addLayerToTreeWidget(const QString& layerName, bool isRaster)
{
	QString fullLayerName = isRaster ? "[Raster] " + layerName : "[Vector] " + layerName;

	// 检查图层是否已经存在于树中，避免重复添加
	for (int i = 0; i < mpLayerTreeWidget->topLevelItemCount(); ++i) {
		if (mpLayerTreeWidget->topLevelItem(i)->text(0) == fullLayerName) {
			return;  // 图层已经存在，直接返回
		}
	}

	// 添加图层到树中
	QTreeWidgetItem* layerItem = new QTreeWidgetItem(mpLayerTreeWidget);
	layerItem->setText(0, fullLayerName); // 设置图层名称
	layerItem->setCheckState(0, Qt::Checked); // 添加复选框并默认勾选

	// 设置图层颜色和图标
	QPixmap pixmap(64, 32);
	pixmap.fill(isRaster ? Qt::gray : mvLayerColors[layerName]);
	layerItem->setIcon(0, QIcon(pixmap));

	mpLayerTreeWidget->addTopLevelItem(layerItem); // 添加图层到顶级

	// 连接复选框的状态改变信号到槽函数，用于控制图层的显示和隐藏
	connect(mpLayerTreeWidget, &QTreeWidget::itemChanged, this, &MYGIS::layerDisplays);
}

//更新图标颜色
void MYGIS::updateLayerIcon(QTreeWidgetItem* layerItem, const QString& layerName)
{
	// 获取图层颜色
	QColor layerColor = mvLayerColors[layerName];

	// 创建一个 64x32 的像素图标
	QPixmap pixmap(64, 32);
	pixmap.fill(Qt::transparent); // 透明背景
	QPainter painter(&pixmap);

	// 获取图层类型，并绘制相应的图标
	const VectorLayerData& layerData = mvVectorLayer[layerName];
	GeometryType geomType = layerData.getFeatures()[0].getGeometry().getGeometryType();

	if (geomType == GeometryType::Point || geomType == GeometryType::MultiPoint) {
		// 绘制点（圆形）
		painter.setBrush(layerColor);
		painter.setPen(Qt::NoPen);
		painter.drawEllipse(QRect(16, 8, 16, 16)); // 圆形
	}
	else if (geomType == GeometryType::Line || geomType == GeometryType::MultiLineString) {
		// 绘制线（有宽度的长条）
		painter.setBrush(layerColor);
		painter.setPen(Qt::NoPen);
		painter.drawRect(QRect(8, 12, 48, 8)); // 长条
	}
	else if (geomType == GeometryType::Polygon || geomType == GeometryType::MultiPolygon) {
		// 绘制面（方块），确保方块是正方形
		painter.setBrush(layerColor);
		painter.setPen(Qt::NoPen);
		painter.drawRect(QRect(16, 8, 32, 16)); // 正方形方块
	}

	// 设置绘制好的图标到 QTreeWidgetItem
	layerItem->setIcon(0, QIcon(pixmap));
}

//影藏和显示图层
void MYGIS::layerDisplays(QTreeWidgetItem* item, int column)
{
	if (column == 0) // 只处理复选框的状态改变
	{
		QString layerName = item->text(0);
		bool isChecked = (item->checkState(0) == Qt::Checked);

		if (layerName.startsWith("[Vector] ")) {
			layerName = layerName.mid(9); // 去掉前缀以获取实际的图层名称
			for (QGraphicsItem* graphicsItem : mpGraphicsScene->items())
			{
				GeometryItem* geomItem = dynamic_cast<GeometryItem*>(graphicsItem);
				if (geomItem && geomItem->getLayerName() == layerName)
				{
					geomItem->setVisible(isChecked);
				}
			}
		}
		else if (layerName.startsWith("[Raster] ")) {
			layerName = layerName.mid(9); // 去掉前缀以获取实际的图层名称
			auto it = mvRasterItems.find(layerName);
			if (it != mvRasterItems.end()) {
				for (auto& rasterItem : it->second) {
					rasterItem->setVisible(isChecked);
				}
			}
		}

		mpGraphicsView->update(); // 更新视图
	}
}

//图层管理操作菜单
void MYGIS::showLayerContextMenu(const QPoint& pos)
{
	QTreeWidgetItem* selectedItem = mpLayerTreeWidget->itemAt(pos);
	if (selectedItem)
	{
		QMenu contextMenu(this);
		QAction* zoomToLayerAction = contextMenu.addAction("缩放到图层");
		QAction* removeLayerAction = contextMenu.addAction("移除图层");

		// 添加第一个分隔符
		contextMenu.addSeparator();

		// 仅为矢量图层添加这些功能
		if (selectedItem->text(0).startsWith("[Vector] ")) {
			QAction* showAttributeDataAction = contextMenu.addAction("显示属性表");
			QAction* layerFeatureCountAction = contextMenu.addAction("图层要素总数");
			QAction* layerColorAction = contextMenu.addAction("图层颜色");
			connect(layerFeatureCountAction, &QAction::triggered, this, [this, selectedItem]() { updateLayerFeatureCount(selectedItem); });
			connect(showAttributeDataAction, &QAction::triggered, this, [this, selectedItem]() { showLayerAttributeData(selectedItem); });
			connect(layerColorAction, &QAction::triggered, this, [this, selectedItem]() { layerColorSystem(); });
		}

		// 添加第一个分隔符
		contextMenu.addSeparator();

		QAction* moveToTopAction = contextMenu.addAction("上移到顶部");
		QAction* moveToBottomAction = contextMenu.addAction("下移动到底部");

		connect(zoomToLayerAction, &QAction::triggered, this, [this, selectedItem]() { zoomToLayer(selectedItem); });
		connect(removeLayerAction, &QAction::triggered, this, [this, selectedItem]() { removeLayer(selectedItem); });
		connect(moveToTopAction, &QAction::triggered, this, [this, selectedItem]() { layerMoveToTop(selectedItem); });
		connect(moveToBottomAction, &QAction::triggered, this, [this, selectedItem]() { layerMoveToBottom(selectedItem); });

		contextMenu.exec(mpLayerTreeWidget->viewport()->mapToGlobal(pos));
	}
}

// 缩放至图层
void MYGIS::zoomToLayer(QTreeWidgetItem* item)
{
	QString layerName = item->text(0);

	// 处理矢量图层的缩放
	if (layerName.startsWith("[Vector] ")) {
		layerName = layerName.mid(9); // 去掉前缀以获取实际的图层名称
		if (mvVectorLayer.find(layerName) != mvVectorLayer.end()) {
			QRectF layerBoundingRect; // 用于存储图层的联合边界矩形
			bool foundLayerItems = false; // 标记是否找到图层项

			// 遍历场景中的所有图形项，找到属于指定图层的几何项
			for (QGraphicsItem* graphicsItem : mpGraphicsScene->items())
			{
				GeometryItem* geomItem = dynamic_cast<GeometryItem*>(graphicsItem);
				if (geomItem && geomItem->getLayerName() == layerName)
				{
					foundLayerItems = true;
					QRectF itemBoundingRect = geomItem->boundingRect();
					layerBoundingRect = layerBoundingRect.united(itemBoundingRect); // 合并边界矩形
				}
			}

			// 如果找到了图层项且联合边界矩形不为空，调整视图以适应该矩形
			if (foundLayerItems && !layerBoundingRect.isNull())
			{
				mpGraphicsView->fitInView(layerBoundingRect, Qt::KeepAspectRatio);
			}
		}
	}
	// 处理栅格图层的缩放
	else if (layerName.startsWith("[Raster] ")) {
		layerName = layerName.mid(9); // 去掉前缀以获取实际的图层名称
		if (mvRasterItems.find(layerName) != mvRasterItems.end()) {
			QRectF layerBoundingRect; // 用于存储栅格图层的联合边界矩形
			bool foundLayerItems = false; // 标记是否找到图层项

			// 遍历所有栅格项，找到属于指定图层的项并合并边界矩形
			for (auto& rasterItem : mvRasterItems[layerName]) {
				foundLayerItems = true;
				QRectF itemBoundingRect = rasterItem->mapRectToScene(rasterItem->boundingRect());
				layerBoundingRect = layerBoundingRect.united(itemBoundingRect);
			}

			// 如果找到了图层项且联合边界矩形不为空，调整视图以适应该矩形
			if (foundLayerItems && !layerBoundingRect.isNull())
			{
				mpGraphicsView->fitInView(layerBoundingRect, Qt::KeepAspectRatio);
			}
		}
	}
}


//移除图层
void MYGIS::removeLayer(QTreeWidgetItem* item)
{
	QString layerName = item->text(0);

	// 移除图层名称的前缀 "[Vector] " 或 "[Raster] "
	if (layerName.startsWith("[Vector] ")) {
		layerName = layerName.mid(9); // 去掉 "[Vector] " 的前缀
	}
	else if (layerName.startsWith("[Raster] ")) {
		layerName = layerName.mid(9); // 去掉 "[Raster] " 的前缀
	}

	// 首先检查并移除矢量图层
	if (mvVectorLayer.find(layerName) != mvVectorLayer.end())
	{
		mvVectorLayer.erase(layerName);
		mvLayerColors.erase(layerName);

		if (mpProjectManager) {
			mpProjectManager->removeLayer(layerName);
		}

		QList<QGraphicsItem*> items = mpGraphicsScene->items();
		for (QGraphicsItem* graphicsItem : items)
		{
			GeometryItem* geomItem = dynamic_cast<GeometryItem*>(graphicsItem);
			if (geomItem && geomItem->getLayerName() == layerName)
			{
				mpGraphicsScene->removeItem(geomItem);
				delete geomItem;
			}
		}

		log4cpp::Category& logger = log4cpp::Category::getRoot();
		logger.info("Vector layer %s has been removed.", layerName.toStdString().c_str());
	}
	else if (mvRasterItems.find(layerName) != mvRasterItems.end()) // 检查 mvRasterItems
	{
		// 移除存储的 RasterItems
		for (auto& rasterItem : mvRasterItems[layerName]) {
			mpGraphicsScene->removeItem(rasterItem);
			delete rasterItem;
		}
		mvRasterItems.erase(layerName);

		if (mpProjectManager) {
			mpProjectManager->removeLayer(layerName);
		}

		log4cpp::Category& logger = log4cpp::Category::getRoot();
		logger.info("Raster layer %s has been removed.", layerName.toStdString().c_str());
	}
	else
	{
		log4cpp::Category& logger = log4cpp::Category::getRoot();
		logger.warn("Attempted to remove non-existent layer: %s", layerName.toStdString().c_str());
		return;
	}

	delete item;
	mpGraphicsView->update();
}

// 实现 图层属性表函数
void MYGIS::showLayerAttributeData(QTreeWidgetItem* item)
{
	QString layerName = item->text(0).remove("[Vector] "); // 去除 "[Vector] " 前缀// 获取图层名称
	if (mvVectorLayer.find(layerName) != mvVectorLayer.end())
	{
		// 创建新的 QDockWidget 来显示属性表，标题为 "图层名称 属性表"
		QDockWidget* dockWidget = new QDockWidget(layerName + " 属性表", this);

		// 创建 QTableWidget 来显示属性数据，并将其设置为 QDockWidget 的中央控件
		QTableWidget* tableWidget = new QTableWidget(dockWidget);
		dockWidget->setWidget(tableWidget);

		// 获取图层数据
		const VectorLayerData& layerData = mvVectorLayer[layerName];
		const std::vector<FeatureData>& features = layerData.getFeatures();

		// 设置表格的行列数
		if (!features.empty()) {
			const FeatureData& firstFeature = features[0];
			int columnCount = firstFeature.getAttributeKeys().size(); // 获取属性的列数
			int rowCount = static_cast<int>(features.size());// 获取要素的行数

			tableWidget->setColumnCount(columnCount);
			tableWidget->setRowCount(rowCount);

			// 设置表头，使用属性的键作为表头
			QStringList headers = firstFeature.getAttributeKeys();
			tableWidget->setHorizontalHeaderLabels(headers);

			// 填充表格数据
			for (int row = 0; row < rowCount; ++row) {
				const FeatureData& feature = features[row];
				for (int col = 0; col < columnCount; ++col) {
					QString key = headers[col];
					std::variant<int, double, QString> value = feature.getAttributes().getAttribute(key);
					QString valueString;

					// 判断属性值的类型，并转换为字符串
					if (std::holds_alternative<int>(value)) {
						valueString = QString::number(std::get<int>(value));
					}
					else if (std::holds_alternative<double>(value)) {
						valueString = QString::number(std::get<double>(value));
					}
					else if (std::holds_alternative<QString>(value)) {
						valueString = std::get<QString>(value);
					}

					// 将属性值添加到表格中
					tableWidget->setItem(row, col, new QTableWidgetItem(valueString));
				}
			}
		}

		// 连接 itemSelectionChanged 信号到 onAttributeItemSelected 槽函数
		connect(tableWidget, &QTableWidget::itemSelectionChanged, this, [this, tableWidget, layerName]() {
			QTableWidgetItem* selectedItem = tableWidget->currentItem();
			if (selectedItem) {
				onAttributeItemSelected(selectedItem, layerName);
			}
			});

		// 设置 QDockWidget 的默认大小为 800x500，并且设置最小大小为 400x200，防止自动缩小
		dockWidget->resize(800, 500);
		dockWidget->setMinimumSize(400, 200);

		// 将 QDockWidget 添加到主窗口中，默认停靠在中间底部
		addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

		// 调整停靠窗口的位置，使其居中
		setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
		setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);

		dockWidget->show(); // 显示 QDockWidget
	}
}
//属性表点击高亮图层
void MYGIS::onAttributeItemSelected(QTableWidgetItem* item, const QString& layerName) {
	// 首先检查是否有选中的项
	if (!item) {
		return;
	}

	// 获取属性表中被选中项的行数，对应要素的ID
	int row = item->row();
	int featureId = row; // 假设属性表的行数与要素ID一致

	// 查找对应的GeometryItem
	for (QGraphicsItem* graphicsItem : mpGraphicsScene->items()) {
		GeometryItem* geomItem = dynamic_cast<GeometryItem*>(graphicsItem);
		if (geomItem && geomItem->getLayerName() == layerName && geomItem->getId() == featureId) {
			// 改变选中要素的颜色为黄色
			geomItem->setColor(Qt::yellow);
		}
	}

	// 连接属性表的 itemSelectionChanged 信号到槽函数，用于取消选择时恢复原始颜色
	connect(item->tableWidget(), &QTableWidget::itemSelectionChanged, this, [this, item, layerName]() {
		QList<QTableWidgetItem*> selectedItems = item->tableWidget()->selectedItems();
		if (selectedItems.isEmpty()) {
			// 如果没有选中的项，恢复所有高亮要素的颜色
			for (QGraphicsItem* graphicsItem : mpGraphicsScene->items()) {
				GeometryItem* geomItem = dynamic_cast<GeometryItem*>(graphicsItem);
				if (geomItem && geomItem->getLayerName() == layerName) {
					geomItem->setColor(geomItem->getOriginalColor());  // 恢复为原始颜色
				}
			}
		}
		});
}
// 图层上移到顶部
void MYGIS::layerMoveToTop(QTreeWidgetItem* item)
{
	QString layerName = item->text(0);

	// 处理矢量图层
	if (layerName.startsWith("[Vector] ")) {
		layerName = layerName.mid(9); // 移除前缀 "[Vector] "
		if (mvVectorLayer.find(layerName) != mvVectorLayer.end())
		{
			int maxZValue = 0;
			QList<QGraphicsItem*> items = mpGraphicsScene->items();

			// 找到当前场景中所有图层的最大Z值
			for (QGraphicsItem* graphicsItem : items) {
				maxZValue = std::max(maxZValue, static_cast<int>(graphicsItem->zValue()));
			}

			int newZValue = maxZValue + 1;
			// 更新与指定图层名称匹配的所有几何项的Z值，使其移到顶部
			for (QGraphicsItem* graphicsItem : items)
			{
				GeometryItem* geomItem = dynamic_cast<GeometryItem*>(graphicsItem);
				if (geomItem && geomItem->getLayerName() == layerName)
				{
					geomItem->setZValue(newZValue++);
				}
			}
		}
	}
	// 处理栅格图层
	else if (layerName.startsWith("[Raster] ")) {
		layerName = layerName.mid(9); // 移除前缀 "[Raster] "
		if (mvRasterItems.find(layerName) != mvRasterItems.end()) {
			int maxZValue = 0;
			QList<QGraphicsItem*> items = mpGraphicsScene->items();

			// 找到当前场景中所有图层的最大Z值
			for (QGraphicsItem* graphicsItem : items) {
				maxZValue = std::max(maxZValue, static_cast<int>(graphicsItem->zValue()));
			}

			// 更新指定栅格图层的Z值，使其移到顶部
			for (auto& rasterItem : mvRasterItems[layerName]) {
				rasterItem->setZValue(maxZValue + 1);
				maxZValue++;
			}
		}
	}
}

// 图层下移到底部
void MYGIS::layerMoveToBottom(QTreeWidgetItem* item)
{
	QString layerName = item->text(0);

	// 处理矢量图层
	if (layerName.startsWith("[Vector] ")) {
		layerName = layerName.mid(9); // 移除前缀 "[Vector] "
		if (mvVectorLayer.find(layerName) != mvVectorLayer.end())
		{
			int minZValue = 0;
			QList<QGraphicsItem*> items = mpGraphicsScene->items();

			// 找到当前场景中所有图层的最小Z值
			for (QGraphicsItem* graphicsItem : items) {
				minZValue = std::min(minZValue, static_cast<int>(graphicsItem->zValue()));
			}

			int newZValue = minZValue - 1;
			// 更新与指定图层名称匹配的所有几何项的Z值，使其移到底部
			for (QGraphicsItem* graphicsItem : items)
			{
				GeometryItem* geomItem = dynamic_cast<GeometryItem*>(graphicsItem);
				if (geomItem && geomItem->getLayerName() == layerName)
				{
					geomItem->setZValue(newZValue--);
				}
			}
		}
	}
	// 处理栅格图层
	else if (layerName.startsWith("[Raster] ")) {
		layerName = layerName.mid(9); // 移除前缀 "[Raster] "
		if (mvRasterItems.find(layerName) != mvRasterItems.end()) {
			int minZValue = 0;
			QList<QGraphicsItem*> items = mpGraphicsScene->items();

			// 找到当前场景中所有图层的最小Z值
			for (QGraphicsItem* graphicsItem : items) {
				minZValue = std::min(minZValue, static_cast<int>(graphicsItem->zValue()));
			}

			// 更新指定栅格图层的Z值，使其移到底部
			for (auto& rasterItem : mvRasterItems[layerName]) {
				rasterItem->setZValue(minZValue - 1);
				minZValue--;
			}
		}
	}
}


//图层颜色系统 
void MYGIS::layerColorSystem()
{
	QTreeWidgetItem* selectedItem = mpLayerTreeWidget->currentItem();
	if (!selectedItem) {
		return;
	}

	QString layerName = selectedItem->text(0);
	if (layerName.startsWith("[Vector] ")) {
		layerName = layerName.mid(9);

		if (mvVectorLayer.find(layerName) != mvVectorLayer.end()) {
			QColor selectedColor = QColorDialog::getColor(Qt::white, this, "选择图层颜色");
			if (!selectedColor.isValid()) {
				return;
			}

			mvLayerColors[layerName] = selectedColor;

			for (QGraphicsItem* graphicsItem : mpGraphicsScene->items())
			{
				GeometryItem* geomItem = dynamic_cast<GeometryItem*>(graphicsItem);
				if (geomItem && geomItem->getLayerName() == layerName)
				{
					geomItem->setColor(selectedColor);
				}
			}

			updateLayerIcon(selectedItem, layerName);
			mpGraphicsView->update();
		}
	}
}
// 更新图层要素总数显示
void MYGIS::updateLayerFeatureCount(QTreeWidgetItem* item)
{
	QString layerName = item->text(0);

	if (layerName.startsWith("[Vector] ")) {
		layerName = layerName.mid(9); // 去掉前缀以获取实际的图层名称

		if (mvVectorLayer.find(layerName) != mvVectorLayer.end())
		{
			int featureCount = static_cast<int>(mvVectorLayer[layerName].getFeatures().size());

			// 设置 ToolTip 来显示要素总数
			item->setToolTip(0, QString("图层: %1\n要素总数: %2").arg(layerName).arg(featureCount));
		}
	}
	else if (layerName.startsWith("[Raster] ")) {
		layerName = layerName.mid(9); // 去掉前缀以获取实际的图层名称

		// 栅格图层没有要素总数，可以设置其他相关信息或不设置
		item->setToolTip(0, QString("图层: %1\n这是一个栅格图层").arg(layerName));
	}
}



//**********************************编辑模式//**********************************
//编辑模式
void MYGIS::toggleEditingMode()
{
	mEditingMode = !mEditingMode;
	//qDebug() << "Editing mode toggled. New state:" << mEditingMode;

	if (!mEditingMode) {
		for (GeometryItem* item : mvSelectedItems) {
			item->setColor(item->getOriginalColor());
		}
		//qDebug() << "Exiting edit mode. Clearing selected items. Before clear: " << mvSelectedItems.size();
		mvSelectedItems.clear();
		//qDebug() << "After clear: " << mvSelectedItems.size();
	}
	mpGraphicsView->update();
}
// 删除所选要素
void MYGIS::deleteSelectedItems() {
	if (!mEditingMode) {
		//qDebug() << "Not in editing mode, deletion aborted.";
		return;
	}

	for (GeometryItem* item : mvSelectedItems) {
		QString layerName = item->getLayerName();
		int itemId = item->getId(); // 获取 GeometryItem 的 ID

		if (mvVectorLayer.find(layerName) != mvVectorLayer.end()) {
			auto& vectorLayer = mvVectorLayer[layerName];

			// 根据 ID 找到并保存删除的 FeatureData 对象
			auto it = std::find_if(vectorLayer.getFeatures().begin(), vectorLayer.getFeatures().end(),
				[itemId](const FeatureData& feature) {
					return feature.getId() == itemId;
				});

			if (it != vectorLayer.getFeatures().end()) {
				FeatureData deletedFeature = *it;
				mvDeletedFeatures.push_back(deletedFeature); // 存储 FeatureData
				vectorLayer.getFeatures().erase(it); // 从图层数据中删除

				//qDebug() << "Deleted Feature ID:" << deletedFeature.getId();
			}

			// 临时保存删除的几何项，便于撤回
			mvDeletedItems.push_back(item);
			mpGraphicsScene->removeItem(item);
			//qDebug() << "Deleted GeometryItem ID:" << item->getId() << "from layer:" << layerName;
			// 
			// 在删除操作后更新受影响图层的要素总数
			QTreeWidgetItem* layerItem = findLayerItemByName(layerName);
			if (layerItem) {
				updateLayerFeatureCount(layerItem);
			}
		}
	}

	mvSelectedItems.clear();
	mpGraphicsScene->update();
	mpGraphicsView->update();
	UpdateAttributeTables();

}

// 撤回删除操作
void MYGIS::withdrawDelete() {
	if (!mEditingMode || mvDeletedItems.empty() || mvDeletedFeatures.empty()) return;

	// 遍历所有被删除的几何项
	for (int i = 0; i < mvDeletedItems.size(); ++i) {
		GeometryItem* item = mvDeletedItems[i];
		int itemId = item->getId();

		// 根据 ID 找到对应的 FeatureData
		auto it = std::find_if(mvDeletedFeatures.begin(), mvDeletedFeatures.end(),
			[itemId](const FeatureData& feature) {
				return feature.getId() == itemId;
			});

		if (it != mvDeletedFeatures.end()) {
			FeatureData restoredFeature = *it;
			QString layerName = item->getLayerName();

			if (mvVectorLayer.find(layerName) != mvVectorLayer.end()) {
				auto& vectorLayer = mvVectorLayer[layerName];

				// 恢复 FeatureData 到图层中
				vectorLayer.addFeature(restoredFeature);

				// 重新添加 GeometryItem 到视图中
				mpGraphicsScene->addItem(item);

				//qDebug() << "Restored item ID:" << restoredFeature.getId() << "to layer:" << layerName;

				// 从 mvDeletedFeatures 中移除已恢复的要素
				mvDeletedFeatures.erase(it);

				// 在撤回操作后更新图层要素总数
				QTreeWidgetItem* layerItem = findLayerItemByName(layerName);
				if (layerItem) {
					updateLayerFeatureCount(layerItem);
				}
			}
		}
	}


	mvDeletedItems.clear();
	mpGraphicsScene->update();
	mpGraphicsView->update();
	UpdateAttributeTables();
}
// 保存编辑操作
void MYGIS::saveEdit() {
	if (!mEditingMode) return;

	mvDeletedItems.clear();
	mvDeletedFeatures.clear(); // 删除操作不可撤回

	mIsSaved = true; // 标识已保存

	log4cpp::Category& logger = log4cpp::Category::getRoot();
	logger.info("Edit operations have been saved.");

	mpGraphicsView->update();
}
// 退出编辑状态
void MYGIS::endEdit() {
	if (!mEditingMode) return;

	if (!mIsSaved) {
		// 如果未保存编辑，则撤回删除操作
		withdrawDelete();
	}

	// 直接使用图层的当前颜色来恢复所有要素的颜色
	for (QGraphicsItem* item : mpGraphicsScene->items()) {
		GeometryItem* geomItem = dynamic_cast<GeometryItem*>(item);
		if (geomItem && geomItem->isHighlighted()) {
			// 获取当前图层的颜色
			QColor layerColor = mvLayerColors[geomItem->getLayerName()];

			// 直接将要素颜色恢复为当前图层颜色
			geomItem->setColor(layerColor);
		}
	}

	// 退出编辑模式
	mEditingMode = false;
	mIsSaved = false; // 重置保存状态

	// 更新视图
	mpGraphicsView->update();
}

//**********************************预览设置//**********************************
//重置默认视图为默认
void MYGIS::resetView()
{
	// 确保 BrowserdockWidget 可见并恢复到停靠状态
	if (ui.BrowserdockWidget) {
		ui.BrowserdockWidget->show();  // 显示
		ui.BrowserdockWidget->setFloating(false);  // 取消浮动
		ui.BrowserdockWidget->raise();  // 提升到前面
	}
	else {
		qWarning() << "BrowserdockWidget not found!";
	}

	// 确保 LayerdockWidget 可见并恢复到停靠状态
	if (ui.LayerdockWidget) {
		ui.LayerdockWidget->show();  // 显示
		ui.LayerdockWidget->setFloating(false);  // 取消浮动
		ui.LayerdockWidget->raise();  // 提升到前面
	}
	else {
		qWarning() << "LayerdockWidget not found!";
	}

	// 确保 TooldockWidget 可见并恢复到停靠状态
	if (ui.TooldockWidget) {
		ui.TooldockWidget->show();  // 显示
		ui.TooldockWidget->setFloating(false);  // 取消浮动
		ui.TooldockWidget->raise();  // 提升到前面
	}
	else {
		qWarning() << "TooldockWidget not found!";
	}
}

//开始预览模式
void MYGIS::startPreviewMode()
{
	// 隐藏 BrowserdockWidget
	if (ui.BrowserdockWidget) {
		ui.BrowserdockWidget->setVisible(false);  // 隐藏
	}
	else {
		qWarning() << "BrowserdockWidget not found!";
	}

	// 隐藏 LayerdockWidget
	if (ui.LayerdockWidget) {
		ui.LayerdockWidget->setVisible(false);  // 隐藏
	}
	else {
		qWarning() << "LayerdockWidget not found!";
	}

	// 隐藏 TooldockWidget
	if (ui.TooldockWidget) {
		ui.TooldockWidget->setVisible(false);  // 隐藏
	}
	else {
		qWarning() << "TooldockWidget not found!";
	}
}

//结束预览模式
void MYGIS::endPreviewMode()
{
	// 显示 BrowserdockWidget
	if (ui.BrowserdockWidget) {
		ui.BrowserdockWidget->show();  // 使用 show() 显示
	}
	else {
		qWarning() << "BrowserdockWidget not found!";
	}

	// 显示 LayerdockWidget
	if (ui.LayerdockWidget) {
		ui.LayerdockWidget->show();  // 使用 show() 显示
	}
	else {
		qWarning() << "LayerdockWidget not found!";
	}

	// 显示 TooldockWidget
	if (ui.TooldockWidget) {
		ui.TooldockWidget->show();  // 使用 show() 显示
	}
	else {
		qWarning() << "TooldockWidget not found!";
	}
}


//**********************************保存矢量和导入接口//**********************************
//提供一个要素转shapefile的接口
bool MYGIS::saveVectorLayerAsShapefile(const QString& layerName, const QString& outputPath)
{
	// 提取文件名
	QString _fileName = QFileInfo(outputPath).fileName();

	// 检查文件名是否包含中文字符
	for (int i = 0; i < _fileName.length(); ++i) {
		if (_fileName[i].unicode() > 127) { // Unicode值大于127表示非ASCII字符
			QMessageBox::warning(nullptr, tr("Error"), tr("文件名包含中文字符，请使用英文命名并保存."));
			return false;
		}
	}

	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "YES"); // 支持中文路径
	CPLSetConfigOption("SHAPE_ENCODING", "GBK"); // 设置 Shapefile 编码为 GBK

	const char* pszDriverName = "ESRI Shapefile";
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
	if (poDriver == nullptr)
	{
		QMessageBox::critical(this, tr("Error"), tr("Shapefile driver not available."));
		return false;
	}

	// 创建新的 Shapefile 数据集
	GDALDataset* poDS = poDriver->Create(outputPath.toUtf8().constData(), 0, 0, 0, GDT_Unknown, nullptr);
	if (poDS == nullptr)
	{
		QMessageBox::critical(this, tr("Error"), tr("Failed to create shapefile."));
		return false;
	}

	// 查找并保存指定的图层
	if (mvVectorLayer.find(layerName) != mvVectorLayer.end())
	{
		const VectorLayerData& layerData = mvVectorLayer[layerName];

		OGRSpatialReference oSRS;
		oSRS.SetWellKnownGeogCS("WGS84");

		// 创建 Shapefile 图层
		OGRLayer* poLayer = poDS->CreateLayer(layerName.toUtf8().constData(), &oSRS, wkbUnknown, nullptr);
		if (poLayer == nullptr)
		{
			QMessageBox::critical(this, tr("Error"), tr("Failed to create layer in shapefile."));
			GDALClose(poDS);
			return false;
		}

		// 创建字段定义（假设属性字段的类型已知）
		if (!layerData.getFeatures().empty()) {
			const FeatureData& firstFeature = layerData.getFeatures().at(0);
			for (const QString& key : firstFeature.getAttributeKeys()) {
				OGRFieldDefn oField(key.toUtf8().constData(), OFTString); // 根据属性类型调整
				poLayer->CreateField(&oField);
			}
		}

		// 添加要素到图层
		for (const FeatureData& feature : layerData.getFeatures())
		{
			OGRFeature* poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());

			// 设置属性
			for (const QString& key : feature.getAttributeKeys())
			{
				std::variant<int, double, QString> value = feature.getAttributes().getAttribute(key);
				if (std::holds_alternative<int>(value))
					poFeature->SetField(key.toUtf8().constData(), std::get<int>(value));
				else if (std::holds_alternative<double>(value))
					poFeature->SetField(key.toUtf8().constData(), std::get<double>(value));
				else if (std::holds_alternative<QString>(value))
					poFeature->SetField(key.toUtf8().constData(), std::get<QString>(value).toUtf8().constData());
			}

			// 设置几何数据
			const GeometryData& geomData = feature.getGeometry();
			OGRGeometry* poGeometry = nullptr;

			// 根据几何类型创建对应的几何对象
			if (geomData.getGeometryType() == GeometryType::Point)
			{
				OGRPoint* poPoint = new OGRPoint();
				poPoint->setX(std::get<std::vector<std::vector<double>>>(geomData.getCoordinates())[0][0]);
				poPoint->setY(std::get<std::vector<std::vector<double>>>(geomData.getCoordinates())[0][1]);
				poGeometry = poPoint;
			}
			else if (geomData.getGeometryType() == GeometryType::Line)
			{
				OGRLineString* poLine = new OGRLineString();
				for (const auto& coord : std::get<std::vector<std::vector<double>>>(geomData.getCoordinates()))
				{
					poLine->addPoint(coord[0], coord[1]);
				}
				poGeometry = poLine;
			}
			else if (geomData.getGeometryType() == GeometryType::Polygon)
			{
				OGRPolygon* poPolygon = new OGRPolygon();
				OGRLinearRing* poRing = new OGRLinearRing();
				for (const auto& coord : std::get<std::vector<std::vector<double>>>(geomData.getCoordinates()))
				{
					poRing->addPoint(coord[0], coord[1]);
				}
				poPolygon->addRing(poRing);
				poGeometry = poPolygon;
			}
			else if (geomData.getGeometryType() == GeometryType::MultiPoint)
			{
				OGRMultiPoint* poMultiPoint = new OGRMultiPoint();
				for (const auto& point : std::get<std::vector<std::vector<double>>>(geomData.getCoordinates()))
				{
					OGRPoint* poPoint = new OGRPoint(point[0], point[1]);
					poMultiPoint->addGeometry(poPoint);
				}
				poGeometry = poMultiPoint;
			}
			else if (geomData.getGeometryType() == GeometryType::MultiLineString)
			{
				OGRMultiLineString* poMultiLine = new OGRMultiLineString();
				for (const auto& line : std::get<std::vector<std::vector<std::vector<double>>>>(geomData.getCoordinates()))
				{
					OGRLineString* poLine = new OGRLineString();
					for (const auto& coord : line)
					{
						poLine->addPoint(coord[0], coord[1]);
					}
					poMultiLine->addGeometry(poLine);
				}
				poGeometry = poMultiLine;
			}
			else if (geomData.getGeometryType() == GeometryType::MultiPolygon)
			{
				OGRMultiPolygon* poMultiPolygon = new OGRMultiPolygon();
				for (const auto& polygon : std::get<std::vector<std::vector<std::vector<double>>>>(geomData.getCoordinates()))
				{
					OGRPolygon* poPolygon = new OGRPolygon();
					OGRLinearRing* poRing = new OGRLinearRing();
					for (const auto& coord : polygon)
					{
						poRing->addPoint(coord[0], coord[1]);
					}
					poPolygon->addRing(poRing);
					poMultiPolygon->addGeometry(poPolygon);
				}
				poGeometry = poMultiPolygon;
			}

			if (poGeometry != nullptr)
			{
				poFeature->SetGeometry(poGeometry);
				if (poLayer->CreateFeature(poFeature) != OGRERR_NONE)
				{
					QMessageBox::critical(this, tr("Error"), tr("Failed to create feature in shapefile."));
					OGRFeature::DestroyFeature(poFeature);
					GDALClose(poDS);
					return false;
				}
				OGRFeature::DestroyFeature(poFeature);
			}
		}
	}
	else
	{
		QMessageBox::warning(this, tr("Warning"), tr("Layer not found."));
		GDALClose(poDS);
		return false;
	}

	GDALClose(poDS); // 保存并关闭数据源
	//QMessageBox::information(this, tr("Success"), tr("成功保存为shapefile."));
	
	// 在保存完 Shapefile 后重置编码配置
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", nullptr); // 还原到默认值
	CPLSetConfigOption("SHAPE_ENCODING", nullptr); // 还原到默认值

	return true;
}

//计算几何
bool MYGIS::calculateGeometry(const QString& layerName, const QString& lineField, const QString& areaField) {
	auto it = mvVectorLayer.find(layerName);
	if (it == mvVectorLayer.end()) {
		QMessageBox::warning(this, tr("Error"), tr("Layer not found!"));
		return false; // 返回false表示操作失败
	}

	VectorLayerData& layerData = it->second;

	// 创建地理坐标系（WGS84）
	OGRSpatialReference oSourceSRS;
	oSourceSRS.SetWellKnownGeogCS("WGS84");

	bool success = true; // 操作标志

	for (auto& feature : layerData.getFeatures()) {
		const GeometryData& geomData = feature.getGeometry();
		OGRGeometry* ogrGeometry = createOGRGeometryFromGeometryData(geomData);

		if (ogrGeometry) {
			// 计算质心来确定 UTM 区号
			OGRPoint centroid;
			ogrGeometry->Centroid(&centroid);
			double longitude = centroid.getX();
			int utmZone = static_cast<int>((longitude + 180) / 6) + 1;

			// 创建投影坐标系（UTM投影）
			OGRSpatialReference oTargetSRS;
			oTargetSRS.SetUTM(utmZone, centroid.getY() >= 0); // 北半球

			OGRCoordinateTransformation* poCT = OGRCreateCoordinateTransformation(&oSourceSRS, &oTargetSRS);
			if (poCT) {
				ogrGeometry->transform(poCT); // 执行投影转换

				double length = 0.0;
				double area = 0.0;

				// 根据几何类型计算长度或面积
				if (ogrGeometry->getGeometryType() == wkbLineString) { // 线
					OGRLineString* lineString = dynamic_cast<OGRLineString*>(ogrGeometry);
					if (lineString) {
						length = lineString->get_Length();
						QString lengthString = QString::number(length, 'f', 6);//保留六位
						feature.getAttributesChangge().addAttribute(lineField, lengthString);
					}
				}

				if (ogrGeometry->getGeometryType() == wkbPolygon) { // 面
					OGRPolygon* polygon = dynamic_cast<OGRPolygon*>(ogrGeometry);
					if (polygon) {
						area = polygon->get_Area();
						QString areaString = QString::number(area, 'f', 6);//保留六位
						feature.getAttributesChangge().addAttribute(areaField, areaString);
					}
				}

				OGRCoordinateTransformation::DestroyCT(poCT);
			}
			else {
				success = false; // 如果转换失败，设置操作标志为false
			}

			OGRGeometryFactory::destroyGeometry(ogrGeometry);
		}
		else {
			success = false; // 如果几何转换失败，设置操作标志为false
		}
	}

	if (success) {
		//QMessageBox::information(this, tr("Success"), tr("Geometry calculations are completed."));
	}

	UpdateAttributeTables();//更新属性表
	return success; // 返回操作结果
}

//重新添加图层函数能够将保存的shp写回图层
void MYGIS::addVectorLayer(const QString& filePath) {
	// 打开指定的 Shapefile 文件
	GDALDataset* poDS = static_cast<GDALDataset*>(GDALOpenEx(filePath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL));
	if (poDS == nullptr)
	{
		QMessageBox::critical(this, tr("Error"), tr("Failed to open shapefile."));
		return;
	}

	QString baseName = QFileInfo(filePath).baseName();

	// 遍历所有图层并读取数据
	for (int i = 0; i < poDS->GetLayerCount(); i++)
	{
		OGRLayer* poLayer = poDS->GetLayer(i);
		if (poLayer == nullptr)
		{
			log4cpp::Category& logger = log4cpp::Category::getRoot();
			logger.warn("Warning: Layer %d is null!", i);
			continue;
		}

		// 创建一个新的 VectorLayerData 对象来存储当前图层的数据
		VectorLayerData layerData;

		// 读取要素
		OGRFeature* poFeature;
		poLayer->ResetReading();
		while ((poFeature = poLayer->GetNextFeature()) != nullptr)
		{
			int featureId = poFeature->GetFID();
			OGRGeometry* poGeometry = poFeature->GetGeometryRef();
			GeometryType geomType = GeometryType::Point;  // 初始化几何类型
			std::vector<std::vector<double>> coordinates;
			std::vector<std::vector<std::vector<double>>> multiPolygonCoordinates;

			// 同 readVectorLayer 类似的几何数据处理逻辑
			if (poGeometry != nullptr)
			{
				const char* geometryName = poGeometry->getGeometryName(); // 获取原始几何类型的名称

				if (wkbFlatten(poGeometry->getGeometryType()) == wkbPoint)
				{
					geomType = GeometryType::Point;
					OGRPoint* poPoint = static_cast<OGRPoint*>(poGeometry);
					coordinates.push_back({ poPoint->getX(), poPoint->getY() });
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbLineString)
				{
					geomType = GeometryType::Line;
					OGRLineString* poLine = static_cast<OGRLineString*>(poGeometry);
					for (int j = 0; j < poLine->getNumPoints(); j++)
					{
						double x = poLine->getX(j);
						double y = poLine->getY(j);
						coordinates.push_back({ x, y });
					}
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon)
				{
					geomType = GeometryType::Polygon;
					OGRPolygon* poPolygon = static_cast<OGRPolygon*>(poGeometry);
					OGRLinearRing* poRing = poPolygon->getExteriorRing();
					for (int j = 0; j < poRing->getNumPoints(); j++)
					{
						double x = poRing->getX(j);
						double y = poRing->getY(j);
						coordinates.push_back({ x, y });
					}
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPoint)
				{
					geomType = GeometryType::MultiPoint;
					//qDebug() << "Reading geometry type: MultiPoint";
					OGRMultiPoint* poMultiPoint = static_cast<OGRMultiPoint*>(poGeometry);
					for (int j = 0; j < poMultiPoint->getNumGeometries(); j++)
					{
						OGRGeometry* poPointGeometry = poMultiPoint->getGeometryRef(j);
						OGRPoint* poPoint = static_cast<OGRPoint*>(poPointGeometry);
						coordinates.push_back({ poPoint->getX(), poPoint->getY() });
					}
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbMultiLineString)
				{
					geomType = GeometryType::MultiLineString;
					//qDebug() << "Reading geometry type: MultiLineString";
					OGRMultiLineString* poMultiLine = static_cast<OGRMultiLineString*>(poGeometry);
					for (int j = 0; j < poMultiLine->getNumGeometries(); j++)
					{
						OGRGeometry* poLineGeometry = poMultiLine->getGeometryRef(j);
						OGRLineString* poLine = static_cast<OGRLineString*>(poLineGeometry);
						std::vector<std::vector<double>> lineCoordinates;
						for (int k = 0; k < poLine->getNumPoints(); k++)
						{
							double x = poLine->getX(k);
							double y = poLine->getY(k);
							lineCoordinates.push_back({ x, y });
						}
						multiPolygonCoordinates.push_back(lineCoordinates);
					}
				}

				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon)
				{
					geomType = GeometryType::MultiPolygon;
					OGRMultiPolygon* poMultiPolygon = static_cast<OGRMultiPolygon*>(poGeometry);
					//qDebug() << "Reading MultiPolygon with" << poMultiPolygon->getNumGeometries() << "sub-polygons.";

					for (int k = 0; k < poMultiPolygon->getNumGeometries(); k++)
					{
						OGRGeometry* poSubGeometry = poMultiPolygon->getGeometryRef(k);
						if (poSubGeometry != nullptr && wkbFlatten(poSubGeometry->getGeometryType()) == wkbPolygon)
						{
							OGRPolygon* poPolygon = static_cast<OGRPolygon*>(poSubGeometry);
							OGRLinearRing* poRing = poPolygon->getExteriorRing();

							std::vector<std::vector<double>> subPolygonCoordinates;
							//qDebug() << "Sub-polygon" << k << "has" << poRing->getNumPoints() << "points:";

							for (int j = 0; j < poRing->getNumPoints(); j++)
							{
								double x = poRing->getX(j);
								double y = poRing->getY(j);
								subPolygonCoordinates.push_back({ x, y });
								//qDebug() << "Point" << j << ":" << x << y;
							}

							multiPolygonCoordinates.push_back(subPolygonCoordinates);
						}
					}
				}

				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbGeometryCollection)
				{
					geomType = GeometryType::GeometryCollection;
					OGRGeometryCollection* poGeomCollection = static_cast<OGRGeometryCollection*>(poGeometry);
					// 遍历几何集合中的所有几何体
					for (int j = 0; j < poGeomCollection->getNumGeometries(); j++)
					{
						OGRGeometry* poSubGeometry = poGeomCollection->getGeometryRef(j);
						// 递归调用相同的逻辑来处理集合中的几何体
						// 可以将上面的逻辑提取为函数，然后在此调用
					}
				}

				// 打印几何类型
				//qDebug() << "Original Geometry Type:" << geometryName;
				//qDebug() << "Internal Geometry Type:" << static_cast<int>(geomType);
			}


			GeometryData geomData;
			if (geomType == GeometryType::MultiPolygon || geomType == GeometryType::MultiLineString) {
				geomData = GeometryData(geomType, multiPolygonCoordinates);
			}
			else {
				geomData = GeometryData(geomType, coordinates);
			}

			// 读取属性数据
			AttributeData attrData;
			for (int j = 0; j < poFeature->GetFieldCount(); j++)
			{
				OGRFieldDefn* poFieldDefn = poFeature->GetFieldDefnRef(j);
				const char* fieldName = poFieldDefn->GetNameRef();
				if (poFeature->IsFieldSet(j))
				{
					if (poFieldDefn->GetType() == OFTInteger)
					{
						int intValue = poFeature->GetFieldAsInteger(j);
						attrData.addAttribute(QString::fromUtf8(fieldName), intValue);
					}
					else if (poFieldDefn->GetType() == OFTReal)
					{
						double doubleValue = poFeature->GetFieldAsDouble(j);
						attrData.addAttribute(QString::fromUtf8(fieldName), doubleValue);
					}
					else if (poFieldDefn->GetType() == OFTString)
					{
						QString stringValue = QString::fromUtf8(poFeature->GetFieldAsString(j));
						attrData.addAttribute(QString::fromUtf8(fieldName), stringValue);
					}
				}
			}

			// 创建 FeatureData 对象
			FeatureData featureData(featureId, geomData, attrData);
			layerData.addFeature(featureData);

			// 释放当前要素
			OGRFeature::DestroyFeature(poFeature);
		}

		// 将图层数据存储到 mvVectorLayer 中
		mvVectorLayer[baseName] = layerData;
		// 存储文件路径和图层名称到 ProjectManager
		mpProjectManager->addLayer(filePath, baseName);
	}

	GDALClose(poDS);

	// 更新 LayertreeWidget 和视图
	// 遍历 LayertreeWidget 中的项，并移除所有矢量图层的项
	for (int i = 0; i < mpLayerTreeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem* item = mpLayerTreeWidget->topLevelItem(i);
		if (item->text(0).startsWith("[Vector] ")) {
			delete mpLayerTreeWidget->takeTopLevelItem(i);
			--i;  // 调整索引以反映删除的项
		}
	}

	QRectF totalBoundingRect;  // 用于存储所有导入图层的总边界

	int baseZValue = 0;  // 初始化基础 Z 值

	// 遍历读取的矢量图层数据
	for (const auto& layerPair : mvVectorLayer)
	{
		const QString& layerName = layerPair.first;
		const VectorLayerData& layerData = layerPair.second;

		// 为图层生成颜色，如果该图层已存在颜色，则使用原有颜色
		QColor layerColor;
		if (mvLayerColors.find(layerName) == mvLayerColors.end())
		{
			do {
				// 如果图层没有颜色，生成一个新颜色
				layerColor = QColor::fromRgb(QRandomGenerator::global()->generate());
			} while (layerColor == Qt::yellow);  // 检查颜色是否与 Qt::yellow 冲突

			// 将颜色存储在 mvLayerColors 中
			mvLayerColors[layerName] = layerColor;
		}
		else
		{
			// 如果图层已有颜色，复用原有颜色
			layerColor = mvLayerColors[layerName];
		}

		// 为图层中的每个要素设置唯一的Z轴值
		int featureIndex = 0;
		for (const auto& feature : layerData.getFeatures())
		{
			int id = feature.getId();
			// 计算每个要素的唯一Z轴值
			int zIndex = baseZValue + featureIndex;

			// 传递 id 和图层名称到 GeometryItem 构造函数
			GeometryItem* item = new GeometryItem(id, feature.getGeometry(), layerColor, zIndex, layerName, this, nullptr);
			mpGraphicsScene->addItem(item);

			totalBoundingRect = totalBoundingRect.united(item->boundingRect());
			featureIndex++;  // 增加索引以确保Z轴值唯一
		}

		for (const auto& layerPair : mvVectorLayer) {
			const QString& layerName = layerPair.first;
			addLayerToTreeWidget(layerName, false); // 矢量图层
		}

		baseZValue += 100;  // 每个图层分配一个新的基础 Z 值范围
	}

	// 调整视图以适应总边界
	if (!totalBoundingRect.isNull())
	{
		mpGraphicsView->fitInView(totalBoundingRect, Qt::KeepAspectRatio);
	}
	else
	{
		mpGraphicsView->fitInView(mpGraphicsScene->sceneRect(), Qt::KeepAspectRatio);
	}

	mpGraphicsView->update(); // 更新视图
}


//-------------------------------------一些额外的功能
//
// 将 variant 类型转换为 QString 以便输出调试信息
QString variantToString(const std::variant<int, double, QString>& value) {
	if (std::holds_alternative<int>(value)) {
		return QString::number(std::get<int>(value));
	}
	else if (std::holds_alternative<double>(value)) {
		return QString::number(std::get<double>(value));
	}
	else if (std::holds_alternative<QString>(value)) {
		return std::get<QString>(value);
	}
	return {};
}

//根据图层名称查找对应的 QTreeWidgetItem
QTreeWidgetItem* MYGIS::findLayerItemByName(const QString& layerName)
{
	// 遍历 LayertreeWidget 的所有顶级项
	for (int i = 0; i < ui.LayertreeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem* item = ui.LayertreeWidget->topLevelItem(i);
		QString itemText = item->text(0);

		// 去除 [Vector] 或 [Raster] 前缀
		itemText.remove("[Vector] ").remove("[Raster] ");

		if (itemText == layerName) {
			return item; // 如果找到与 layerName 匹配的项，返回该项
		}
	}
	return nullptr; // 如果未找到，返回空指针
}

//更新属性表
void MYGIS::UpdateAttributeTables()
{
	// 遍历所有的 QDockWidget 子对象，找到与属性表相关的窗口
	foreach(QDockWidget * dockWidget, this->findChildren<QDockWidget*>())
	{
		if (dockWidget->windowTitle().contains("属性表"))
		{
			QTableWidget* tableWidget = qobject_cast<QTableWidget*>(dockWidget->widget());
			if (tableWidget)
			{
				// 获取图层名称
				QString layerName = dockWidget->windowTitle().replace(" 属性表", "");

				// 检查图层是否仍然存在
				if (mvVectorLayer.find(layerName) == mvVectorLayer.end())
				{
					// 如果图层已经不存在，关闭属性表窗口
					dockWidget->close();
					continue;
				}

				// 获取图层数据
				const VectorLayerData& layerData = mvVectorLayer[layerName];
				const std::vector<FeatureData>& features = layerData.getFeatures();

				// 清除旧的表格内容并准备重新填充
				tableWidget->setRowCount(0);  // 重置行数为0以清除旧内容

				if (!features.empty())
				{
					tableWidget->setRowCount(static_cast<int>(features.size()));
					tableWidget->setColumnCount(features[0].getAttributeKeys().size());
					tableWidget->setHorizontalHeaderLabels(features[0].getAttributeKeys());

					for (int row = 0; row < features.size(); ++row)
					{
						const FeatureData& feature = features[row];
						for (int col = 0; col < feature.getAttributeKeys().size(); ++col)
						{
							QString key = feature.getAttributeKeys()[col];
							std::variant<int, double, QString> value = feature.getAttributes().getAttribute(key);

							QString valueString;
							if (std::holds_alternative<int>(value))
								valueString = QString::number(std::get<int>(value));
							else if (std::holds_alternative<double>(value))
								valueString = QString::number(std::get<double>(value));
							else if (std::holds_alternative<QString>(value))
								valueString = std::get<QString>(value);

							tableWidget->setItem(row, col, new QTableWidgetItem(valueString));
						}
					}
				}
				else
				{
					// 如果图层中没有要素，显示空表格
					tableWidget->setRowCount(0);
					tableWidget->setColumnCount(0);
				}

				// 刷新表格
				tableWidget->viewport()->update();
			}
		}
	}
}

// 辅助函数：将 GeometryData 转换为 OGRGeometry
// 整合后的函数：从 GeometryData 创建 OGRGeometry
OGRGeometry* MYGIS::createOGRGeometryFromGeometryData(const GeometryData& geomData) {
	OGRGeometry* ogrGeometry = nullptr;

	switch (geomData.getGeometryType()) {
	case GeometryType::Point: {
		const auto& coords = std::get<std::vector<std::vector<double>>>(geomData.getCoordinates());
		if (!coords.empty()) {
			ogrGeometry = new OGRPoint(coords[0][0], coords[0][1]);
		}
		break;
	}
	case GeometryType::Line: {
		OGRLineString* lineString = new OGRLineString();
		const auto& coordinates = std::get<std::vector<std::vector<double>>>(geomData.getCoordinates());
		for (const auto& coord : coordinates) {
			lineString->addPoint(coord[0], coord[1]);
		}
		ogrGeometry = lineString;
		break;
	}
	case GeometryType::Polygon: {
		OGRPolygon* polygon = new OGRPolygon();
		OGRLinearRing* ring = new OGRLinearRing();
		const auto& coordinates = std::get<std::vector<std::vector<double>>>(geomData.getCoordinates());
		for (const auto& coord : coordinates) {
			ring->addPoint(coord[0], coord[1]);
		}
		polygon->addRing(ring);
		ogrGeometry = polygon;
		break;
	}
	case GeometryType::MultiPoint: {
		const auto& coords = std::get<std::vector<std::vector<double>>>(geomData.getCoordinates());
		OGRMultiPoint* poMultiPoint = new OGRMultiPoint();
		for (const auto& coord : coords) {
			OGRPoint* poPoint = new OGRPoint(coord[0], coord[1]);
			poMultiPoint->addGeometry(poPoint);
		}
		ogrGeometry = poMultiPoint;
		break;
	}
	case GeometryType::MultiLineString: {
		OGRMultiLineString* multiLineString = new OGRMultiLineString();
		const auto& lineStrings = std::get<std::vector<std::vector<std::vector<double>>>>(geomData.getCoordinates());
		for (const auto& lineCoords : lineStrings) {
			OGRLineString* lineString = new OGRLineString();
			for (const auto& coord : lineCoords) {
				lineString->addPoint(coord[0], coord[1]);
			}
			multiLineString->addGeometry(lineString);
		}
		ogrGeometry = multiLineString;
		break;
	}
	case GeometryType::MultiPolygon: {
		OGRMultiPolygon* multiPolygon = new OGRMultiPolygon();
		const auto& polygons = std::get<std::vector<std::vector<std::vector<double>>>>(geomData.getCoordinates());
		for (const auto& polygonCoords : polygons) {
			OGRPolygon* polygon = new OGRPolygon();
			OGRLinearRing* ring = new OGRLinearRing();
			for (const auto& coord : polygonCoords) {
				ring->addPoint(coord[0], coord[1]);
			}
			polygon->addRing(ring);
			multiPolygon->addGeometry(polygon);
		}
		ogrGeometry = multiPolygon;
		break;
	}
	default:
		break;
	}

	return ogrGeometry;
}


//**********************************工程IO//**********************************
// 获取当前视图中的图层颜色和 Z 轴值
void MYGIS::storeLayerPropertiesFromView() {
	for (QGraphicsItem* graphicsItem : mpGraphicsScene->items()) {
		// 首先处理矢量图层
		GeometryItem* geomItem = dynamic_cast<GeometryItem*>(graphicsItem);
		if (geomItem) {
			QString layerName = geomItem->getLayerName();
			QColor layerColor = geomItem->getColor();
			int zIndex = static_cast<int>(geomItem->zValue());

			// 将这些值存储到 mpProjectManager 中
			if (mpProjectManager) {
				mpProjectManager->updateLayerInfo(layerName, zIndex, layerColor);
			}
		}

		// 然后处理栅格图层
		RasterItem* rasterItem = dynamic_cast<RasterItem*>(graphicsItem);
		if (rasterItem) {
			QString layerName = rasterItem->getLayerName();
			int zIndex = static_cast<int>(rasterItem->zValue());
			// 栅格图层的颜色属性不重要
			// 在这里我们只存储路径和 Z 轴值
			if (mpProjectManager) {
				mpProjectManager->updateLayerInfo(layerName, zIndex);
			}
		}
	}
}

//保存工程
void MYGIS::saveProject()
{
	// 调用 storeLayerPropertiesFromView 函数，保存当前视图中图层的属性信息
	storeLayerPropertiesFromView();

	// 获取 mpProjectManager 中的所有图层路径及其对应的 zIndex 和颜色
	if (mpProjectManager)
	{
		const std::map<QString, ProjectManager::LayerInfo>& layers = mpProjectManager->getLayers();
		const std::map<QString, QString>& paths = mpProjectManager->getAllPaths();

		QString fileName = QFileDialog::getSaveFileName(this, tr("保存项目"), "", tr("文本文件 (*.txt)"));

		if (!fileName.isEmpty())
		{
			QFile file(fileName);
			if (file.open(QIODevice::WriteOnly | QIODevice::Text))
			{
				QTextStream out(&file);

				for (const auto& pathEntry : paths)
				{
					QString layerName = pathEntry.first;
					QString filePath = pathEntry.second;

					if (layers.find(layerName) != layers.end())
					{
						int zIndex = layers.at(layerName).zIndex;

						// 对于栅格图层，假设不保存颜色信息，只保存路径和 zIndex
						if (layerName.contains("tif")) // 假设均衡化图层的名称有特定后缀
						{
							// 仅保存路径和 Z 轴值
							out << filePath << "," << zIndex << "\n";
						}
						else
						{
							QColor layerColor = layers.at(layerName).layerColor;

							// 将路径、zIndex 和颜色信息用逗号分隔写入文件
							out << filePath << "," << zIndex << ","
								<< layerColor.red() << ","
								<< layerColor.green() << ","
								<< layerColor.blue() << "\n";
						}
					}
				}

				file.close();
				QMessageBox::information(this, tr("保存成功"), tr("项目文件已保存."));

				// 日志记录成功保存项目
				logger.info("Project saved successfully to %s", fileName.toStdString().c_str());
			}
			else
			{
				QMessageBox::warning(this, tr("保存失败"), tr("无法保存项目文件."));

				// 日志记录保存失败
				logger.error("Failed to save project to %s", fileName.toStdString().c_str());
			}
		}
	}
}

//打开工程
// 打开工程
void MYGIS::openProject()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"), "", tr("文本文件 (*.txt)"));
	if (fileName.isEmpty())
		return;

	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, tr("打开失败"), tr("无法打开项目文件."));
		return;
	}

	QTextStream in(&file);
	while (!in.atEnd())
	{
		QString line = in.readLine();
		QStringList parts = line.split(',');

		if (parts.size() == 5) // 矢量图层
		{
			QString filePath = parts[0];
			int zIndex = parts[1].toInt();
			int red = parts[2].toInt();
			int green = parts[3].toInt();
			int blue = parts[4].toInt();
			QColor layerColor(red, green, blue);

			qDebug() << "Reading vector layer from project file:" << filePath;
			qDebug() << "Z-index:" << zIndex << "Color:" << layerColor;

			loadVectorLayerFromProperties(filePath, zIndex, layerColor);//绘制矢量
		}
		else if (parts.size() == 2) // 栅格图层
		{
			QString filePath = parts[0];
			int zIndex = parts[1].toInt();

			qDebug() << "Reading raster layer from project file:" << filePath;
			qDebug() << "Z-index:" << zIndex;

			loadRasterLayerFromProperties(filePath, zIndex);//绘制栅格
		}
	}

	file.close();
}

//读取工程里面的路径绘制矢量图形
void MYGIS::loadVectorLayerFromProperties(const QString& filePath, int zIndex, const QColor& layerColor)
{
	qDebug() << "Loading vector layer from file:" << filePath;

	// 打开矢量文件
	GDALDataset* poDS = static_cast<GDALDataset*>(GDALOpenEx(filePath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL));
	if (poDS == nullptr)
	{
		qDebug() << "Failed to open file:" << filePath;
		QMessageBox::critical(this, tr("Error"), tr("Failed to open file."));
		return;
	}

	// 获取文件名（不带路径和扩展名）
	QString baseName = QFileInfo(filePath).baseName();
	QString extension = QFileInfo(filePath).suffix().toLower();

	// 遍历所有图层并读取数据
	for (int i = 0; i < poDS->GetLayerCount(); i++)
	{
		OGRLayer* poLayer = poDS->GetLayer(i);
		if (poLayer == nullptr)
		{
			log4cpp::Category& logger = log4cpp::Category::getRoot();
			logger.warn("Warning: Layer %d is null!", i);
			continue;
		}

		// 创建一个新的VectorLayerData对象来存储当前图层的数据
		VectorLayerData layerData;

		// 读取要素
		OGRFeature* poFeature;
		poLayer->ResetReading();
		while ((poFeature = poLayer->GetNextFeature()) != nullptr)
		{
			// 获取要素的 FID 作为 ID
			int featureId = poFeature->GetFID();

			// 读取几何数据
			OGRGeometry* poGeometry = poFeature->GetGeometryRef();
			GeometryType geomType = GeometryType::Point;  // 初始化几何类型
			std::vector<std::vector<double>> coordinates;
			std::vector<std::vector<std::vector<double>>> multiPolygonCoordinates;

			if (poGeometry != nullptr)
			{
				if (wkbFlatten(poGeometry->getGeometryType()) == wkbPoint)
				{
					geomType = GeometryType::Point;
					OGRPoint* poPoint = static_cast<OGRPoint*>(poGeometry);
					coordinates.push_back({ poPoint->getX(), poPoint->getY() });
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbLineString)
				{
					geomType = GeometryType::Line;
					OGRLineString* poLine = static_cast<OGRLineString*>(poGeometry);
					for (int j = 0; j < poLine->getNumPoints(); j++)
					{
						double x = poLine->getX(j);
						double y = poLine->getY(j);
						coordinates.push_back({ x, y });
					}
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon)
				{
					geomType = GeometryType::Polygon;
					OGRPolygon* poPolygon = static_cast<OGRPolygon*>(poGeometry);
					OGRLinearRing* poRing = poPolygon->getExteriorRing();
					for (int j = 0; j < poRing->getNumPoints(); j++)
					{
						double x = poRing->getX(j);
						double y = poRing->getY(j);
						coordinates.push_back({ x, y });
					}
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPoint)
				{
					geomType = GeometryType::MultiPoint;
					OGRMultiPoint* poMultiPoint = static_cast<OGRMultiPoint*>(poGeometry);
					for (int j = 0; j < poMultiPoint->getNumGeometries(); j++)
					{
						OGRGeometry* poPointGeometry = poMultiPoint->getGeometryRef(j);
						OGRPoint* poPoint = static_cast<OGRPoint*>(poPointGeometry);
						coordinates.push_back({ poPoint->getX(), poPoint->getY() });
					}
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbMultiLineString)
				{
					geomType = GeometryType::MultiLineString;
					OGRMultiLineString* poMultiLine = static_cast<OGRMultiLineString*>(poGeometry);
					for (int j = 0; j < poMultiLine->getNumGeometries(); j++)
					{
						OGRGeometry* poLineGeometry = poMultiLine->getGeometryRef(j);
						OGRLineString* poLine = static_cast<OGRLineString*>(poLineGeometry);
						std::vector<std::vector<double>> lineCoordinates;
						for (int k = 0; k < poLine->getNumPoints(); k++)
						{
							double x = poLine->getX(k);
							double y = poLine->getY(k);
							lineCoordinates.push_back({ x, y });
						}
						multiPolygonCoordinates.push_back(lineCoordinates);
					}
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon)
				{
					geomType = GeometryType::MultiPolygon;
					OGRMultiPolygon* poMultiPolygon = static_cast<OGRMultiPolygon*>(poGeometry);
					for (int k = 0; k < poMultiPolygon->getNumGeometries(); k++)
					{
						OGRGeometry* poSubGeometry = poMultiPolygon->getGeometryRef(k);
						if (poSubGeometry != nullptr && wkbFlatten(poSubGeometry->getGeometryType()) == wkbPolygon)
						{
							OGRPolygon* poPolygon = static_cast<OGRPolygon*>(poSubGeometry);
							OGRLinearRing* poRing = poPolygon->getExteriorRing();

							std::vector<std::vector<double>> subPolygonCoordinates;
							for (int j = 0; j < poRing->getNumPoints(); j++)
							{
								double x = poRing->getX(j);
								double y = poRing->getY(j);
								subPolygonCoordinates.push_back({ x, y });
							}

							multiPolygonCoordinates.push_back(subPolygonCoordinates);
						}
					}
				}
			}

			GeometryData geomData;
			if (geomType == GeometryType::MultiPolygon || geomType == GeometryType::MultiLineString) {
				geomData = GeometryData(geomType, multiPolygonCoordinates);
			}
			else {
				geomData = GeometryData(geomType, coordinates);
			}

			// 读取属性数据
			AttributeData attrData;
			for (int j = 0; j < poFeature->GetFieldCount(); j++)
			{
				OGRFieldDefn* poFieldDefn = poFeature->GetFieldDefnRef(j);
				const char* fieldName = poFieldDefn->GetNameRef();
				if (poFeature->IsFieldSet(j))
				{
					if (poFieldDefn->GetType() == OFTInteger)
					{
						int intValue = poFeature->GetFieldAsInteger(j);
						attrData.addAttribute(QString::fromUtf8(fieldName), intValue);
					}
					else if (poFieldDefn->GetType() == OFTReal)
					{
						double doubleValue = poFeature->GetFieldAsDouble(j);
						attrData.addAttribute(QString::fromUtf8(fieldName), doubleValue);
					}
					else if (poFieldDefn->GetType() == OFTString)
					{
						QString stringValue = QString::fromUtf8(poFeature->GetFieldAsString(j));
						attrData.addAttribute(QString::fromUtf8(fieldName), stringValue);
					}
				}
			}

			FeatureData featureData(featureId, geomData, attrData);
			layerData.addFeature(featureData);

			OGRFeature::DestroyFeature(poFeature);
		}

		// 判断文件类型并设置图层名称
			QString layerName;
			if (extension == "csv") {
				layerName = baseName + "_WKTLayer";
			}
			else if (extension == "geojson") {
				layerName = baseName + "_geojsonLayer";
			}
			else {
				const char* rawLayerName = poLayer->GetName();
				if (rawLayerName && strlen(rawLayerName) > 0) {
					layerName = QString::fromUtf8(rawLayerName);
				}
				else {
					layerName = baseName; // 默认使用基础名称
				}
			}

		// 将图层数据存储到相应的容器中
		mvVectorLayer[layerName] = layerData;
		mvLayerColors[layerName] = layerColor;

		// 存储文件路径和图层名称到 ProjectManager
		mpProjectManager->addLayer(filePath, layerName);

		qDebug() << "Adding layer:" << layerName << "with zIndex:" << zIndex << "and color:" << layerColor;

		// 根据读取的zIndex和layerColor来设置图层属性
		for (auto& feature : mvVectorLayer[layerName].getFeatures())
		{
			GeometryItem* item = new GeometryItem(feature.getId(), feature.getGeometry(), layerColor, zIndex, layerName, this, nullptr);
			mpGraphicsScene->addItem(item);
			//qDebug() << "Added feature with ID:" << feature.getId();
		}

		for (const auto& layerPair : mvVectorLayer) {
			const QString& layerName = layerPair.first;
			addLayerToTreeWidget(layerName, false); // 矢量图层
		}
		qDebug() << "Layer" << layerName << "added to tree widget.";
	}

	GDALClose(poDS);
	mpGraphicsView->update(); // 更新视图
	qDebug() << "Finished loading layer from properties.";
}

//读取工程里面的路径绘制栅格图形
void MYGIS::loadRasterLayerFromProperties(const QString& filePath, int zIndex) {
	// 获取图层名
	QString layerName = QFileInfo(filePath).fileName();

	// 创建 GDALRasterRead 对象并加载栅格数据
	auto rasterLayer = std::make_shared<GDALRasterRead>();
	bool success = rasterLayer->loadFromGDAL(filePath.toStdString().c_str(), layerName);

	if (success) {
		qDebug() << "loadRasterLayerFromProperties: Raster layer" << layerName << "loaded successfully.";

		// 将加载的栅格图层保存到管理结构中
		mvRasterLayers[layerName] = rasterLayer;
		addLayerToTreeWidget(layerName, true); // 仅调用一次，避免重复

		RasterData* rasterData = rasterLayer->getRasterData()[layerName];
		if (rasterData && rasterData->getBandnum() >= 1) { // 现在支持单波段和多波段
			QImage image = createImageFromRasterData(rasterData);
			QPixmap pixmap = QPixmap::fromImage(image);

			double geoTransform[6];
			GDALDataset* dataset = rasterLayer->getGDALDataset();
			if (dataset->GetGeoTransform(geoTransform) == CE_None) {
				QPointF topLeft = mapRasterToViewCoordinates(mpGraphicsScene, geoTransform[0], geoTransform[3]);
				QPointF bottomRight = mapRasterToViewCoordinates(mpGraphicsScene, geoTransform[0] + geoTransform[1] * rasterData->getCols(), geoTransform[3] + geoTransform[5] * rasterData->getRows());
				QRectF imageRect(topLeft, bottomRight);

				// 创建 RasterItem 并添加到场景中
				RasterItem* item = new RasterItem(pixmap, layerName);
				item->setPos(imageRect.topLeft());
				item->setScale(imageRect.width() / pixmap.width());
				item->setZValue(zIndex); // 设置 Z 轴值
				mpGraphicsScene->addItem(item);

				// 在 std::map 中存储 RasterItem 指针集合以便管理
				mvRasterItems[layerName].push_back(item);

				mpProjectManager->addLayer(filePath, layerName);//存储导入的栅格

				// 适应视图范围
				mpGraphicsView->fitInView(imageRect, Qt::KeepAspectRatio);
				
				qDebug() << "loadRasterLayerFromProperties: Raster image displayed successfully.";
			}
		}
		else {
			qDebug() << "loadRasterLayerFromProperties: Raster does not have enough bands for RGB or grayscale.";
		}
	}
	else {
		qDebug() << "loadRasterLayerFromProperties: Failed to load raster layer" << layerName;
	}
}



//**********************************栅格扩展//**********************************
//关于栅格Y轴翻转
QPointF MYGIS::mapRasterToViewCoordinates(QGraphicsScene* scene, double x, double y) {
	// 获取场景的边界矩形
	QRectF sceneRect = scene->sceneRect();
	// 将地理坐标转换为视图坐标，Y 轴需要进行翻转
	double mappedX = x;
	double mappedY = sceneRect.bottom() - (y - sceneRect.top());
	return QPointF(mappedX, mappedY);
}

//添加分析后的栅格进行管理
void MYGIS::addRasterToManage(const QPixmap& pixmap, const QString& layerName, const double* geoTransform, int rows, int cols) {
	// 确保图层名称是唯一的
	if (mvRasterItems.find(layerName) != mvRasterItems.end()) {
		qDebug() << "addRasterToManage: Layer name already exists. Cannot add the same layer twice.";
		return;
	}

	// 计算图像的地理边界框
	QPointF topLeft = mapRasterToViewCoordinates(mpGraphicsScene, geoTransform[0], geoTransform[3]);
	QPointF bottomRight = mapRasterToViewCoordinates(mpGraphicsScene, geoTransform[0] + geoTransform[1] * cols, geoTransform[3] + geoTransform[5] * rows);
	QRectF imageRect(topLeft, bottomRight);

	// 创建 RasterItem 并添加到场景中
	RasterItem* item = new RasterItem(pixmap, layerName);
	item->setPos(imageRect.topLeft());
	item->setScale(imageRect.width() / pixmap.width());
	mpGraphicsScene->addItem(item);

	// 在 std::map 中存储 RasterItem 指针集合以便管理
	mvRasterItems[layerName].push_back(item);

	// 适应视图范围
	mpGraphicsView->fitInView(imageRect, Qt::KeepAspectRatio);

	// 将图层添加到图层管理结构中
	addLayerToTreeWidget(layerName, true);

	qDebug() << "addRasterToManage: Raster layer" << layerName << "added successfully.";
}
