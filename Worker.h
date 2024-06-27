#pragma once
#ifndef WORKER_H
#define WORKER_H
#include <QObject>
#include <QDebug>
#include <QThread>

class Worker : public QObject {
	Q_OBJECT

public slots:
	void doWork() {
		qDebug() << "Working in thread:" << QThread::currentThread();
		// 执行耗时任务
		QThread::sleep(3); // 模拟耗时操作
		qDebug() << "Work done";
		emit workFinished();
	}

signals:
	void workFinished();
};

#endif // WORKER_H