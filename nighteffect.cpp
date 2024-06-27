#include "nighteffect.h"

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QFileDialog>
#include <QProcess>
#include <QTime>  
#include <QDebug.h>
#include <QtConcurrent/QtConcurrent>
#include <qmap.h>
#include<sstream>
#include <fstream>
#include<iostream>
#include <QIODevice>

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonValueRef>

using namespace std;
#include <QDesktopServices>
#include<QtWidgets/QListWidget>
#include "QDirIterator"
#include<io.h>
#include <direct.h>
#if _MSC_VER >= 1600
#pragma  execution_character_set("utf-8")
#endif

// 记录文件名字和大小， 判断是否执行成功
QMap<QString, int> m_map;

QJsonObject Main_Json;

QJsonArray arr_json;

// json 文件配置
// Components数组, 查找路径对应的Code, 查找矩阵对应的Code
void SetJsonPath(const QJsonValue& RootObj, const QString& PathCode, const QString& MatrixCode, int action, const QString Name, QJsonObject& inner_Json) {
	//qDebug() << PathCode << "\n";
	//qDebug() << MatrixCode << "\n";
	QJsonValue ConceptValue = RootObj.toObject().value(Name);

	if (ConceptValue.type() == QJsonValue::Array) {
		QJsonArray ConArray = ConceptValue.toArray();

		for (int i = 0; i < ConArray.size(); ++i) {
			QJsonValue ConceptObject = ConArray.at(i);
			QString CheckCode = ConceptObject.toObject().value("Id").toObject().value("Code").toString();
			//qDebug() << "IdCode:  " << CheckCode << "\n";
			switch (action) {
			case(0): { // 处理path路径
				if (!QString::compare(CheckCode, PathCode)) {
					QString Geometry = ConceptObject.toObject().value("Geometry").toString();
					//qDebug() << " Geometry:  " << Geometry << "\n";
					SetJsonPath(RootObj, Geometry, MatrixCode, 1, "ExternalDatas", inner_Json);
					return;
				}
				break;

			}
			case(1): { // 得到路径
				if (!QString::compare(CheckCode, PathCode)) {
					QString OuterUrl = ConceptObject.toObject().value("OuterUrl").toString();
					//qDebug() << "OuterUrl" << OuterUrl << "\n";
					inner_Json.insert("Url", OuterUrl);
					return;
				}
				break;
			}
			case(2): { // 写入矩阵
				if (!QString::compare(CheckCode, MatrixCode)) {
					double PanX = ConceptObject.toObject().value("PanX").toDouble();
					//qDebug() << "PanX" << PanX << "\n";

					
					inner_Json.insert("Panx", ConceptObject.toObject().value("PanX").toDouble());
					inner_Json.insert("panY", ConceptObject.toObject().value("PanY").toDouble());
					inner_Json.insert("panZ", ConceptObject.toObject().value("PanZ").toDouble());
					inner_Json.insert("RotateX", ConceptObject.toObject().value("RotateX").toDouble());
					inner_Json.insert("RotateY", ConceptObject.toObject().value("RotateY").toDouble());
					inner_Json.insert("RotateZ", ConceptObject.toObject().value("RotateZ").toDouble());
					inner_Json.insert("Scale", ConceptObject.toObject().value("Scale").toDouble());
					return;
				}
			}
			}

		}
	}



}

void WriteJsonFile(QJsonObject& inner_Json) {
	

}

// 传入参数： 需要匹配的ConceptCode, 得到的最外层对象(数组),   Code字符串， Comments字符串, 传入1表示读取ID值 0 表示得到Components
void Get_IdCodeByConcept(QJsonValue& ConceptValue, QJsonObject& rootObj, QString &Code, QString &Comments, bool flag, QString ConceptCode = "") {


	if (ConceptValue.type() == QJsonValue::Array) {
		QJsonArray ConArray = ConceptValue.toArray();
		

		

		for (int i = 0; i < ConArray.size(); ++i) {
			QJsonValue ConceptObject = ConArray.at(i);
			
			if (ConceptObject.type() == QJsonValue::Object) {
				QJsonObject obj = ConceptObject.toObject();
				QString CheckCode = obj.value("ConceptCode").toString();
				
				if (flag) { // 没有传入ID则说明需要得到id
					if (!QString::compare(ConceptCode, CheckCode)) {
						QJsonObject Id = obj.value("Id").toObject();
						Code = Id.value("Code").toString();
						Comments = Id.value("Comments").toString();
						return;
					}
				}
				else {
					QJsonArray arr = obj.value("BelongConcepts").toArray();
					bool IsTarget = false;
					for (int i = 0; i < arr.size(); ++i) {
						QJsonObject blgc = arr.at(i).toObject();
						
						if (!QString::compare(blgc.value("Code").toString(), Code) &&
							!QString::compare(blgc.value("Comments").toString(), Comments)) {
							QJsonObject inner_Json;
							IsTarget = true;
							//qDebug() << "找到目标！" << "\n";

							// 得到Components下的第四个Code 即路径的Code
							QJsonArray PathCodeArr = obj.value("Components").toArray();
							QString PathCode = PathCodeArr.at(3).toObject().value("Code").toString();
							QString MatrixCode = PathCodeArr.at(2).toObject().value("Code").toString();
							

							// 创建json对象

							SetJsonPath(rootObj, PathCode, MatrixCode, 0, "Components", inner_Json);
							SetJsonPath(rootObj, PathCode, MatrixCode, 2, "Components", inner_Json);


							arr_json.append(inner_Json);


						}
					}
				}

			}
			else {
				qDebug() << "传入的数不为对象， 请检查！\n";
			}
		}
	}
	else {
		qDebug() << "传入的数不为数组， 请检查！\n";
	}
	
}

QJsonDocument read_file(QString file_path)
{
	QFile this_file_ptr(file_path);

	// 判断文件是否存在
	if (false == this_file_ptr.exists())
	{
		qDebug() << "文件不存在" << "\n";
	}
	if (false == this_file_ptr.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "打开失败" << "\n";
	}

	// 读取到文本中的字符串
	QTextStream stream(&this_file_ptr);
	stream.setCodec("UTF-8");
	QString string_value = stream.readAll();
	this_file_ptr.close();

	// QJsonParseError类用于在JSON解析期间报告错误。
	QJsonParseError jsonError;
	// 将json解析为UTF-8编码的json文档，并从中创建一个QJsonDocument。
	// 如果解析成功，返回QJsonDocument对象，否则返回null
	QJsonDocument doc = QJsonDocument::fromJson(string_value.toUtf8(), &jsonError);
	// 判断是否解析失败
	if (jsonError.error != QJsonParseError::NoError && !doc.isNull()) {
		qDebug() << "Json格式错误！" << jsonError.error;
	}
	

	return doc;

}

void ActionFile(QJsonDocument& docFile, QJsonDocument& docMatrix, QString ConceptCode = "") {
	QJsonObject rootObj = docFile.object();

	QJsonValue ConceptValue = rootObj.value("Concepts");

	QString Code;
	QString Comments;


	Get_IdCodeByConcept(ConceptValue, rootObj, Code, Comments, 1, ConceptCode);

	//qDebug() << "Code:  " << Code << "\n";
	//qDebug() << "Comments:  " << Comments << "\n";
	rootObj = docMatrix.object();

	ConceptValue = rootObj.value("Entities");
	Get_IdCodeByConcept(ConceptValue, rootObj, Code, Comments, 0);
}

void Act(QString path1, QString path2, QString ConceptCode) {
	//QJsonDocument docFile = read_file("D:\\CIM3_6\\NNU-MiniCIM_05-14\\CIMDataSet_05_14\\Schema\\CIMStandard.json");
	//QJsonDocument docMatrix = read_file("D:\\CIM3_6\\NNU-MiniCIM_05-14\\CIMDataSet_05_14\\SyntheticModel\\CIM3\\CIM3.json");

	QJsonDocument docFile = read_file(path1);
	QJsonDocument docMatrix = read_file(path2);
	ActionFile(docFile, docMatrix, ConceptCode);

}


// 遍历文件函数
QStringList getFileListUnderDir(const QString& dirPath, int& ModelNum, int flag) {

	QStringList fileList;
	QDir dir(dirPath);
	QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs);
	foreach(auto fileInfo, fileInfoList) {
		if (fileInfo.isDir()) {
			getFileListUnderDir(fileInfo.absoluteFilePath(), ModelNum, 1);

		}

		if (fileInfo.isFile()) {
			if (fileInfo.suffix() == "osgb") {
				if (flag) {  // 开始统计数量
					ModelNum++;
					m_map[fileInfo.fileName()] = fileInfo.size();
				}
				else {		// 检验通过数量
					if (fileInfo.size()==m_map[fileInfo.fileName()]) {
						ModelNum--;
						
					}
				}
			}

		}
	}
	return fileList;
}

int FechFile(const QString& path, int& ModelNum, bool flag) {
	QStringList fileList;
	QDir dir(path);
	QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs);
	foreach(auto fileInfo, fileInfoList) {
		if (fileInfo.isDir()) {
			//qDebug() << __FUNCTION__ << __LINE__ << " : " << fileInfo.absoluteFilePath() << "\n";
			getFileListUnderDir(fileInfo.absoluteFilePath(), ModelNum, flag);
		}
	}
	return ModelNum;
}

//(源文件目录路劲，目的文件目录，文件存在是否覆盖)
bool copyDirectory(const QString& srcPath, const QString& dstPath, bool coverFileIfExist)
{
	QDir srcDir(srcPath);
	QDir dstDir(dstPath);
	if (!dstDir.exists()) { //目的文件目录不存在则创建文件目录
		if (!dstDir.mkdir(dstDir.absolutePath()))
			return false;
	}
	QFileInfoList fileInfoList = srcDir.entryInfoList();
	foreach(QFileInfo fileInfo, fileInfoList) {
		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;

		if (fileInfo.isDir()) {    // 当为目录时，递归的进行copy
			if (!copyDirectory(fileInfo.filePath(), dstDir.filePath(fileInfo.fileName()), coverFileIfExist))
				return false;
		}
		else {            //当允许覆盖操作时，将旧文件进行删除操作
			if (coverFileIfExist && dstDir.exists(fileInfo.fileName())) {
				dstDir.remove(fileInfo.fileName());
			}
			/// 进行文件copy
			if (!QFile::copy(fileInfo.filePath(), dstDir.filePath(fileInfo.fileName()))) {
				return false;
			}
		}
	}
	return true;
}

//(源文件文件路径，目的文件路径，文件存在是否覆盖)
bool copyFile(QString srcPath, QString dstPath, bool coverFileIfExist)
{
	srcPath.replace("\\", "/");
	dstPath.replace("\\", "/");
	if (srcPath == dstPath) {
		return true;
	}

	if (!QFile::exists(srcPath)) {  //源文件不存在
		return false;
	}

	if (QFile::exists(dstPath)) {
		if (coverFileIfExist) {
			QFile::remove(dstPath);
		}
	}

	if (!QFile::copy(srcPath, dstPath)) {
		return false;
	}
	return true;
}

// 读取文件内容
QString readFile(const QString& filePath) {
	QString content;

	// 创建 QFile 对象并指定文件路径
	QFile file(filePath);

	// 打开文件（以 ReadOnly 模式）
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		// 创建 QTextStream 对象，并将其与文件关联
		QTextStream in(&file);

		// 读取文件的全部内容
		content = in.readAll();

		// 关闭文件
		file.close();
	}
	else {
		// 打开文件失败
		qDebug() << "Failed to open file for reading.";
	}

	return content;
}

void NightEffect::LOD3Process_thread() {
	
}

void NightEffect::preLOD3()
{
	QObject* obj = this;
	QThread *workerThread = new QThread();

	this->moveToThread(workerThread);

	connect(workerThread, &QThread::started, this, &NightEffect::LOD3Process);
	connect(this, &NightEffect::destroyed, workerThread, &QThread::quit);
	connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);
	connect(workerThread, &QThread::finished, this, &QObject::deleteLater);

	workerThread->start();
}

void NightEffect::LOD3Process()
{
	// 处理json 文件
	QString CIMStandard = m_pFilePathLineEdit->text() + "\\Schema\\CIMStandard.json";
	

	QFile BasicInfo(m_pFilePathLineEdit->text() + "\\BasicInfo.CIM");
	BasicInfo.open(QIODevice::ReadOnly | QIODevice::Text);
	QTextStream stream(&BasicInfo);
	QString string_value = stream.readAll();
	QJsonParseError jsonError;
	QJsonDocument doc = QJsonDocument::fromJson(string_value.toUtf8(), &jsonError);
	QJsonObject obj = doc.object();

	QString path_info = obj.value("综合模型路径").toArray().at(0).toObject().value("路径").toString();
	path_info = m_pFilePathLineEdit->text() + "\\SyntheticModel\\CIM3\\CIM3.json";

	QFile CodeFile(".\\ConceptCode.txt");
	if (!CodeFile.exists()) qDebug() << "错误缺少ConceptCode.txt 文件\n";
	CodeFile.open(QIODevice::ReadOnly);
	//按行读取
	while (!CodeFile.atEnd())
	{
		//方式1
		QByteArray array2 = CodeFile.readLine();
		Act(CIMStandard, path_info, array2.left(12));
		//qDebug() << "CIMStandard: " << CIMStandard << "\n";
		//qDebug() << "path_info:  " << path_info << "\n";
		
	}


	Main_Json.insert("Info", arr_json);

	QJsonDocument document;
	document.setObject(Main_Json);
	QByteArray byteArray = document.toJson(QJsonDocument::Compact);
	QString strJson(byteArray);


	QFile file(".\\Path.json");
	//打开文件，不存在则创建
	file.open(QIODevice::ReadWrite | QIODevice::Text);
	//写入文件需要字符串为QByteArray格式
	file.write(strJson.toUtf8());
	file.close();


	//Act(CIMStandard, path_info, "324226033101");

	// 	QString btnObj = ((QPushButton*)sender())->text(); //获取按钮txt文字
	pProgressBar->setValue(0);//执行开始时恢复初始状态。
	auto buttonThis = (QPushButton*)sender();
	//buttonThis->blockSignals(true);

	//1 增加判断(输入地址不为空)
	if (m_pFilePathLineEdit->text().isEmpty() || m_pFilePathLineEditOUTPUT->text().isEmpty())
	{
		QWidget* PreViewWidget = new QWidget();
		PreViewWidget->setWindowTitle("警告");
		PreViewWidget->setFixedSize(200, 100);
		QHBoxLayout* pHighLightayout = new QHBoxLayout();
		QLabel* pLabel1 = new QLabel(QString("请输入地址后再执行!"));
		pHighLightayout->addWidget(pLabel1);
		PreViewWidget->setLayout(pHighLightayout);
		PreViewWidget->show();
		return;
	}

	//2 创建输出目录
	buttonThis->setText("处理中...");
	buttonThis->setEnabled(false);

	//外部输入地址并更新
	QString userPath = m_pFilePathLineEdit->text();
	//QString userPath = 

	// D:\CIM3_6\NNU-MiniCIM_05-14\CIMDataSet_05_14
	while (userPath.at(userPath.size() - 1) == "/" || userPath.at(userPath.size() - 1) == "\\")
		userPath.chop(1);//删除末尾斜杠
	QString inputPath = userPath + LOD3Path;//输入地址
	QString outputFolder = userPath + LOD3Path;//写出地址（同读入目录）
											   //QString inputPath = "set inputPath=" + m_pFilePathLineEdit3EDPART->text();//三方库地址
											   //ui->playVideoButton->setEnabled(false);
	QDir outputPath(outputFolder);
	QString tempPath = exePath + "\\temp";//临时文件夹位置
	QDir qDirWtsTemp = tempPath;//删除上次生成的中间文件
	qDirWtsTemp.removeRecursively();
	QString jsonFile = userPath + "\\SyntheticModel\\CIM3\\CIM3.json";


	// 记录模型成功与否的数量
	int SumModel = 0;
	int SuccessModel = 0;

	// 获得文件下模型个数
	QString ModelFile = userPath + "\\SyntheticModel\\CIM3\\ExternalData\\building\\surface";
	FechFile(ModelFile, SumModel, 1);
	

	//不删除原始output文件夹。
	//	outputPath.removeRecursively();
	//	outputPath.mkdir(outputFolder);  // 使用mkdir来创建文件夹

	//3 更新并执行bat，创建cfg文件
	//3.1 传参更新bat
	string batPath = "./LOD3Process.bat";
	ifstream ifs(batPath);
	string batSTR;
	string temp;	//记录当前行行内容
	while (getline(ifs, temp))
	{
		istringstream iss(temp);	// 输入流-当前行
		if (temp.find("set userPath=") <= 0)//找到第一个字符串位置
			temp = "set userPath=" + userPath.toStdString();
		else if (temp.find("set tempPath=") <= 0)
			temp = "set tempPath=" + tempPath.toStdString();
		else if (temp.find("set inputPath=") <= 0)
			temp = "set inputPath=" + inputPath.toStdString();
		else if (temp.find("set outputFolder=") <= 0)
			temp = "set outputFolder=" + outputFolder.toStdString();
		else if (temp.find("set jsonFile=") <= 0)
			temp = "set jsonFile=" + jsonFile.toStdString();//json地址写死
															// 		if (temp.find("set inputPath=") <= 0)//找到第一个字符串位置
															// 			temp = "set inputPath=" + inputPath.toStdString();
															// 		else if (temp.find("set outputFolder=") <= 0)//找到第一个字符串位置
															// 			temp = "set outputFolder=" + outputFolder.toStdString();
		batSTR += temp + "\n";
	}
	ifs.close();
	ofstream ofs(batPath);
	ofs << batSTR;
	ofs.close();
	//3.2 执行bat创建cfg
	QProcess* process = new QProcess();
	process->setWorkingDirectory("./");
	//QString commed = "./LOD3Process.bat";
	process->start(QString::fromStdString(batPath));
	process->waitForFinished(-1);
	pProgressBar->setValue(1);//进度条

							  //4 开始按照顺序执行程序。
							  //cout << QDir::currentPath().toStdString() << endl;
							  //QString exePath = QDir::currentPath();//获取当前运行环境
							  //QString exePath = "H:\\LOD3\\LOD3Pack20240417\\exe";//三方库地址
	QTime qTime;
	qTime.start();
	string logs = "建筑墙面构件化运行日志：\n\n";
	QDateTime dateTime = QDateTime::currentDateTime();//获取系统当前的时间
	logs += "开始时间: "+dateTime.toString("yyyy-MM-dd hh:mm:ss").toStdString() + "\n";//格式化时间
	QProcess program;
	QStringList args;
	//4.0 0formatOsgbObj
	QString exeFile = exePath + "\\0formatOsgbObj\\PO_formatOsgbObj.exe";
	QString cfgFolder = tempPath + "\\cfg";//定义cfg文件夹位置
	QString cfgFile = cfgFolder + "\\0osgb2obj.cfg";
	args.append("--cfg");
	args.append(cfgFile);
	program.setWorkingDirectory(exePath + "\\0formatOsgbObj");//exe运行环境
	program.start(exeFile, args);
	program.waitForFinished(-1);
	//耗时统计
	double time0 = qTime.elapsed() / 1.0e3;
	//logs += "0formatOsgbObj:   " + to_string(time0) + "s\n";
	++SuccessModel;
	//进度条更新
	QCoreApplication::processEvents();
	pProgressBar->setValue(2);
	//4.1 1contour
	exeFile = exePath + "\\1contour\\GPO_buildingsContourSimplify.exe";
	cfgFile = cfgFolder + "\\1buildingsContourSimplify.cfg";
	args.pop_back();
	args.append(cfgFile);
	program.setWorkingDirectory(exePath + "\\1contour");
	program.start(exeFile, args);
	program.waitForFinished(-1);
	//耗时统计
	double time1 = qTime.elapsed() / 1.0e3;
	//logs += "1contour:         " + to_string(time1 - time0) + "s\n";
	++SuccessModel;
	//进度条更新
	QCoreApplication::processEvents();
	pProgressBar->setValue(3);
	//4.2 2LOD2
	exeFile = exePath + "\\2LOD2\\GPO_LOD2.exe";
	cfgFile = cfgFolder + "\\2LOD2.cfg";
	args.pop_back();
	args.append(cfgFile);
	program.setWorkingDirectory(exePath + "\\2LOD2");
	program.start(exeFile, args);
	program.waitForFinished(-1);
	//耗时统计
	double time2 = qTime.elapsed() / 1.0e3;
	//logs += "2LOD2:            " + to_string(time2 - time1) + "s\n";
	++SuccessModel;
	//进度条更新
	QCoreApplication::processEvents();
	pProgressBar->setValue(4);
	//4.3 3render2file

	//test
	// 	QString exePath = QDir::currentPath();//获取当前运行环境
	// 	QString outputFolder = m_pFilePathLineEditOUTPUT->text();//写出地址
	//QString exeFile = exePath + "\\3render2file\\GPO_singleBuildingFacadeImage.exe";
	//QString cfgFile = outputFolder + "\\cfg\\3render2file.cfg";
	//QProcess program;
	//QStringList args;
	//args.append("--cfg");
	//args.append(cfgFile);
	//program.setWorkingDirectory(exePath + "\\3render2file");
	//program.start(exeFile, args);
	//program.waitForFinished();

	//QProcess program2;
	exeFile = exePath + "\\3render2file\\GPO_singleBuildingFacadeImage.exe";
	cfgFile = cfgFolder + "\\3render2file.cfg";
	args.pop_back();
	args.append(cfgFile);
	program.setWorkingDirectory(exePath + "\\3render2file");
	program.start(exeFile, args);
	program.waitForFinished(-1);

	//耗时统计
	double time3 = qTime.elapsed() / 1.0e3;
	//logs += "3render2file:     " + to_string(time3 - time2) + "s\n";
	++SuccessModel;
	//进度条更新
	QCoreApplication::processEvents();
	pProgressBar->setValue(5);
	//4.4 4WallPicSeg
	exeFile = exePath + "\\479LOD3\\GPO_wallClassify.exe";
	cfgFile = cfgFolder + "\\4WallPicSeg.cfg";
	args.pop_back();
	args.append(cfgFile);
	program.setWorkingDirectory(exePath + "\\479LOD3");
	program.start(exeFile, args);
	program.waitForFinished(-1);
	//耗时统计
	double time4 = qTime.elapsed() / 1.0e3;
	//logs += "4WallPicSeg:      " + to_string(time4 - time3) + "s\n";
	++SuccessModel;
	//进度条更新
	QCoreApplication::processEvents();
	pProgressBar->setValue(6);
	//4.5 5samyolo
	//复制exe到执行位置
	QString yolosamEnv = m_pFilePathLineEdit3EDPART->text() + "/5yoloseg";
	copyDirectory(exePath + "/5yoloseg", yolosamEnv, true);
	exeFile = yolosamEnv + "\\window_door_seg.exe";
	cfgFile = cfgFolder + "\\5samyolo.cfg";
	args.pop_back();
	args.append(cfgFile);
	program.setWorkingDirectory(yolosamEnv);
	program.start(exeFile, args);
	program.waitForFinished(-1);
	//耗时统计
	double time5 = qTime.elapsed() / 1.0e3;
	//logs += "5samyolo:         " + to_string(time5 - time4) + "s\n";
	++SuccessModel;
	//进度条更新
	QCoreApplication::processEvents();
	pProgressBar->setValue(7);
	//4.6 6layoutReasoning
	exeFile = exePath + "\\6layoutReasoning\\layoutReasoning.exe";
	cfgFile = cfgFolder + "\\6layoutReasoning.cfg";
	args.pop_back();
	args.append(cfgFile);
	program.setWorkingDirectory(exePath + "\\6layoutReasoning");
	program.start(exeFile, args);
	program.waitForFinished(-1);
	//耗时统计
	double time6 = qTime.elapsed() / 1.0e3;
	//logs += "6layoutReasoning: " + to_string(time6 - time5) + "s\n";
	++SuccessModel;
	//进度条更新
	QCoreApplication::processEvents();
	pProgressBar->setValue(8);
	//4.7 7generateLOD3
	exeFile = exePath + "\\479LOD3\\GPO_wallClassify.exe";
	cfgFile = cfgFolder + "\\7generateLOD3.cfg";
	args.pop_back();
	args.append(cfgFile);
	program.setWorkingDirectory(exePath + "\\479LOD3");
	program.start(exeFile, args);
	program.waitForFinished(-1);
	//耗时统计
	double time7 = qTime.elapsed() / 1.0e3;
	//logs += "7generateLOD3:    " + to_string(time7 - time6) + "s\n";
	++SuccessModel;
	//进度条更新
	QCoreApplication::processEvents();
	pProgressBar->setValue(9);
	//4.8 8textureSelect
	exeFile = exePath + "\\8textureSelect\\textmatch.exe";
	cfgFile = cfgFolder + "\\8textureSelect.cfg";
	args.pop_back();
	args.append(cfgFile);
	program.setWorkingDirectory(exePath + "\\8textureSelect");
	program.start(exeFile, args);
	program.waitForFinished(-1);
	//耗时统计
	double time8 = qTime.elapsed() / 1.0e3;
	//logs += "8textureSelect:   " + to_string(time8 - time7) + "s\n";
	++SuccessModel;
	//进度条更新
	QCoreApplication::processEvents();
	pProgressBar->setValue(10);
	//4.9 9buildingTexture
	exeFile = exePath + "\\479LOD3\\GPO_wallClassify.exe";
	cfgFile = cfgFolder + "\\9buildingTexture.cfg";
	args.pop_back();
	args.append(cfgFile);
	program.setWorkingDirectory(exePath + "\\479LOD3");
	program.start(exeFile, args);
	program.waitForFinished(-1);
	//耗时统计
	double time9 = qTime.elapsed() / 1.0e3;
	//logs += "9buildingTexture: " + to_string(time9 - time8) + "s\n";
	++SuccessModel;
	//进度条更新
	QCoreApplication::processEvents();
	pProgressBar->setValue(11);
	//4.10 10obj2osgb
	exeFile = exePath + "\\0formatOsgbObj\\PO_formatOsgbObj.exe";
	cfgFile = cfgFolder + "\\10obj2osgb.cfg";
	args.pop_back();
	args.append(cfgFile);
	program.setWorkingDirectory(exePath + "\\0formatOsgbObj");
	program.start(exeFile, args);//注意：最后一个改成start控制台无输出。
	program.waitForFinished(-1);
	//耗时统计
	double time10 = qTime.elapsed() / 1.0e3;
	//logs += "10obj2osgb:       " + to_string(time10 - time9) + "s\n";
	++SuccessModel;
	//进度条更新
	QCoreApplication::processEvents();
	pProgressBar->setValue(12);


	QDateTime dateTime2 = QDateTime::currentDateTime();
	int minutes = dateTime.secsTo(dateTime2);
	int time_ = dateTime.msecsTo(dateTime2);
	logs += "结束时间: " + dateTime.toString("yyyy-MM-dd hh:mm:ss").toStdString() + "\n";//格式化时间

	logs += "总耗时: " + minutes;
	logs += "分\n\n";

	//5 写出log
	//logs += "Total Time:       " + to_string(time10 - time0) + "s\n";

	logs += "总模型数量      " + to_string(SumModel) + "\n";
	int tol = SumModel;
	// 检验有多少模型成功
	FechFile(ModelFile, SumModel, 0);
		
	logs += "成功模型数量     " + to_string(SumModel) + "\n";
	logs += "失败模型数量     " + to_string(tol - SumModel) + "\n";

	string logFile = exePath.toStdString() + "\\lod3.log";//log放入运行环境
	ofstream ofsb(logFile);
	ofsb << logs;
	ofsb.close();
	//6 收尾工作
	//6.1 删除yolosamexe
	QFile::remove(yolosamEnv + "/window_door_seg.exe");
	QFile::remove(yolosamEnv + "/temp.png");
	QFile::remove(yolosamEnv + "/traced_model.pt");
	QDir qDirWts(yolosamEnv + "/weights");
	qDirWts.removeRecursively();
	//qDirWts = tempPath;//删除tempPath（改为初始化时删除，后面保留方便查看）
	//qDirWts.removeRecursively();

	//6.2 按钮复原
	buttonThis->setText("开始处理");
	buttonThis->setEnabled(true);
	//buttonThis->blockSignals(false);
}

void NightEffect::openLog()
{
	QString outputPath = exePath + "/lod3.log";//当前将log放入运行环境中。
	QUrl url = QUrl::fromUserInput(outputPath, QDir::currentPath(), QUrl::AssumeLocalFile);
	QDesktopServices::openUrl(url);
	//QFileDialog::getOpenFileName(this, "open file", "./output/lod3.log");//打开D盘study文件夹
}

/*
*@brief 遍历所有子文件夹，筛选出特定格式的文件。
*@param[in]inputFolderPath		当前选中的点云目标。
*@param[in]appendixStr			需要筛选的文件后缀。例如：".obj"
return	目标格式的文件列表。
*/
vector<std::string> findAllFiles(string inputFolderPath, string appendixStr)
{
	//安全：判断字符串最后一位是否以“/”或“\\”结尾。
	auto a = inputFolderPath.back();
	if (a == '/' || a == '\\')
		inputFolderPath.pop_back();
	vector<std::string> childrenList;
	_finddata_t data;
	auto handle = _findfirst((inputFolderPath + "/*.*").c_str(), &data); //读取第1文件或文件夹
	if (handle == -1)//判断是否可以读取文件
	{
		cout << "can not read file!";
		_findclose(handle);// 关闭搜索句柄
		return childrenList;
	}

	do
	{
		string s = data.name;
		if (data.attrib & _A_SUBDIR)//是目录
		{
			if (s != "." && s != "..") //判断是否为"."当前目录，或".."上一层目录
			{
				vector<std::string> tempList = findAllFiles(inputFolderPath + "\\" + s, appendixStr);//递归调用
				childrenList.insert(childrenList.end(), tempList.begin(), tempList.end());
			}
		}
		else if (!(data.attrib & _A_SUBDIR)) //读入是文件
		{
			if (s.rfind(appendixStr) == s.size() - appendixStr.size()) // 判断后缀是否为appendixStr
			{
				childrenList.push_back(inputFolderPath + "\\" + s);
			}
		}
		else
			continue;

	} while (_findnext(handle, &data) == 0); //读取下一个文件夹

	_findclose(handle);// 关闭搜索句柄
					   //cout << "列表目标文件数量：" << childrenList.size() << endl;
	return  childrenList;
}
// QFileInfoList FindFile(const QString &strFilePath, const QString &strNameFilters)
// {
// 	QFileInfoList fileList;
// 	if (strFilePath.isEmpty() || strNameFilters.isEmpty())
// 	{
// 		return fileList;
// 	}
//
// 	QDir dir;
// 	QStringList filters;
// 	filters << strNameFilters;
// 	dir.setPath(strFilePath);
// 	dir.setNameFilters(filters);
// 	QDirIterator iter(dir, QDirIterator::Subdirectories);
// 	while (iter.hasNext())
// 	{
// 		iter.next();
// 		QFileInfo info = iter.fileInfo();
// 		if (info.isFile())
// 		{
// 			fileList.append(info);
// 		}
// 	}
// 	return fileList;
// }

//调用脚本打开osgviewer，显示点击的目标
void NightEffect::openOsgviewer(QListWidgetItem *item)
{
	//1 获取文件名
	string name = item->text().toStdString();
	//2 查找对应完整地址
	//读入所有osgb
	QString userPath = m_pFilePathLineEdit->text();
	while (userPath.at(userPath.size() - 1) == "/" || userPath.at(userPath.size() - 1) == "\\")
		userPath.chop(1);//删除末尾斜杠
	QString inputPath = userPath + LOD3Path;//输入地址
	vector<std::string> osgbList = findAllFiles(inputPath.toStdString(), ".osgb");
	//QFileInfoList osgbList = FindFile(inputPath, ".osgb");
	QString osgbFilePath;
	for (int i = 0; i < osgbList.size(); i++)
	{
		if (osgbList[i].find(name) <= 1000)//找到所需覆盖的osgb
		{
			osgbFilePath = QString::fromStdString(osgbList[i]);
			break;
		}
	}

	//3 执行脚本
	QProcess program;
	QStringList args;
	QString exeFile = exePath + "\\osgviewer.exe";
	//QString osgbFilePath = cfgFolder + "\\0osgb2obj.cfg";
	//args.append("--window 100 100 800 600");
	args.append("--window");
	args.append("100");
	args.append("100");
	args.append("800");
	args.append("600");
	args.append(osgbFilePath);
	program.setWorkingDirectory(exePath);//exe运行环境
	program.start(exeFile, args);
	program.waitForFinished(-1);
}

void NightEffect::openOutput()
{
	if (IsOpen) {
		listWidget->close();
		IsOpen = false;
		return;
	}
	//this->setFixedSize(1000, 700);//修改主窗口大小
								  //this->setFixedSize(500, 350);//修改主窗口大小
	listWidget = new QListWidget(this);
	listWidget->setGeometry(5, 200, 990, 200);//(左上点距离坐边距离，左上点距离上边距离，宽度，高度)
											  //listWidget->setGeometry(5, 180, 490, 150);//(左上点距离坐边距离，左上点距离上边距离，宽度，高度)
											  //listWidget->resize(200, 400);

											  //读入所有osgb
	listWidget->setFixedSize(600, 300);
	QString userPath = m_pFilePathLineEdit->text();
	while (userPath.at(userPath.size() - 1) == "/" || userPath.at(userPath.size() - 1) == "\\")
		userPath.chop(1);//删除末尾斜杠
	QString inputPath = userPath + LOD3Path;//输入地址
	vector<std::string> osgbList = findAllFiles(inputPath.toStdString(), ".osgb");
	//QFileInfoList osgbList = FindFile(inputPath, ".osgb");
	for (int i = 0; i < osgbList.size(); i++)
	{
		string osgbPath = osgbList[i];
		int index = max(osgbPath.find_last_of('/'), osgbPath.find_last_of('\\'));
		//std::string folderPath = osgbPath.substr(0, index);//"G:\Desktop\LOD3\LOD3SAutoExe\testdata\input\0buildingObj\CIM2a40"
		std::string filename = osgbPath.substr(index + 1, osgbPath.size() - index - 1);//CIM2a40.osgb
																					   //listWidget->addItem("C语言中文网");
		QString osgbFile = QFileInfo(QString::fromStdString(osgbList[i])).fileName();
		listWidget->addItem(osgbFile);
	}
	listWidget->show();
	IsOpen = true;
	listWidget->setViewMode(QListView::ListMode);

	//点击osgb文件打开
	connect(listWidget, &QListWidget::itemClicked, this, &NightEffect::openOsgviewer);

	// 	//上一版本
	//  	QString outputPath = m_pFilePathLineEditOUTPUT->text();
	// 	QUrl url = QUrl::fromUserInput(outputPath, QDir::currentPath(), QUrl::AssumeLocalFile);
	// 	QDesktopServices::openUrl(url);
}

//grc::RenderControl g_controller;

void NightEffect::SetFilePath()
{
	QString str = QFileDialog::getExistingDirectory(
		this, "choose src Directory",
		"/");


	m_pFilePathLineEdit->setText(str);
	m_pFilePathLineEdit2->setText(str);

	// 配置文件
	QFile file("Setting.txt");

	file.open(QIODevice::WriteOnly);

	QTextStream out(&file);

	// 写入内容到文件
	out << str;

	// 关闭文件
	file.close();
	

}


void SetJsonFile() {

}

NightEffect::NightEffect(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	
}


NightEffect::~NightEffect()
{

}


void NightEffect::OnBrowseFileData()
{
	QString DataDir = QFileDialog::getOpenFileName(this, QObject::tr("Open ACIM File"), "./", QObject::tr("ACIM File(*.json);;All files(*.*)"));
	//去掉关闭窗口时，警告弹窗
	if (DataDir.isEmpty())
		return;

	m_pFilePathLineEdit->setText(DataDir);
}


void NightEffect::OnProcessFile()
{
	return;
}






// void NightEffect::OnPreView()
// {
//     QWidget* PreViewWidget = new QWidget();
//     PreViewWidget->setWindowFlags(Qt::WindowCloseButtonHint);
//     //PreViewWidget->setWindowFlags(Qt::FramelessWindowHint);
//     PreViewWidget->setWindowIcon(QIcon());
//     PreViewWidget->setFixedSize(600, 500);
//
//     ViewerWidget* pViewerWidget = new ViewerWidget(PreViewWidget);
//     pViewerWidget->setGeometry(10,10,580,480);
//     pViewerWidget->initViewer();
//
//
//     PreViewWidget->show();
// }



void NightEffect::initWindow()
{
	QWidget* centralWidget = new QWidget();
	centralWidget->setObjectName(QStringLiteral("centralWidget"));
	this->setCentralWidget(centralWidget);


	QVBoxLayout* pMainLayerout = new QVBoxLayout();
	exePath = QDir::currentPath();//初始化exe地址
	exePath.replace("/", "\\");//bat脚本地址不识别正斜杠
	LOD3Path = "\\SyntheticModel\\CIM3\\ExternalData\\building\\surface";//初始化LOD3相对地址

	QHBoxLayout* pHighLightayout = new QHBoxLayout();
	QLabel* pLabel1 = new QLabel(QString("输入模型 "));
	pHighLightayout->addWidget(pLabel1);

	m_pFilePathLineEdit = new QLineEdit();
	m_pFilePathLineEdit->setObjectName(QString("File"));
	m_pFilePathLineEdit->setText("请选择您的路径：");


	QHBoxLayout* pHighLightayout2 = new QHBoxLayout();
	QLabel* pLabel2 = new QLabel(QString("输出目录 "));

	pHighLightayout2->addWidget(pLabel2);

	m_pFilePathLineEdit2 = new QLineEdit();
	m_pFilePathLineEdit2->setObjectName(QString("File"));
	m_pFilePathLineEdit->setText("");

	QFile FileSetting("./Setting.txt");
	if (FileSetting.exists()) {
		QString pathSetting = readFile("./Setting.txt");
		m_pFilePathLineEdit->setText(pathSetting);
		m_pFilePathLineEdit2->setText(pathSetting);
	}
	

	pHighLightayout->addWidget(m_pFilePathLineEdit);
	pHighLightayout2->addWidget(m_pFilePathLineEdit2);

	//connect(m_btn, SIGNAL(clicked()), this, SLOT(SetFilePath()));
	m_btn = new QPushButton;
	//QIcon icon;
	//icon.addFile(tr("D:\\Vs2015\\LOD3UI\\build\\FilePic.png"));
	//m_btn->setIcon(icon);

	m_btn->setText("预览");
	m_btn->setFixedSize(60, 40);

	m_btn->setStyleSheet("QPushButton{background-color: rgb(225, 225, 225);border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}"
		"QPushButton:hover{background-color:rgb(229, 241, 251); color: black;}"
		"QPushButton:pressed{background-color:rgb(204, 228, 247);border-style: inset;}");

	

	connect(m_btn, &QPushButton::clicked, this, &NightEffect::SetFilePath);
	pHighLightayout->addWidget(m_btn);
	
	//     QPushButton* pBrowseFileButton = new QPushButton("...");
	//     pBrowseFileButton->setObjectName(QStringLiteral("FileBrowse"));
	//     pHighLightayout->addWidget(pBrowseFileButton);
	//     //屏蔽掉回车触发信号,添加浏览文件
	//     pBrowseFileButton->setDefault(false);
	//     pBrowseFileButton->setAutoDefault(true);
	//     connect(pBrowseFileButton, SIGNAL(clicked()), this, SLOT(OnBrowseFileData()));

	// 	//xuek 文件写出地址
	// 	QHBoxLayout* pHighLightayout3 = new QHBoxLayout();
	// 	//1）字符提示
	// 	QLabel* pLabelOUTPUT = new QLabel(QString("写出地址："));
	// 	pHighLightayout3->addWidget(pLabelOUTPUT);
	// 	//2）地址输入框（不显示）
	m_pFilePathLineEditOUTPUT = new QLineEdit();
	m_pFilePathLineEditOUTPUT->setObjectName(QString("File"));
	m_pFilePathLineEditOUTPUT->setText("H:\\LOD3\\LOD3Pack20240417\\testdata\\output");
	// 	pHighLightayout3->addWidget(m_pFilePathLineEditOUTPUT);
	// 	//3）文件搜索框
	// 	QPushButton* pBrowseFileButtonOUTPUT = new QPushButton("...");
	// 	pBrowseFileButtonOUTPUT->setObjectName(QStringLiteral("FileBrowse"));
	// 	pHighLightayout3->addWidget(pBrowseFileButtonOUTPUT);
	// 	//屏蔽掉回车触发信号,添加浏览文件
	// 	pBrowseFileButtonOUTPUT->setDefault(false);
	// 	pBrowseFileButtonOUTPUT->setAutoDefault(true);

	//xuek 三方库地址
	// 	QHBoxLayout* pHighLightayout2 = new QHBoxLayout();
	// 	//1）字符提示
	// 	QLabel* pLabel3EDPART = new QLabel(QString("三方库：  "));
	// 	pHighLightayout2->addWidget(pLabel3EDPART);
	// 	//2）地址输入框
	QDir *dirNow = new QDir(QDir::currentPath());
	dirNow->cdUp();//获取上一级目录
	m_pFilePathLineEdit3EDPART = new QLineEdit();
	m_pFilePathLineEdit3EDPART->setObjectName(QString("File"));
	QString LOD3DLLPath = dirNow->path() + "/LOD3DLL";//三方库位置写死。
	m_pFilePathLineEdit3EDPART->setText(LOD3DLLPath);
	// 	pHighLightayout2->addWidget(m_pFilePathLineEdit3EDPART);
	// 	//3）文件搜索框
	// 	QPushButton* pBrowseFileButton3EDPART = new QPushButton("...");
	// 	pBrowseFileButton3EDPART->setObjectName(QStringLiteral("FileBrowse"));
	// 	pHighLightayout2->addWidget(pBrowseFileButton3EDPART);
	// 	//屏蔽掉回车触发信号,添加浏览文件
	// 	pBrowseFileButton3EDPART->setDefault(false);
	// 	pBrowseFileButton3EDPART->setAutoDefault(true);

	//设置temp地址

	//读入bat中路径，初始化input、output地址
	ifstream ifs("./LOD3Process.bat");
	//string batSTR;
	string temp;	//记录当前行行内容
	while (getline(ifs, temp))
	{
		istringstream iss(temp);	// 输入流-当前行
		if (temp.find("set userPath=") <= 0)//读入 userPath
		{
			string userPath = temp.substr(13);
			//string LOD3Path = "\\SyntheticModel\\CIM3\\ExternalData\\building\\surface";
			m_pFilePathLineEdit->setText(QString::fromStdString(userPath));
			m_pFilePathLineEdit2->setText(QString::fromStdString(userPath));
			m_pFilePathLineEditOUTPUT->setText(QString::fromStdString(userPath) + LOD3Path);
			break;//找到后就退出
		}
		// 			m_pFilePathLineEdit->setText(QString::fromStdString(temp.substr(13)));
		// 		else if (temp.find("set outputFolder=") <= 0)//找到第一个字符串位置
		// 		{
		// 			m_pFilePathLineEditOUTPUT->setText(QString::fromStdString(temp.substr(17)));
		// 			break;//找到后就退出
	}
	ifs.close();

	//进度条
	QVBoxLayout* pProgressLayout = new QVBoxLayout();

	//1）字符提示
	//QLabel* pLabelPROGRESS = new QLabel(QString("处理进度："));
	//pProgressLayout->addWidget(pLabelPROGRESS);
	//2）进度条
	pProgressBar = new QProgressBar();
	//pProgressBar = new QProgressBar();
	pProgressBar->setObjectName(QStringLiteral("Progress Bar"));
	pProgressBar->setRange(0, 12);
	pProgressBar->setValue(0);
	pProgressLayout->addWidget(pProgressBar);
	pProgressBar->setAlignment(Qt::AlignCenter);


	//添加按钮
	QHBoxLayout* pProcessLayout = new QHBoxLayout();
	QPushButton* pProcessButton = new QPushButton("运行");
	pProcessButton->setObjectName(QStringLiteral("Process"));
	pProcessLayout->addWidget(pProcessButton, 1);
	//pProcessButton->setGeometry(0, 0, 100, 25);
	pProcessButton->setFixedSize(125, 50);
	connect(pProcessButton, &QPushButton::clicked, this, &NightEffect::LOD3Process);

	pProcessButton->setStyleSheet("QPushButton{background-color: rgb(225, 225, 225);border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}"
		"QPushButton:hover{background-color:rgb(229, 241, 251); color: black;}"
		"QPushButton:pressed{background-color:rgb(204, 228, 247);border-style: inset;}");


	//connect(pProcessButton, SIGNAL(clicked()), this, SLOT(copyFileToDst()));
	//     //屏蔽掉回车触发信号,添加浏览文件
	//     pProcessButton->setDefault(false);
	//     pProcessButton->setAutoDefault(true);
	//     pProcessButton->setFixedHeight(40);

	QPushButton* pPreViewButton = new QPushButton("预览...");
	pPreViewButton->setObjectName(QStringLiteral("view"));
	pProcessLayout->addWidget(pPreViewButton);
	pPreViewButton->setFixedSize(125, 50);
	connect(pPreViewButton, &QPushButton::clicked, this, &NightEffect::openOutput);
	//     //屏蔽掉回车触发信号,添加浏览文件
	//     pPreViewButton->setDefault(false);
	//     pPreViewButton->setAutoDefault(true);
	//     pPreViewButton->setFixedHeight(40);
	//    // connect(pPreViewButton, SIGNAL(clicked()), this, SLOT(OnPreView()));
	pPreViewButton->setStyleSheet("QPushButton{background-color: rgb(225, 225, 225);border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}"
		"QPushButton:hover{background-color:rgb(229, 241, 251); color: black;}"
		"QPushButton:pressed{background-color:rgb(204, 228, 247);border-style: inset;}");

	QPushButton* pLogButton = new QPushButton("日志...");
	pLogButton->setObjectName(QStringLiteral("Log"));
	pProcessLayout->addWidget(pLogButton);
	pLogButton->setFixedSize(125, 50);
	connect(pLogButton, &QPushButton::clicked, this, &NightEffect::openLog);

	pLogButton->setStyleSheet("QPushButton{background-color: rgb(225, 225, 225);border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}"
		"QPushButton:hover{background-color:rgb(229, 241, 251); color: black;}"
		"QPushButton:pressed{background-color:rgb(204, 228, 247);border-style: inset;}");

	// 添加两个实例图片
	QLabel* pic1 = new QLabel();
	QLabel* pic2 = new QLabel();
	QLabel* pic3 = new QLabel();
	QHBoxLayout* pPicLayout = new QHBoxLayout(); 
	QHBoxLayout* pPicLayout2 = new QHBoxLayout();
	QVBoxLayout* pv1Layout = new QVBoxLayout();
	QVBoxLayout* pv2Layout = new QVBoxLayout();
	QVBoxLayout* pv3Layout = new QVBoxLayout();
	QLabel* pic1Info = new QLabel();
	QLabel* pic2Info = new QLabel();

	//pPicLayout->setContentsMargins(20, 20, 20, 20);
	pPicLayout->setContentsMargins(20, 20, 20, 20);


	pPicLayout2->addWidget(pic2);
	pPicLayout2->addWidget(pic3);


	pic1Info->setText(tr("处理前(示意)"));
	pic1Info->raise();

	pic2Info->setText(tr("处理后(示意)"));

	pic1->setFixedSize(300, 231);
	pic2->setFixedSize(200, 231);
	pic3->setFixedSize(161, 231);

	pic1->setFrameShape(QFrame::Box);
	pic1->setMargin(10);
	pic1->setStyleSheet("QLabel { border-top: 2px solid #d3d3d3; border-left: 2px solid #d3d3d3; border-bottom: 2px solid #d3d3d3; border-right: 2px solid #d3d3d3; }");
	pic2->setStyleSheet("QLabel { border-top: 2px solid #d3d3d3; border-left: 2px solid #d3d3d3; border-bottom: 2px solid #d3d3d3; border-right: 0px; }");
	pic3->setStyleSheet("QLabel { border-top: 2px solid #d3d3d3; border-left: 0px; border-bottom: 2px solid #d3d3d3; border-right: 2px solid #d3d3d3; }");

	pv1Layout->addWidget(pic1Info);
	pv1Layout->addWidget(pic1);
	
	pv2Layout->addWidget(pic2Info);
	pv2Layout->addLayout(pPicLayout2);
	

	//QDir *dirNow = new QDir(QDir::currentPath());

	// 加载并设置图片到 QLabel
	QPixmap picBefore(".\\pic_before.png");
	QPixmap picEnd(".\\pic_end0.png");
	QPixmap picEnd0(".\\pic_end.png");

	if (picBefore.isNull()) {
		qWarning() << "Failed to load image: :/new/pict/pic_before.png";
	}
	else {
		pic1->setPixmap(picBefore.scaled(pic1->size(), Qt::KeepAspectRatio));
	}

	if (picEnd.isNull()) {
		qWarning() << "Failed to load image: :/new/pict/pic_end.png";
	}
	else {
		pic2->setPixmap(picEnd.scaled(pic2->size(), Qt::KeepAspectRatio));
		pic3->setPixmap(picEnd0.scaled(pic3->size(), Qt::KeepAspectRatio));
	}

	pPicLayout->addLayout(pv1Layout);
	pPicLayout->addLayout(pv2Layout);
	//pPicLayout->addWidget(pic1);
	//pPicLayout->addWidget(pic2);

	//     //屏蔽掉回车触发信号,添加浏览文件
	//     pLogButton->setDefault(false);
	//     pLogButton->setAutoDefault(true);
	//     pLogButton->setFixedHeight(40);


	// 	QListWidget * listout = new QListWidget;
	// 	listout->resize(200, 200);
	// 	pProcessLayout->addWidget(*listout);
	// 	QListWidget list_widget;
	// 	list_widget.setGeometry(50, 70, 50, 60);
	// 	list_widget.addItem("C语言中文网");
	// 	list_widget.addItem("http://c.biancheng.net");
	// 	list_widget.addItem(new QListWidgetItem("Qt教程"));
	// 	list_widget.setLayoutMode(QListView::Batched);



	//主布局
	pMainLayerout->addLayout(pHighLightayout);
	pMainLayerout->addSpacing(20);

	pMainLayerout->addLayout(pHighLightayout2);
	pMainLayerout->addSpacing(40);
	// 	pMainLayerout->addLayout(pHighLightayout3);
	// 	pMainLayerout->addSpacing(10);
	// 	pMainLayerout->addLayout(pHighLightayout2);
	// 	pMainLayerout->addSpacing(10);


	// 添加两个图片
	pMainLayerout->addLayout(pPicLayout);
	pMainLayerout->addSpacing(40);

	// 进度条
	pMainLayerout->addLayout(pProgressLayout);
	pMainLayerout->addSpacing(40);
	// 三个按钮
	pMainLayerout->addLayout(pProcessLayout);
	pMainLayerout->addStretch();


	pMainLayerout->addSpacing(40);

	

	
	centralWidget->setLayout(pMainLayerout);
	//centralWidget->setStyleSheet("QWidget { border: 2px solid #d3d3d3; }");
	// 	centralWidget->setFixedSize()
	// centralWidget->setFixedSize(500, 300);

	

}



// class CallbackProcessor
// {
// public:
//     CallbackProcessor();
//     ~CallbackProcessor();
//
//     bool InterCallback(gre::GUIEvent* pEvent, gre::GraphicsWindow* pGraphicsWindow,
//         gre::RenderEngine* pRenderEngine, bool);
// };
//
// CallbackProcessor::CallbackProcessor()
// {
//
// }
// CallbackProcessor::~CallbackProcessor()
// {
//
// }

// bool CallbackProcessor::InterCallback(gre::GUIEvent* event,
//     gre::GraphicsWindow* gwAlias, gre::RenderEngine* re, bool)
// {
//     return true;
// }

// CallbackProcessor m_callback;



ViewerWidget::ViewerWidget(QWidget *parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);

	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);
	setAutoFillBackground(true);    // for compatibility

	setFocusPolicy(Qt::WheelFocus);

}

ViewerWidget::~ViewerWidget()
{
	delete m_pViewer;
}



// void ViewerWidget::initViewer()
// {
//     m_pWindow = new grc::ControllerGraphicsWindowWin32();
//
//     if (!m_pWindow.Valid())
//     {
//         return;
//     }
//
//     m_pWindow->Initialize();
//     m_pWindow->BindWindowHandle((HWND)this->winId(), grs::RST_D3D12, false, false);
//
//
//
//
//     grc::ControllerGraphicsWindowWin32::ControllerEventCallback delegate;
//     delegate.bind(&m_callback, &CallbackProcessor::InterCallback);
//     m_pWindow->SetControllerEventCallback(delegate);
//
//
//     g_controller.Is3DWindowValid(true);
//     g_controller.Initialize(m_pWindow, true);
//
//     //g_controller.SetTriDTransSpeedUp(2.0);
//     ////InitSkyBox();
//
//
//     //if (true)
//     //    g_controller.Start();
//     ////Sleep(1000
//
//
//
//     ggp::CViewer* pViewer = m_pWindow->GetGGPViewer();
//     pViewer->Options()->SetMiddleButtonNavEnabled(false);
//     pViewer->Options()->SetMouseWheelOverrided(true);
//
//     pViewer->CommandManager()->ExecuteCommand(L"CityMakerCmd");
//     grc::CityMakerCmd* pCityMakerCmd = dynamic_cast<grc::CityMakerCmd*>(pViewer->CommandManager()->GetActiveCommand());
//     pCityMakerCmd->SetGraphicsWindow(m_pWindow);
//
//     m_pWindow->SetRenderEngine(g_controller.GetRenderEngine());
//
//     m_pWindow->GetEventQueue()->SetGGPViewer(pViewer);
//     g_controller.GetRenderEngine()->GetEventQueue()->SetGGPViewer(pViewer);
//     g_controller.GetEditEventQueue()->SetGGPViewer(pViewer);
//
//
//
//     //夜景 初始化环境
//     {
//         //g_controller.RenderSpecialEffect(8);
//
//         const ggt::Vec3d position = ggt::Vec3d(13356318.249969264, 3779708.5116773215, 421.9046);
//         const ggt::EulerAngle angle = ggt::EulerAngle(0.0000000000000000, -32.704220486917677, 0.0000000000000000);
//         g_controller.SetCamera(position, angle, grc::RenderControl::gvNone);
//
//
//         const gvString TDTile0 = L"G:/3dTile1.1/shitang/tileset.json";
//         gfx::Ptr<grc::TDTileLayer> ptrTemp0;
//         //添加新的图层NodeFeature
//         int32_t nResult0 = g_controller.GetIObjectManager()->
//             CreateTDTileLayer(TDTile0, L"", &ptrTemp0);
//
//
//        // g_controller.FlyToObject(ptrTemp0->GetGuid(),
//         //    grc::RenderControl::gvAcJump);
//
//
//
//         //ggp::CRealisticRenderManager* pManager = g_controller.GetMainWindow()->GetGGPViewer()->GetRealisticRenderManager();
//         //pManager->GetRealisticRenderParameters()->GetDynamicEnvironmentParams()->EnableStarSky(true);
//         //pManager->GetRealisticRenderParameters()->GetDynamicEnvironmentParams()->SetTime(17.0);
//         //pManager->GetEnvironmentModule()->Update();
//         //pManager->GetRealisticRenderParameters()->GetReflectEnvironmentAndSkyParams()->SetEnvironmentCubeMapDirty(true);
//
//         //g_controller.SetRayleighScattering(ggp::CVector4f(0.02, 0.01, 0.1, 1));
//         //g_controller.SetLensFlareEnable(false);
//
//     }
//
// }



QPaintEngine *ViewerWidget::paintEngine() const
{
	return nullptr;
}

// bool ViewerWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
// {
//     if (m_pWindow)
//     {
//         std::vector<char> data(eventType.begin(), eventType.end());
//         return m_pWindow->GetGGPViewer()->nativeEvent(data, message, result);
//     }
//
//     return QWidget::nativeEvent(eventType, message, result);
// }
//
// void ViewerWidget::wheelEvent(QWheelEvent *pEvent)
// {
//     if (m_pWindow == 0)
//     {
//         return;
//     }
//
//     bool bCtrlDown = (pEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier;
//     bool bShiftDown = (pEvent->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier;
//     bool bAltDown = (pEvent->modifiers() & Qt::AltModifier) == Qt::AltModifier;
//
//     ggp::CMouseEventArgs mouseArgs(ggp::ET_MOUSEWHEEL, 0,
//         pEvent->x(),
//         pEvent->y(),
//         (float)pEvent->delta() / (float)120,
//         bCtrlDown,
//         bShiftDown,
//         bAltDown);
//
//     m_pWindow->GetGGPViewer()->ProcessEvent(&mouseArgs);
// }
//
// void ViewerWidget::keyPressEvent(QKeyEvent *pEvent)
// {
//     if (m_pWindow == 0)
//     {
//         return;
//     }
//
//     ggp::CKeyEventArgs keyArgs(ggp::ET_KEYDOWN, pEvent->nativeVirtualKey(), 0);
//     m_pWindow->GetGGPViewer()->ProcessEvent(&keyArgs);
// }
//
// void ViewerWidget::keyReleaseEvent(QKeyEvent *pEvent)
// {
//     if (m_pWindow == 0)
//     {
//         return;
//     }
//
//     ggp::CKeyEventArgs keyArgs(ggp::ET_KEYUP, pEvent->nativeVirtualKey(), 0);
//     m_pWindow->GetGGPViewer()->ProcessEvent(&keyArgs);
//
// }
//
