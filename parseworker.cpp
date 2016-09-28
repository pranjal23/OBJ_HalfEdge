#include "parseworker.h"

ParseWorker::ParseWorker(QObject *parent) : QObject(parent)
{

}

QSharedPointer<PolygonMesh> sOutMesh;
class ParseThread : public QThread
{
public:
    ParseThread(QString fileName){
        sFileName = fileName;
    }

private:
    void run()
    {
        OBJFileParser mFileParser;
        sOutMesh = QSharedPointer<PolygonMesh>(mFileParser.parseFile( sFileName ));
        qDebug() << "Parse Complete";
    }
    QString sFileName;
};

void ParseWorker::parse()
{
    ParseThread* t = new ParseThread(mFileName);
    QObject::connect(t, SIGNAL(finished()), this, SLOT(parseDoneInThread()));
    t->start();
}

void ParseWorker::setFileName(QString fileName){
    mFileName = fileName;
}

void ParseWorker::parseDoneInThread(){
    emit parseComplete(sOutMesh);
}
