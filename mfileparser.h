#ifndef MFILEPARSER_H
#define MFILEPARSER_H

#include "trianglemesh.h"
#include "viewportwidget.h"
#include <QtWidgets>
#include <map>
#include <QMatrix4x4>

class OBJFileParser
{
public:
    OBJFileParser();
    PolygonMesh getTriangleMesh(QString fileName);
    PolygonMesh* parseFile(QString fileName);
    void scaleAndMoveToOrigin(QVector3D scaleV,
                              QVector3D transV,
                              QVector3D* vertV);
    PolygonMesh::Normal* calculateFaceNormal(PolygonMesh::HE_face* face);
    PolygonMesh::HE_vert* calculateFaceCentroid(PolygonMesh::HE_face* face);
    PolygonMesh::Normal* calculateVertexNormal(QList<PolygonMesh::HE_face*> faces);
};

#endif // MFILEPARSER_H
