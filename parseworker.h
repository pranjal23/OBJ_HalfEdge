#ifndef PARSEWORKER_H
#define PARSEWORKER_H

#include <QObject>

#include "mfileparser.h"

class ParseWorker : public QObject
{
    Q_OBJECT
public:
    explicit ParseWorker(QObject *parent = 0);

signals:
    void parseComplete(QSharedPointer<PolygonMesh>);

public slots:
    void parse();
    void parseDoneInThread();
    void setFileName(QString fileName);

private:
    QString mFileName;
};

#endif // PARSEWORKER_H
