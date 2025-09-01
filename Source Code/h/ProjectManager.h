#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QString>
#include <vector>
#include <map>
#include <QColor>

class ProjectManager {
public:
	// ͼ����Ϣ�ṹ�壬���ڴ洢ÿ��ͼ���Z��˳�����ɫ��Ϣ
	struct LayerInfo {
		int zIndex;       // Z��˳��
		QColor layerColor; // ͼ����ɫ
	};

	// ���ͼ��·�������Ƶ�������
	void addLayer(const QString& filePath, const QString& layerName) {
		mvPaths[layerName] = filePath;
		mvLayerOrder.push_back(layerName);
	}

	// ���ͼ�㼰�� ZIndex ����ɫ��Ϣ��������
	void addLayer(const QString& filePath, const QString& layerName, int zIndex, const QColor& layerColor) {
		mvPaths[layerName] = filePath;
		mvLayers[layerName] = { zIndex, layerColor };
	}

	// �ӹ��������Ƴ�ͼ��
	void removeLayer(const QString& layerName) {
		mvPaths.erase(layerName);
		mvLayers.erase(layerName);
	}

	// ����ͼ��� ZIndex ֵ
	void updateLayerZIndex(const QString& layerName, int newZIndex) {
		if (mvLayers.find(layerName) != mvLayers.end()) {
			mvLayers[layerName].zIndex = newZIndex;
		}
	}

	// ����ͼ�����ɫ��Ϣ
	void updateLayerColor(const QString& layerName, const QColor& newColor) {
		if (mvLayers.find(layerName) != mvLayers.end()) {
			mvLayers[layerName].layerColor = newColor;
		}
	}

	// ͼ��������л�ȡz���ͼ����ɫ
	void updateLayerInfo(const QString& layerName, int zIndex, const QColor& layerColor) {
		if (mvLayers.find(layerName) != mvLayers.end()) {
			mvLayers[layerName].zIndex = zIndex;
			mvLayers[layerName].layerColor = layerColor;
		}
		else {
			// ���ͼ����Ϣ�������ڣ�������µ�ͼ����Ϣ
			mvLayers[layerName] = { zIndex, layerColor };
		}
	}

	// ����ͼ����Ϣ�������� ZIndex��
	void updateLayerInfo(const QString& layerName, int zIndex) {
		if (mvLayers.find(layerName) != mvLayers.end()) {
			mvLayers[layerName].zIndex = zIndex;
		}
		else {
			// ���ͼ����Ϣ�������ڣ����Ĭ����ɫ����ͼ����Ϣ
			mvLayers[layerName] = { zIndex, QColor() }; // ʹ��Ĭ����ɫ
		}
	}

	// ��ȡ����ͼ��� ZIndex ����ɫ��Ϣ
	const std::map<QString, LayerInfo>& getLayers() const {
		return mvLayers;
	}

	// ��ȡָ��ͼ���·��
	const QString& getLayerPath(const QString& layerName) const {
		return mvPaths.at(layerName);
	}

	// ��ȡ����ͼ���·����Ϣ
	const std::map<QString, QString>& getAllPaths() const {
		return mvPaths;
	}

	// ��ȡͼ�����ʾ˳��
	const std::vector<QString>& getLayerOrder() const {
		return mvLayerOrder;
	}

	// ���ͼ�㵽��ʾ˳��
	void addLayerToOrder(const QString& layerName) {
		mvLayerOrder.push_back(layerName);
	}

	// ����ʾ˳�����Ƴ�ͼ��
	void removeLayerFromOrder(const QString& layerName) {
		mvLayerOrder.erase(std::remove(mvLayerOrder.begin(), mvLayerOrder.end(), layerName), mvLayerOrder.end());
	}

private:
	std::map<QString, QString> mvPaths;// �洢ͼ��·��
	std::map<QString, LayerInfo> mvLayers; // �洢ͼ��� zIndex ����ɫ��Ϣ
	std::vector<QString> mvLayerOrder;// �洢ͼ�����ʾ˳��
};

#endif
