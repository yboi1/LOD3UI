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

// ��¼�ļ����ֺʹ�С�� �ж��Ƿ�ִ�гɹ�
QMap<QString, int> m_map;

QJsonObject Main_Json;

QJsonArray arr_json;

// json �ļ�����
// Components����, ����·����Ӧ��Code, ���Ҿ����Ӧ��Code
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
			case(0): { // ����path·��
				if (!QString::compare(CheckCode, PathCode)) {
					QString Geometry = ConceptObject.toObject().value("Geometry").toString();
					//qDebug() << " Geometry:  " << Geometry << "\n";
					SetJsonPath(RootObj, Geometry, MatrixCode, 1, "ExternalDatas", inner_Json);
					return;
				}
				break;

			}
			case(1): { // �õ�·��
				if (!QString::compare(CheckCode, PathCode)) {
					QString OuterUrl = ConceptObject.toObject().value("OuterUrl").toString();
					//qDebug() << "OuterUrl" << OuterUrl << "\n";
					inner_Json.insert("Url", OuterUrl);
					return;
				}
				break;
			}
			case(2): { // д�����
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

// ��������� ��Ҫƥ���ConceptCode, �õ�����������(����),   Code�ַ����� Comments�ַ���, ����1��ʾ��ȡIDֵ 0 ��ʾ�õ�Components
void Get_IdCodeByConcept(QJsonValue& ConceptValue, QJsonObject& rootObj, QString &Code, QString &Comments, bool flag, QString ConceptCode = "") {


	if (ConceptValue.type() == QJsonValue::Array) {
		QJsonArray ConArray = ConceptValue.toArray();
		

		

		for (int i = 0; i < ConArray.size(); ++i) {
			QJsonValue ConceptObject = ConArray.at(i);
			
			if (ConceptObject.type() == QJsonValue::Object) {
				QJsonObject obj = ConceptObject.toObject();
				QString CheckCode = obj.value("ConceptCode").toString();
				
				if (flag) { // û�д���ID��˵����Ҫ�õ�id
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
							//qDebug() << "�ҵ�Ŀ�꣡" << "\n";

							// �õ�Components�µĵ��ĸ�Code ��·����Code
							QJsonArray PathCodeArr = obj.value("Components").toArray();
							QString PathCode = PathCodeArr.at(3).toObject().value("Code").toString();
							QString MatrixCode = PathCodeArr.at(2).toObject().value("Code").toString();
							

							// ����json����

							SetJsonPath(rootObj, PathCode, MatrixCode, 0, "Components", inner_Json);
							SetJsonPath(rootObj, PathCode, MatrixCode, 2, "Components", inner_Json);


							arr_json.append(inner_Json);


						}
					}
				}

			}
			else {
				qDebug() << "���������Ϊ���� ���飡\n";
			}
		}
	}
	else {
		qDebug() << "���������Ϊ���飬 ���飡\n";
	}
	
}

QJsonDocument read_file(QString file_path)
{
	QFile this_file_ptr(file_path);

	// �ж��ļ��Ƿ����
	if (false == this_file_ptr.exists())
	{
		qDebug() << "�ļ�������" << "\n";
	}
	if (false == this_file_ptr.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "��ʧ��" << "\n";
	}

	// ��ȡ���ı��е��ַ���
	QTextStream stream(&this_file_ptr);
	stream.setCodec("UTF-8");
	QString string_value = stream.readAll();
	this_file_ptr.close();

	// QJsonParseError��������JSON�����ڼ䱨�����
	QJsonParseError jsonError;
	// ��json����ΪUTF-8�����json�ĵ��������д���һ��QJsonDocument��
	// ��������ɹ�������QJsonDocument���󣬷��򷵻�null
	QJsonDocument doc = QJsonDocument::fromJson(string_value.toUtf8(), &jsonError);
	// �ж��Ƿ����ʧ��
	if (jsonError.error != QJsonParseError::NoError && !doc.isNull()) {
		qDebug() << "Json��ʽ����" << jsonError.error;
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


// �����ļ�����
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
				if (flag) {  // ��ʼͳ������
					ModelNum++;
					m_map[fileInfo.fileName()] = fileInfo.size();
				}
				else {		// ����ͨ������
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

//(Դ�ļ�Ŀ¼·����Ŀ���ļ�Ŀ¼���ļ������Ƿ񸲸�)
bool copyDirectory(const QString& srcPath, const QString& dstPath, bool coverFileIfExist)
{
	QDir srcDir(srcPath);
	QDir dstDir(dstPath);
	if (!dstDir.exists()) { //Ŀ���ļ�Ŀ¼�������򴴽��ļ�Ŀ¼
		if (!dstDir.mkdir(dstDir.absolutePath()))
			return false;
	}
	QFileInfoList fileInfoList = srcDir.entryInfoList();
	foreach(QFileInfo fileInfo, fileInfoList) {
		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;

		if (fileInfo.isDir()) {    // ��ΪĿ¼ʱ���ݹ�Ľ���copy
			if (!copyDirectory(fileInfo.filePath(), dstDir.filePath(fileInfo.fileName()), coverFileIfExist))
				return false;
		}
		else {            //�������ǲ���ʱ�������ļ�����ɾ������
			if (coverFileIfExist && dstDir.exists(fileInfo.fileName())) {
				dstDir.remove(fileInfo.fileName());
			}
			/// �����ļ�copy
			if (!QFile::copy(fileInfo.filePath(), dstDir.filePath(fileInfo.fileName()))) {
				return false;
			}
		}
	}
	return true;
}

//(Դ�ļ��ļ�·����Ŀ���ļ�·�����ļ������Ƿ񸲸�)
bool copyFile(QString srcPath, QString dstPath, bool coverFileIfExist)
{
	srcPath.replace("\\", "/");
	dstPath.replace("\\", "/");
	if (srcPath == dstPath) {
		return true;
	}

	if (!QFile::exists(srcPath)) {  //Դ�ļ�������
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

// ��ȡ�ļ�����
QString readFile(const QString& filePath) {
	QString content;

	// ���� QFile ����ָ���ļ�·��
	QFile file(filePath);

	// ���ļ����� ReadOnly ģʽ��
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		// ���� QTextStream ���󣬲��������ļ�����
		QTextStream in(&file);

		// ��ȡ�ļ���ȫ������
		content = in.readAll();

		// �ر��ļ�
		file.close();
	}
	else {
		// ���ļ�ʧ��
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
	// ����json �ļ�
	QString CIMStandard = m_pFilePathLineEdit->text() + "\\Schema\\CIMStandard.json";
	

	QFile BasicInfo(m_pFilePathLineEdit->text() + "\\BasicInfo.CIM");
	BasicInfo.open(QIODevice::ReadOnly | QIODevice::Text);
	QTextStream stream(&BasicInfo);
	QString string_value = stream.readAll();
	QJsonParseError jsonError;
	QJsonDocument doc = QJsonDocument::fromJson(string_value.toUtf8(), &jsonError);
	QJsonObject obj = doc.object();

	QString path_info = obj.value("�ۺ�ģ��·��").toArray().at(0).toObject().value("·��").toString();
	path_info = m_pFilePathLineEdit->text() + "\\SyntheticModel\\CIM3\\CIM3.json";

	QFile CodeFile(".\\ConceptCode.txt");
	if (!CodeFile.exists()) qDebug() << "����ȱ��ConceptCode.txt �ļ�\n";
	CodeFile.open(QIODevice::ReadOnly);
	//���ж�ȡ
	while (!CodeFile.atEnd())
	{
		//��ʽ1
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
	//���ļ����������򴴽�
	file.open(QIODevice::ReadWrite | QIODevice::Text);
	//д���ļ���Ҫ�ַ���ΪQByteArray��ʽ
	file.write(strJson.toUtf8());
	file.close();


	//Act(CIMStandard, path_info, "324226033101");

	// 	QString btnObj = ((QPushButton*)sender())->text(); //��ȡ��ťtxt����
	pProgressBar->setValue(0);//ִ�п�ʼʱ�ָ���ʼ״̬��
	auto buttonThis = (QPushButton*)sender();
	//buttonThis->blockSignals(true);

	//1 �����ж�(�����ַ��Ϊ��)
	if (m_pFilePathLineEdit->text().isEmpty() || m_pFilePathLineEditOUTPUT->text().isEmpty())
	{
		QWidget* PreViewWidget = new QWidget();
		PreViewWidget->setWindowTitle("����");
		PreViewWidget->setFixedSize(200, 100);
		QHBoxLayout* pHighLightayout = new QHBoxLayout();
		QLabel* pLabel1 = new QLabel(QString("�������ַ����ִ��!"));
		pHighLightayout->addWidget(pLabel1);
		PreViewWidget->setLayout(pHighLightayout);
		PreViewWidget->show();
		return;
	}

	//2 �������Ŀ¼
	buttonThis->setText("������...");
	buttonThis->setEnabled(false);

	//�ⲿ�����ַ������
	QString userPath = m_pFilePathLineEdit->text();
	//QString userPath = 

	// D:\CIM3_6\NNU-MiniCIM_05-14\CIMDataSet_05_14
	while (userPath.at(userPath.size() - 1) == "/" || userPath.at(userPath.size() - 1) == "\\")
		userPath.chop(1);//ɾ��ĩβб��
	QString inputPath = userPath + LOD3Path;//�����ַ
	QString outputFolder = userPath + LOD3Path;//д����ַ��ͬ����Ŀ¼��
											   //QString inputPath = "set inputPath=" + m_pFilePathLineEdit3EDPART->text();//�������ַ
											   //ui->playVideoButton->setEnabled(false);
	QDir outputPath(outputFolder);
	QString tempPath = exePath + "\\temp";//��ʱ�ļ���λ��
	QDir qDirWtsTemp = tempPath;//ɾ���ϴ����ɵ��м��ļ�
	qDirWtsTemp.removeRecursively();
	QString jsonFile = userPath + "\\SyntheticModel\\CIM3\\CIM3.json";


	// ��¼ģ�ͳɹ���������
	int SumModel = 0;
	int SuccessModel = 0;

	// ����ļ���ģ�͸���
	QString ModelFile = userPath + "\\SyntheticModel\\CIM3\\ExternalData\\building\\surface";
	FechFile(ModelFile, SumModel, 1);
	

	//��ɾ��ԭʼoutput�ļ��С�
	//	outputPath.removeRecursively();
	//	outputPath.mkdir(outputFolder);  // ʹ��mkdir�������ļ���

	//3 ���²�ִ��bat������cfg�ļ�
	//3.1 ���θ���bat
	string batPath = "./LOD3Process.bat";
	ifstream ifs(batPath);
	string batSTR;
	string temp;	//��¼��ǰ��������
	while (getline(ifs, temp))
	{
		istringstream iss(temp);	// ������-��ǰ��
		if (temp.find("set userPath=") <= 0)//�ҵ���һ���ַ���λ��
			temp = "set userPath=" + userPath.toStdString();
		else if (temp.find("set tempPath=") <= 0)
			temp = "set tempPath=" + tempPath.toStdString();
		else if (temp.find("set inputPath=") <= 0)
			temp = "set inputPath=" + inputPath.toStdString();
		else if (temp.find("set outputFolder=") <= 0)
			temp = "set outputFolder=" + outputFolder.toStdString();
		else if (temp.find("set jsonFile=") <= 0)
			temp = "set jsonFile=" + jsonFile.toStdString();//json��ַд��
															// 		if (temp.find("set inputPath=") <= 0)//�ҵ���һ���ַ���λ��
															// 			temp = "set inputPath=" + inputPath.toStdString();
															// 		else if (temp.find("set outputFolder=") <= 0)//�ҵ���һ���ַ���λ��
															// 			temp = "set outputFolder=" + outputFolder.toStdString();
		batSTR += temp + "\n";
	}
	ifs.close();
	ofstream ofs(batPath);
	ofs << batSTR;
	ofs.close();
	//3.2 ִ��bat����cfg
	QProcess* process = new QProcess();
	process->setWorkingDirectory("./");
	//QString commed = "./LOD3Process.bat";
	process->start(QString::fromStdString(batPath));
	process->waitForFinished(-1);
	pProgressBar->setValue(1);//������

							  //4 ��ʼ����˳��ִ�г���
							  //cout << QDir::currentPath().toStdString() << endl;
							  //QString exePath = QDir::currentPath();//��ȡ��ǰ���л���
							  //QString exePath = "H:\\LOD3\\LOD3Pack20240417\\exe";//�������ַ
	QTime qTime;
	qTime.start();
	string logs = "����ǽ�湹����������־��\n\n";
	QDateTime dateTime = QDateTime::currentDateTime();//��ȡϵͳ��ǰ��ʱ��
	logs += "��ʼʱ��: "+dateTime.toString("yyyy-MM-dd hh:mm:ss").toStdString() + "\n";//��ʽ��ʱ��
	QProcess program;
	QStringList args;
	//4.0 0formatOsgbObj
	QString exeFile = exePath + "\\0formatOsgbObj\\PO_formatOsgbObj.exe";
	QString cfgFolder = tempPath + "\\cfg";//����cfg�ļ���λ��
	QString cfgFile = cfgFolder + "\\0osgb2obj.cfg";
	args.append("--cfg");
	args.append(cfgFile);
	program.setWorkingDirectory(exePath + "\\0formatOsgbObj");//exe���л���
	program.start(exeFile, args);
	program.waitForFinished(-1);
	//��ʱͳ��
	double time0 = qTime.elapsed() / 1.0e3;
	//logs += "0formatOsgbObj:   " + to_string(time0) + "s\n";
	++SuccessModel;
	//����������
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
	//��ʱͳ��
	double time1 = qTime.elapsed() / 1.0e3;
	//logs += "1contour:         " + to_string(time1 - time0) + "s\n";
	++SuccessModel;
	//����������
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
	//��ʱͳ��
	double time2 = qTime.elapsed() / 1.0e3;
	//logs += "2LOD2:            " + to_string(time2 - time1) + "s\n";
	++SuccessModel;
	//����������
	QCoreApplication::processEvents();
	pProgressBar->setValue(4);
	//4.3 3render2file

	//test
	// 	QString exePath = QDir::currentPath();//��ȡ��ǰ���л���
	// 	QString outputFolder = m_pFilePathLineEditOUTPUT->text();//д����ַ
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

	//��ʱͳ��
	double time3 = qTime.elapsed() / 1.0e3;
	//logs += "3render2file:     " + to_string(time3 - time2) + "s\n";
	++SuccessModel;
	//����������
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
	//��ʱͳ��
	double time4 = qTime.elapsed() / 1.0e3;
	//logs += "4WallPicSeg:      " + to_string(time4 - time3) + "s\n";
	++SuccessModel;
	//����������
	QCoreApplication::processEvents();
	pProgressBar->setValue(6);
	//4.5 5samyolo
	//����exe��ִ��λ��
	QString yolosamEnv = m_pFilePathLineEdit3EDPART->text() + "/5yoloseg";
	copyDirectory(exePath + "/5yoloseg", yolosamEnv, true);
	exeFile = yolosamEnv + "\\window_door_seg.exe";
	cfgFile = cfgFolder + "\\5samyolo.cfg";
	args.pop_back();
	args.append(cfgFile);
	program.setWorkingDirectory(yolosamEnv);
	program.start(exeFile, args);
	program.waitForFinished(-1);
	//��ʱͳ��
	double time5 = qTime.elapsed() / 1.0e3;
	//logs += "5samyolo:         " + to_string(time5 - time4) + "s\n";
	++SuccessModel;
	//����������
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
	//��ʱͳ��
	double time6 = qTime.elapsed() / 1.0e3;
	//logs += "6layoutReasoning: " + to_string(time6 - time5) + "s\n";
	++SuccessModel;
	//����������
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
	//��ʱͳ��
	double time7 = qTime.elapsed() / 1.0e3;
	//logs += "7generateLOD3:    " + to_string(time7 - time6) + "s\n";
	++SuccessModel;
	//����������
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
	//��ʱͳ��
	double time8 = qTime.elapsed() / 1.0e3;
	//logs += "8textureSelect:   " + to_string(time8 - time7) + "s\n";
	++SuccessModel;
	//����������
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
	//��ʱͳ��
	double time9 = qTime.elapsed() / 1.0e3;
	//logs += "9buildingTexture: " + to_string(time9 - time8) + "s\n";
	++SuccessModel;
	//����������
	QCoreApplication::processEvents();
	pProgressBar->setValue(11);
	//4.10 10obj2osgb
	exeFile = exePath + "\\0formatOsgbObj\\PO_formatOsgbObj.exe";
	cfgFile = cfgFolder + "\\10obj2osgb.cfg";
	args.pop_back();
	args.append(cfgFile);
	program.setWorkingDirectory(exePath + "\\0formatOsgbObj");
	program.start(exeFile, args);//ע�⣺���һ���ĳ�start����̨�������
	program.waitForFinished(-1);
	//��ʱͳ��
	double time10 = qTime.elapsed() / 1.0e3;
	//logs += "10obj2osgb:       " + to_string(time10 - time9) + "s\n";
	++SuccessModel;
	//����������
	QCoreApplication::processEvents();
	pProgressBar->setValue(12);


	QDateTime dateTime2 = QDateTime::currentDateTime();
	int minutes = dateTime.secsTo(dateTime2);
	int time_ = dateTime.msecsTo(dateTime2);
	logs += "����ʱ��: " + dateTime.toString("yyyy-MM-dd hh:mm:ss").toStdString() + "\n";//��ʽ��ʱ��

	logs += "�ܺ�ʱ: " + minutes;
	logs += "��\n\n";

	//5 д��log
	//logs += "Total Time:       " + to_string(time10 - time0) + "s\n";

	logs += "��ģ������      " + to_string(SumModel) + "\n";
	int tol = SumModel;
	// �����ж���ģ�ͳɹ�
	FechFile(ModelFile, SumModel, 0);
		
	logs += "�ɹ�ģ������     " + to_string(SumModel) + "\n";
	logs += "ʧ��ģ������     " + to_string(tol - SumModel) + "\n";

	string logFile = exePath.toStdString() + "\\lod3.log";//log�������л���
	ofstream ofsb(logFile);
	ofsb << logs;
	ofsb.close();
	//6 ��β����
	//6.1 ɾ��yolosamexe
	QFile::remove(yolosamEnv + "/window_door_seg.exe");
	QFile::remove(yolosamEnv + "/temp.png");
	QFile::remove(yolosamEnv + "/traced_model.pt");
	QDir qDirWts(yolosamEnv + "/weights");
	qDirWts.removeRecursively();
	//qDirWts = tempPath;//ɾ��tempPath����Ϊ��ʼ��ʱɾ�������汣������鿴��
	//qDirWts.removeRecursively();

	//6.2 ��ť��ԭ
	buttonThis->setText("��ʼ����");
	buttonThis->setEnabled(true);
	//buttonThis->blockSignals(false);
}

void NightEffect::openLog()
{
	QString outputPath = exePath + "/lod3.log";//��ǰ��log�������л����С�
	QUrl url = QUrl::fromUserInput(outputPath, QDir::currentPath(), QUrl::AssumeLocalFile);
	QDesktopServices::openUrl(url);
	//QFileDialog::getOpenFileName(this, "open file", "./output/lod3.log");//��D��study�ļ���
}

/*
*@brief �����������ļ��У�ɸѡ���ض���ʽ���ļ���
*@param[in]inputFolderPath		��ǰѡ�еĵ���Ŀ�ꡣ
*@param[in]appendixStr			��Ҫɸѡ���ļ���׺�����磺".obj"
return	Ŀ���ʽ���ļ��б�
*/
vector<std::string> findAllFiles(string inputFolderPath, string appendixStr)
{
	//��ȫ���ж��ַ������һλ�Ƿ��ԡ�/����\\����β��
	auto a = inputFolderPath.back();
	if (a == '/' || a == '\\')
		inputFolderPath.pop_back();
	vector<std::string> childrenList;
	_finddata_t data;
	auto handle = _findfirst((inputFolderPath + "/*.*").c_str(), &data); //��ȡ��1�ļ����ļ���
	if (handle == -1)//�ж��Ƿ���Զ�ȡ�ļ�
	{
		cout << "can not read file!";
		_findclose(handle);// �ر��������
		return childrenList;
	}

	do
	{
		string s = data.name;
		if (data.attrib & _A_SUBDIR)//��Ŀ¼
		{
			if (s != "." && s != "..") //�ж��Ƿ�Ϊ"."��ǰĿ¼����".."��һ��Ŀ¼
			{
				vector<std::string> tempList = findAllFiles(inputFolderPath + "\\" + s, appendixStr);//�ݹ����
				childrenList.insert(childrenList.end(), tempList.begin(), tempList.end());
			}
		}
		else if (!(data.attrib & _A_SUBDIR)) //�������ļ�
		{
			if (s.rfind(appendixStr) == s.size() - appendixStr.size()) // �жϺ�׺�Ƿ�ΪappendixStr
			{
				childrenList.push_back(inputFolderPath + "\\" + s);
			}
		}
		else
			continue;

	} while (_findnext(handle, &data) == 0); //��ȡ��һ���ļ���

	_findclose(handle);// �ر��������
					   //cout << "�б�Ŀ���ļ�������" << childrenList.size() << endl;
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

//���ýű���osgviewer����ʾ�����Ŀ��
void NightEffect::openOsgviewer(QListWidgetItem *item)
{
	//1 ��ȡ�ļ���
	string name = item->text().toStdString();
	//2 ���Ҷ�Ӧ������ַ
	//��������osgb
	QString userPath = m_pFilePathLineEdit->text();
	while (userPath.at(userPath.size() - 1) == "/" || userPath.at(userPath.size() - 1) == "\\")
		userPath.chop(1);//ɾ��ĩβб��
	QString inputPath = userPath + LOD3Path;//�����ַ
	vector<std::string> osgbList = findAllFiles(inputPath.toStdString(), ".osgb");
	//QFileInfoList osgbList = FindFile(inputPath, ".osgb");
	QString osgbFilePath;
	for (int i = 0; i < osgbList.size(); i++)
	{
		if (osgbList[i].find(name) <= 1000)//�ҵ����踲�ǵ�osgb
		{
			osgbFilePath = QString::fromStdString(osgbList[i]);
			break;
		}
	}

	//3 ִ�нű�
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
	program.setWorkingDirectory(exePath);//exe���л���
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
	//this->setFixedSize(1000, 700);//�޸������ڴ�С
								  //this->setFixedSize(500, 350);//�޸������ڴ�С
	listWidget = new QListWidget(this);
	listWidget->setGeometry(5, 200, 990, 200);//(���ϵ�������߾��룬���ϵ�����ϱ߾��룬��ȣ��߶�)
											  //listWidget->setGeometry(5, 180, 490, 150);//(���ϵ�������߾��룬���ϵ�����ϱ߾��룬��ȣ��߶�)
											  //listWidget->resize(200, 400);

											  //��������osgb
	listWidget->setFixedSize(600, 300);
	QString userPath = m_pFilePathLineEdit->text();
	while (userPath.at(userPath.size() - 1) == "/" || userPath.at(userPath.size() - 1) == "\\")
		userPath.chop(1);//ɾ��ĩβб��
	QString inputPath = userPath + LOD3Path;//�����ַ
	vector<std::string> osgbList = findAllFiles(inputPath.toStdString(), ".osgb");
	//QFileInfoList osgbList = FindFile(inputPath, ".osgb");
	for (int i = 0; i < osgbList.size(); i++)
	{
		string osgbPath = osgbList[i];
		int index = max(osgbPath.find_last_of('/'), osgbPath.find_last_of('\\'));
		//std::string folderPath = osgbPath.substr(0, index);//"G:\Desktop\LOD3\LOD3SAutoExe\testdata\input\0buildingObj\CIM2a40"
		std::string filename = osgbPath.substr(index + 1, osgbPath.size() - index - 1);//CIM2a40.osgb
																					   //listWidget->addItem("C����������");
		QString osgbFile = QFileInfo(QString::fromStdString(osgbList[i])).fileName();
		listWidget->addItem(osgbFile);
	}
	listWidget->show();
	IsOpen = true;
	listWidget->setViewMode(QListView::ListMode);

	//���osgb�ļ���
	connect(listWidget, &QListWidget::itemClicked, this, &NightEffect::openOsgviewer);

	// 	//��һ�汾
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

	// �����ļ�
	QFile file("Setting.txt");

	file.open(QIODevice::WriteOnly);

	QTextStream out(&file);

	// д�����ݵ��ļ�
	out << str;

	// �ر��ļ�
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
	//ȥ���رմ���ʱ�����浯��
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
	exePath = QDir::currentPath();//��ʼ��exe��ַ
	exePath.replace("/", "\\");//bat�ű���ַ��ʶ����б��
	LOD3Path = "\\SyntheticModel\\CIM3\\ExternalData\\building\\surface";//��ʼ��LOD3��Ե�ַ

	QHBoxLayout* pHighLightayout = new QHBoxLayout();
	QLabel* pLabel1 = new QLabel(QString("����ģ�� "));
	pHighLightayout->addWidget(pLabel1);

	m_pFilePathLineEdit = new QLineEdit();
	m_pFilePathLineEdit->setObjectName(QString("File"));
	m_pFilePathLineEdit->setText("��ѡ������·����");


	QHBoxLayout* pHighLightayout2 = new QHBoxLayout();
	QLabel* pLabel2 = new QLabel(QString("���Ŀ¼ "));

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

	m_btn->setText("Ԥ��");
	m_btn->setFixedSize(60, 40);

	m_btn->setStyleSheet("QPushButton{background-color: rgb(225, 225, 225);border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}"
		"QPushButton:hover{background-color:rgb(229, 241, 251); color: black;}"
		"QPushButton:pressed{background-color:rgb(204, 228, 247);border-style: inset;}");

	

	connect(m_btn, &QPushButton::clicked, this, &NightEffect::SetFilePath);
	pHighLightayout->addWidget(m_btn);
	
	//     QPushButton* pBrowseFileButton = new QPushButton("...");
	//     pBrowseFileButton->setObjectName(QStringLiteral("FileBrowse"));
	//     pHighLightayout->addWidget(pBrowseFileButton);
	//     //���ε��س������ź�,�������ļ�
	//     pBrowseFileButton->setDefault(false);
	//     pBrowseFileButton->setAutoDefault(true);
	//     connect(pBrowseFileButton, SIGNAL(clicked()), this, SLOT(OnBrowseFileData()));

	// 	//xuek �ļ�д����ַ
	// 	QHBoxLayout* pHighLightayout3 = new QHBoxLayout();
	// 	//1���ַ���ʾ
	// 	QLabel* pLabelOUTPUT = new QLabel(QString("д����ַ��"));
	// 	pHighLightayout3->addWidget(pLabelOUTPUT);
	// 	//2����ַ����򣨲���ʾ��
	m_pFilePathLineEditOUTPUT = new QLineEdit();
	m_pFilePathLineEditOUTPUT->setObjectName(QString("File"));
	m_pFilePathLineEditOUTPUT->setText("H:\\LOD3\\LOD3Pack20240417\\testdata\\output");
	// 	pHighLightayout3->addWidget(m_pFilePathLineEditOUTPUT);
	// 	//3���ļ�������
	// 	QPushButton* pBrowseFileButtonOUTPUT = new QPushButton("...");
	// 	pBrowseFileButtonOUTPUT->setObjectName(QStringLiteral("FileBrowse"));
	// 	pHighLightayout3->addWidget(pBrowseFileButtonOUTPUT);
	// 	//���ε��س������ź�,�������ļ�
	// 	pBrowseFileButtonOUTPUT->setDefault(false);
	// 	pBrowseFileButtonOUTPUT->setAutoDefault(true);

	//xuek �������ַ
	// 	QHBoxLayout* pHighLightayout2 = new QHBoxLayout();
	// 	//1���ַ���ʾ
	// 	QLabel* pLabel3EDPART = new QLabel(QString("�����⣺  "));
	// 	pHighLightayout2->addWidget(pLabel3EDPART);
	// 	//2����ַ�����
	QDir *dirNow = new QDir(QDir::currentPath());
	dirNow->cdUp();//��ȡ��һ��Ŀ¼
	m_pFilePathLineEdit3EDPART = new QLineEdit();
	m_pFilePathLineEdit3EDPART->setObjectName(QString("File"));
	QString LOD3DLLPath = dirNow->path() + "/LOD3DLL";//������λ��д����
	m_pFilePathLineEdit3EDPART->setText(LOD3DLLPath);
	// 	pHighLightayout2->addWidget(m_pFilePathLineEdit3EDPART);
	// 	//3���ļ�������
	// 	QPushButton* pBrowseFileButton3EDPART = new QPushButton("...");
	// 	pBrowseFileButton3EDPART->setObjectName(QStringLiteral("FileBrowse"));
	// 	pHighLightayout2->addWidget(pBrowseFileButton3EDPART);
	// 	//���ε��س������ź�,�������ļ�
	// 	pBrowseFileButton3EDPART->setDefault(false);
	// 	pBrowseFileButton3EDPART->setAutoDefault(true);

	//����temp��ַ

	//����bat��·������ʼ��input��output��ַ
	ifstream ifs("./LOD3Process.bat");
	//string batSTR;
	string temp;	//��¼��ǰ��������
	while (getline(ifs, temp))
	{
		istringstream iss(temp);	// ������-��ǰ��
		if (temp.find("set userPath=") <= 0)//���� userPath
		{
			string userPath = temp.substr(13);
			//string LOD3Path = "\\SyntheticModel\\CIM3\\ExternalData\\building\\surface";
			m_pFilePathLineEdit->setText(QString::fromStdString(userPath));
			m_pFilePathLineEdit2->setText(QString::fromStdString(userPath));
			m_pFilePathLineEditOUTPUT->setText(QString::fromStdString(userPath) + LOD3Path);
			break;//�ҵ�����˳�
		}
		// 			m_pFilePathLineEdit->setText(QString::fromStdString(temp.substr(13)));
		// 		else if (temp.find("set outputFolder=") <= 0)//�ҵ���һ���ַ���λ��
		// 		{
		// 			m_pFilePathLineEditOUTPUT->setText(QString::fromStdString(temp.substr(17)));
		// 			break;//�ҵ�����˳�
	}
	ifs.close();

	//������
	QVBoxLayout* pProgressLayout = new QVBoxLayout();

	//1���ַ���ʾ
	//QLabel* pLabelPROGRESS = new QLabel(QString("������ȣ�"));
	//pProgressLayout->addWidget(pLabelPROGRESS);
	//2��������
	pProgressBar = new QProgressBar();
	//pProgressBar = new QProgressBar();
	pProgressBar->setObjectName(QStringLiteral("Progress Bar"));
	pProgressBar->setRange(0, 12);
	pProgressBar->setValue(0);
	pProgressLayout->addWidget(pProgressBar);
	pProgressBar->setAlignment(Qt::AlignCenter);


	//��Ӱ�ť
	QHBoxLayout* pProcessLayout = new QHBoxLayout();
	QPushButton* pProcessButton = new QPushButton("����");
	pProcessButton->setObjectName(QStringLiteral("Process"));
	pProcessLayout->addWidget(pProcessButton, 1);
	//pProcessButton->setGeometry(0, 0, 100, 25);
	pProcessButton->setFixedSize(125, 50);
	connect(pProcessButton, &QPushButton::clicked, this, &NightEffect::LOD3Process);

	pProcessButton->setStyleSheet("QPushButton{background-color: rgb(225, 225, 225);border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}"
		"QPushButton:hover{background-color:rgb(229, 241, 251); color: black;}"
		"QPushButton:pressed{background-color:rgb(204, 228, 247);border-style: inset;}");


	//connect(pProcessButton, SIGNAL(clicked()), this, SLOT(copyFileToDst()));
	//     //���ε��س������ź�,�������ļ�
	//     pProcessButton->setDefault(false);
	//     pProcessButton->setAutoDefault(true);
	//     pProcessButton->setFixedHeight(40);

	QPushButton* pPreViewButton = new QPushButton("Ԥ��...");
	pPreViewButton->setObjectName(QStringLiteral("view"));
	pProcessLayout->addWidget(pPreViewButton);
	pPreViewButton->setFixedSize(125, 50);
	connect(pPreViewButton, &QPushButton::clicked, this, &NightEffect::openOutput);
	//     //���ε��س������ź�,�������ļ�
	//     pPreViewButton->setDefault(false);
	//     pPreViewButton->setAutoDefault(true);
	//     pPreViewButton->setFixedHeight(40);
	//    // connect(pPreViewButton, SIGNAL(clicked()), this, SLOT(OnPreView()));
	pPreViewButton->setStyleSheet("QPushButton{background-color: rgb(225, 225, 225);border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}"
		"QPushButton:hover{background-color:rgb(229, 241, 251); color: black;}"
		"QPushButton:pressed{background-color:rgb(204, 228, 247);border-style: inset;}");

	QPushButton* pLogButton = new QPushButton("��־...");
	pLogButton->setObjectName(QStringLiteral("Log"));
	pProcessLayout->addWidget(pLogButton);
	pLogButton->setFixedSize(125, 50);
	connect(pLogButton, &QPushButton::clicked, this, &NightEffect::openLog);

	pLogButton->setStyleSheet("QPushButton{background-color: rgb(225, 225, 225);border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}"
		"QPushButton:hover{background-color:rgb(229, 241, 251); color: black;}"
		"QPushButton:pressed{background-color:rgb(204, 228, 247);border-style: inset;}");

	// �������ʵ��ͼƬ
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


	pic1Info->setText(tr("����ǰ(ʾ��)"));
	pic1Info->raise();

	pic2Info->setText(tr("�����(ʾ��)"));

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

	// ���ز�����ͼƬ�� QLabel
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

	//     //���ε��س������ź�,�������ļ�
	//     pLogButton->setDefault(false);
	//     pLogButton->setAutoDefault(true);
	//     pLogButton->setFixedHeight(40);


	// 	QListWidget * listout = new QListWidget;
	// 	listout->resize(200, 200);
	// 	pProcessLayout->addWidget(*listout);
	// 	QListWidget list_widget;
	// 	list_widget.setGeometry(50, 70, 50, 60);
	// 	list_widget.addItem("C����������");
	// 	list_widget.addItem("http://c.biancheng.net");
	// 	list_widget.addItem(new QListWidgetItem("Qt�̳�"));
	// 	list_widget.setLayoutMode(QListView::Batched);



	//������
	pMainLayerout->addLayout(pHighLightayout);
	pMainLayerout->addSpacing(20);

	pMainLayerout->addLayout(pHighLightayout2);
	pMainLayerout->addSpacing(40);
	// 	pMainLayerout->addLayout(pHighLightayout3);
	// 	pMainLayerout->addSpacing(10);
	// 	pMainLayerout->addLayout(pHighLightayout2);
	// 	pMainLayerout->addSpacing(10);


	// �������ͼƬ
	pMainLayerout->addLayout(pPicLayout);
	pMainLayerout->addSpacing(40);

	// ������
	pMainLayerout->addLayout(pProgressLayout);
	pMainLayerout->addSpacing(40);
	// ������ť
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
//     //ҹ�� ��ʼ������
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
//         //����µ�ͼ��NodeFeature
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
