#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include <QList>
#include <QMap>
#include <vector>
#include <QVector3D>
#ifdef _WIN32
    #include <Windows.h>
    #include <GL/glu.h>
#elif __APPLE__
    #include <OpenGL/glu.h>
#endif

class PolygonMesh
{
public:
    explicit PolygonMesh();
    ~PolygonMesh();
    struct HE_edge;
    struct HE_vert;
    struct HE_face;
    struct Normal;
    struct Normal {
        float x, y, z;
    };
    struct HE_edge {
        long index;
        HE_vert* vert;
        HE_edge* pair;
        HE_face* face;
        HE_edge* prev;
        HE_edge* next;
    };
    struct HE_vert {
        long index;
        float x, y, z;
        HE_edge* edge;
        Normal* normal;
    };
    struct HE_face {
        long index;
        HE_edge* edge;
        Normal* normal;
    };
    HE_vert* first_vertex;
    std::vector<HE_edge>* edgeVector;
    void copyEdge(HE_edge* in, HE_edge* out);

    //Max and Min X,Y,Z positions to draw bounding box
    QVector3D* maxVector;
    QVector3D* minVector;
};

#endif // TRIANGLEMESH_H
