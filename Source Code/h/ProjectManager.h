#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QString>
#include <vector>
#include <map>
#include <QColor>

class ProjectManager {
public:
	// 图层信息结构体，用于存储每个图层的Z轴顺序和颜色信息
	struct LayerInfo {
		int zIndex;       // Z轴顺序
		QColor layerColor; // 图层颜色
	};

	// 添加图层路径和名称到管理器
	void addLayer(const QString& filePath, const QString& layerName) {
		mvPaths[layerName] = filePath;
		mvLayerOrder.push_back(layerName);
	}

	// 添加图层及其 ZIndex 和颜色信息到管理器
	void addLayer(const QString& filePath, const QString& layerName, int zIndex, const QColor& layerColor) {
		mvPaths[layerName] = filePath;
		mvLayers[layerName] = { zIndex, layerColor };
	}

	// 从管理器中移除图层
	void removeLayer(const QString& layerName) {
		mvPaths.erase(layerName);
		mvLayers.erase(layerName);
	}

	// 更新图层的 ZIndex 值
	void updateLayerZIndex(const QString& layerName, int newZIndex) {
		if (mvLayers.find(layerName) != mvLayers.end()) {
			mvLayers[layerName].zIndex = newZIndex;
		}
	}

	// 更新图层的颜色信息
	void updateLayerColor(const QString& layerName, const QColor& newColor) {
		if (mvLayers.find(layerName) != mvLayers.end()) {
			mvLayers[layerName].layerColor = newColor;
		}
	}

	// 图层管理器中获取z轴和图层颜色
	void updateLayerInfo(const QString& layerName, int zIndex, const QColor& layerColor) {
		if (mvLayers.find(layerName) != mvLayers.end()) {
			mvLayers[layerName].zIndex = zIndex;
			mvLayers[layerName].layerColor = layerColor;
		}
		else {
			// 如果图层信息还不存在，则添加新的图层信息
			mvLayers[layerName] = { zIndex, layerColor };
		}
	}

	// 更新图层信息（仅更新 ZIndex）
	void updateLayerInfo(const QString& layerName, int zIndex) {
		if (mvLayers.find(layerName) != mvLayers.end()) {
			mvLayers[layerName].zIndex = zIndex;
		}
		else {
			// 如果图层信息还不存在，添加默认颜色的新图层信息
			mvLayers[layerName] = { zIndex, QColor() }; // 使用默认颜色
		}
	}

	// 获取所有图层的 ZIndex 和颜色信息
	const std::map<QString, LayerInfo>& getLayers() const {
		return mvLayers;
	}

	// 获取指定图层的路径
	const QString& getLayerPath(const QString& layerName) const {
		return mvPaths.at(layerName);
	}

	// 获取所有图层的路径信息
	const std::map<QString, QString>& getAllPaths() const {
		return mvPaths;
	}

	// 获取图层的显示顺序
	const std::vector<QString>& getLayerOrder() const {
		return mvLayerOrder;
	}

	// 添加图层到显示顺序
	void addLayerToOrder(const QString& layerName) {
		mvLayerOrder.push_back(layerName);
	}

	// 从显示顺序中移除图层
	void removeLayerFromOrder(const QString& layerName) {
		mvLayerOrder.erase(std::remove(mvLayerOrder.begin(), mvLayerOrder.end(), layerName), mvLayerOrder.end());
	}

private:
	std::map<QString, QString> mvPaths;// 存储图层路径
	std::map<QString, LayerInfo> mvLayers; // 存储图层的 zIndex 和颜色信息
	std::vector<QString> mvLayerOrder;// 存储图层的显示顺序
};

#endif
