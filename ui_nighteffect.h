/********************************************************************************
** Form generated from reading UI file 'nighteffect.ui'
**
** Created by: Qt User Interface Compiler version 5.9.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NIGHTEFFECT_H
#define UI_NIGHTEFFECT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_NightEffectClass
{
public:
	QMenuBar *menuBar;
	QToolBar *mainToolBar;
	QWidget *centralWidget;
	QStatusBar *statusBar;

	void setupUi(QMainWindow *NightEffectClass)
	{
		if (NightEffectClass->objectName().isEmpty())
			NightEffectClass->setObjectName(QStringLiteral("NightEffectClass"));
		NightEffectClass->resize(600, 400);
		menuBar = new QMenuBar(NightEffectClass);
		menuBar->setObjectName(QStringLiteral("menuBar"));
		NightEffectClass->setMenuBar(menuBar);
		mainToolBar = new QToolBar(NightEffectClass);
		mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
		NightEffectClass->addToolBar(mainToolBar);
		centralWidget = new QWidget(NightEffectClass);
		centralWidget->setObjectName(QStringLiteral("centralWidget"));
		NightEffectClass->setCentralWidget(centralWidget);
		statusBar = new QStatusBar(NightEffectClass);
		statusBar->setObjectName(QStringLiteral("statusBar"));
		NightEffectClass->setStatusBar(statusBar);

		retranslateUi(NightEffectClass);

		QMetaObject::connectSlotsByName(NightEffectClass);
	} // setupUi

	void retranslateUi(QMainWindow *NightEffectClass)
	{
		NightEffectClass->setWindowTitle(QApplication::translate("NightEffectClass", "NightEffect", Q_NULLPTR));
	} // retranslateUi

};

namespace Ui {
	class NightEffectClass : public Ui_NightEffectClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NIGHTEFFECT_H
