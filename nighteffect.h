#ifndef NIGHTEFFECT_H
#define NIGHTEFFECT_H

#include <QtWidgets/QMainWindow>
#include "ui_nighteffect.h"
#include <QtGui/QWheelEvent>
#include <QProgressBar>
#include<QtWidgets/QListWidget>
#include <qmap.h>
#include "Worker.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QThread>



//#include <gfx/Ptr.h>
//#include <RenderControl/ControllerGraphicsWindowWin32.h>

class WorkerThread : public QThread {
	Q_OBJECT
public:
	void run() override {
		// �������д��Ҫ�����߳���ִ�еĴ���
		// �����ʱ�ļ���������������
		emit resultReady(); // �����źű�ʾִ�����
	}
signals:
	void resultReady(); // �Զ����źţ���ʾִ�����
};

class NightEffect : public QMainWindow
{
	Q_OBJECT

public:
	NightEffect(QWidget *parent = 0);
	~NightEffect();


public:
	void initWindow();

	public slots:
	void OnBrowseFileData();
	void OnProcessFile();
	
	//void OnPreView();
	public slots:
	void SetFilePath();

private:
	// QString m_pFileDir;
	QLineEdit* m_pFilePathLineEdit;//�����ַ
	QLineEdit* m_pFilePathLineEdit2;
	QLineEdit* m_pFilePathLineEditOUTPUT;//�ļ�д��
	QLineEdit* m_pFilePathLineEdit3EDPART;//�������ַ
	QString LOD3Path;//LOD3��Ե�ַ��д��
	QString exePath;

	QPushButton* m_btn;

	QProgressBar* pProgressBar;

	QListWidget* listWidget;
	
	bool IsOpen = 0;

	void preLOD3();
	//LOD3���̿�ʼ����
	void LOD3Process();
	void LOD3Process_thread();
	//��log�ļ�
	void openLog();
	//���������������ļ���
	void openOutput();
	//���ýű���osgviewer
	void openOsgviewer(QListWidgetItem *item);

	
private:
	Ui::NightEffectClass ui;


private slots:
	void startWork() {
		Worker *worker = new Worker();
		QThread *workerThread = new QThread();

		worker->moveToThread(workerThread);

		connect(workerThread, &QThread::started, worker, &Worker::doWork);
		connect(worker, &Worker::workFinished, workerThread, &QThread::quit);
		connect(worker, &Worker::workFinished, worker, &QObject::deleteLater);
		connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);






		workerThread->start();
	}
};




namespace grc
{
	class ControllerGraphicsWindowWin32;
	class RenderControl;
}


class ViewerWidget : public QWidget
{
	Q_OBJECT

public:
	ViewerWidget(QWidget *parent = 0);
	~ViewerWidget();

	//ggp::CViewer *getViewer() { return m_pViewer; }

	//  void initViewer();

protected:
	QPaintEngine *paintEngine() const;

protected:
	//     virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result);
	//     virtual void wheelEvent(QWheelEvent *pEvent);
	//     virtual void keyPressEvent(QKeyEvent *);
	//     virtual void keyReleaseEvent(QKeyEvent *);

private:
	void *m_pViewer;

	//gfx::Ptr<grc::ControllerGraphicsWindowWin32> m_pWindow;
	//grc::RenderControl* m_pController;
};

#endif // NIGHTEFFECT_H
