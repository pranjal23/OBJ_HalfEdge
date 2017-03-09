#include "mfileparser.h"
#include <QString>

static const bool showDebug = false;

OBJFileParser::OBJFileParser()
{}

int getNumberOfDigits(long number)
{
    int digits = 0;
    if (number < 0) digits = 1;
    while (number) {
        number /= 10;
        digits++;
    }
    return digits;
}

struct TempFace
{
    long faceId;
    std::vector<long> vertexList;
};

QString debugPrintEdge(PolygonMesh::HE_edge* edge){
    return " : edge: " + QString::number(edge->index) + " , vertex: "
            + QString::number(edge->prev->vert->index)
            + " -> " + QString::number(edge->vert->index);
}

PolygonMesh OBJFileParser::getTriangleMesh(QString fileName){
    PolygonMesh* mesh = parseFile(fileName);
    return *mesh;
}

PolygonMesh* OBJFileParser::parseFile(QString fileName){
    if(showDebug)
        qDebug() << "Mesh " << fileName << " to be opened:\n";
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0,"error",file.errorString());
        return NULL;
    }

    if (!file.isReadable()) {
        QMessageBox::information(0,"Unable to read file %s, aborting\n", file.fileName().toStdString().c_str());
        return NULL;
    }

    PolygonMesh* mesh = new PolygonMesh();
    QMap<quint64,PolygonMesh::HE_vert*>* vertMap = new QMap<quint64,PolygonMesh::HE_vert*>();
    QMap<quint64,PolygonMesh::HE_face*>* faceMap = new QMap<quint64,PolygonMesh::HE_face*>();
    QMap<quint64,PolygonMesh::HE_edge*>* edgeMap = new QMap<quint64,PolygonMesh::HE_edge*>();

    QList<PolygonMesh::Normal*> normalList;
    QMap<quint64,quint64>* normalMap = new QMap<quint64,quint64>();

    QVector3D max = QVector3D(0.0,0.0,0.0);
    QVector3D min = QVector3D(0.0,0.0,0.0);

    QMultiMap<quint64,PolygonMesh::HE_face*>* vert2faceMap = new QMultiMap<quint64,PolygonMesh::HE_face*>();


    QList<TempFace*>* faceDataList = new QList<TempFace*>();

    unsigned long vertex_count=1;
    unsigned long face_count=1;
    unsigned long vertex_normal_count=1;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();

        /// processing
        QString data = line.trimmed();
        if(!data.startsWith("#"))
        {
            QStringList parts = data.split(QRegExp("\\s+"));

            if(((QString)parts.at(0)).toLower()=="v")
            {
                PolygonMesh::HE_vert* vert = new PolygonMesh::HE_vert();
                bool ok;
                vert->index = vertex_count;
                vert->x = ((QString)parts.at(1)).toFloat(&ok);
                vert->y = ((QString)parts.at(2)).toFloat(&ok);
                vert->z = ((QString)parts.at(3)).toFloat(&ok);

                //max and min vectors
                max.setX(qMax(max.x(),vert->x));
                max.setY(qMax(max.y(),vert->y));
                max.setZ(qMax(max.z(),vert->z));
                min.setX(qMin(min.x(),vert->x));
                min.setY(qMin(min.y(),vert->y));
                min.setZ(qMin(min.z(),vert->z));

                if(ok)
                {
                    vertMap->insert(vert->index,vert);
                    if(mesh->first_vertex==NULL){
                        mesh->first_vertex = vert;
                    }
                    vertex_count++;
                }
                else
                {
                    QMessageBox::information(0,"Unable to read file, error in type conversion %s, aborting\n", file.fileName().toStdString().c_str());
                    return NULL;
                }
            }

            if(((QString)parts.at(0)).toLower()=="vn")
            {
                PolygonMesh::Normal* normal = new PolygonMesh::Normal();
                bool ok;
                normal->x = ((QString)parts.at(1)).toFloat(&ok);
                normal->y = ((QString)parts.at(2)).toFloat(&ok);
                normal->z = ((QString)parts.at(3)).toFloat(&ok);

                normalList.push_back(normal);
                vertex_normal_count++;
            }

            if(((QString)parts.at(0)).toLower()=="f")
            {
                if(showDebug)
                    qDebug() <<"\n" << "Face: " ;
                int part_count;
                long index = face_count;
                TempFace* facedata = new TempFace();
                facedata->faceId = index;
                for(part_count=1; part_count<parts.size();part_count++)
                {
                    QString facepart = (QString)parts.at(part_count);
                    QStringList fparts = facepart.trimmed().split(QRegExp("\\/"));

                    int fpart_count;
                    long vertex_id = -1;
                    for(fpart_count=0; fpart_count<fparts.size();fpart_count++)
                    {
                        if(fpart_count==0)
                        {
                            QString v_str = ((QString)fparts.at(fpart_count));
                            bool ok;
                            vertex_id = v_str.toLong(&ok,10);
                            if(ok)
                            {
                                if(showDebug)
                                    qDebug() << "Vertex Id: " << vertex_id ;
                                facedata->vertexList.push_back(vertex_id);
                            }
                        }

                        if(vertex_id!=-1 && fpart_count==2)
                        {
                            //Vertex normals
                            QString vn_str = fparts.at(fpart_count);
                            if(!vn_str.isEmpty())
                            {
                                bool ok2;
                                long normal_id = vn_str.toLong(&ok2,10);
                                if(ok2)
                                {
                                    normalMap->insert(vertex_id,normal_id);
                                }
                            }
                        }
                    }

                }
                faceDataList->append(facedata);
                face_count++;
            }
        }
    }
    file.close();


    //Find the translation vector
    float mx = (max.x()+min.x())/2;
    float my = (max.y()+min.y())/2;
    float mz = (max.z()+min.z())/2;
    QVector3D tr(-mx,-my,-mz);

    //Find the scale vector
    float lx = (max.x()-min.x());
    float ly = (max.y()-min.y());
    float lz = (max.z()-min.z());
    float ll = sqrt((lx*lx) + (ly*ly) + (lz*lz)) / 2;
    if(ll==0){ll=1;}//check divide by zero just in case
    float a=1/ll;
    QVector3D sr(a,a,a);

    //Normalize vertexes and move to origin
    QMap<quint64,PolygonMesh::HE_vert*>::iterator i = vertMap->begin();
    while (i != vertMap->end()) {
        PolygonMesh::HE_vert* vert = i.value();
        QVector3D* vertV = new QVector3D(vert->x,vert->y,vert->z);
        scaleAndMoveToOrigin(sr,tr,vertV);
        vert->x = vertV->x();
        vert->y = vertV->y();
        vert->z = vertV->z();
        delete vertV;
        ++i;
    }

    //Normalize and assign the max vector
    mesh->maxVector->setX(max.x());
    mesh->maxVector->setY(max.y());
    mesh->maxVector->setZ(max.z());
    scaleAndMoveToOrigin(sr,tr,mesh->maxVector);

    //Normalize and assign the min vector
    mesh->minVector->setX(min.x());
    mesh->minVector->setY(min.y());
    mesh->minVector->setZ(min.z());
    scaleAndMoveToOrigin(sr,tr,mesh->minVector);


    int power_factor = getNumberOfDigits(vertex_count);
    long units = pow(10,power_factor) * 1000;
    long edgeCount=0;

    QListIterator<TempFace*> iter(*faceDataList);
    while(iter.hasNext())
    {
        TempFace* facedata = iter.next();
        long index = facedata->faceId;

        if(showDebug)
            qDebug() <<"\n" << "Face id: " << index << ", number of vertices in face: " << facedata->vertexList.size();

        QList<PolygonMesh::HE_vert*> vList;
        unsigned long vid_count;
        for(vid_count=0;vid_count<facedata->vertexList.size();vid_count++)
        {
            PolygonMesh::HE_vert* vert = vertMap->value(facedata->vertexList.at(vid_count),NULL);
            if(vert!=NULL)
            {
                vList.push_back(vertMap->value(facedata->vertexList.at(vid_count),NULL));
            }
        }

        if(showDebug)
            qDebug() <<"\n" << "Number of HE_vert in list: " << vList.size();

        PolygonMesh::HE_face* face = new PolygonMesh::HE_face();
        //Create the face
        face->index = index;

        PolygonMesh::HE_edge* first_edge=nullptr;
        PolygonMesh::HE_edge* prev_edge=nullptr;

        int vert_count;
        for(vert_count=0;vert_count<vList.size();vert_count++)
        {
            //retreive vertexes
            PolygonMesh::HE_vert* vertex_one = vList.at(vert_count);
            vert2faceMap->insertMulti(vertex_one->index,face);

            PolygonMesh::HE_vert* vertex_two;
            if(vert_count<vList.size()-1)
            {
                vertex_two = vList.at(vert_count+1);
            }
            else
            {
                vertex_two = vList.at(0);
            }

            //create half-edge
            PolygonMesh::HE_edge* curr_edge = new PolygonMesh::HE_edge();
            if(vert_count==0)
            {
                first_edge = curr_edge;
            }

            curr_edge->vert = vertex_two;
            curr_edge->index = ++edgeCount;

            if(prev_edge!=nullptr)
            {
                prev_edge->next=curr_edge;
            }
            curr_edge->prev = prev_edge;

            if(vert_count == vList.size()-1)
            {
                first_edge->prev = curr_edge;
                curr_edge->next = first_edge;
            }

            vertex_one->edge = curr_edge;

            prev_edge = curr_edge;
            quint64 edge_key1 =  vertex_one->index * units + vertex_two->index;
            edgeMap->insert(edge_key1,curr_edge);

            curr_edge->face = face;
            face->edge = curr_edge;


            //pairing curr edge
            quint64 pair_key =  vertex_two->index * units + vertex_one->index;
            PolygonMesh::HE_edge* pair1 = edgeMap->take(pair_key);
            if(pair1!=NULL){
                PolygonMesh::HE_edge* pairEdge = pair1;
                if(showDebug)
                    qDebug() << "Pair found: Current edge- " << curr_edge->index <<  " ,Pair edge- " << pairEdge->index;
                curr_edge->pair = pairEdge;
                pairEdge->pair = curr_edge;
                edgeMap->insert(pair_key,pairEdge);
            }
        }

        faceMap->insert(face->index,face);
    }


    //Assign surface normal
    QMap<quint64,PolygonMesh::HE_face*>::iterator iv = faceMap->begin();
    while (iv != faceMap->end()) {
        iv.value()->normal = calculateFaceNormal(iv.value());
        iv.value()->centroid = calculateFaceCentroid(iv.value());
        ++iv;
    }

    //Assign vertex normals
    if(normalMap->size()==0 || normalMap->size()!=vertMap->size())
    {
        qDebug() << "Calculating vertex normals";
        QMap<quint64,PolygonMesh::HE_vert*>::iterator im = vertMap->begin();
        while (im != vertMap->end()) {
            quint64 vertIndex = im.value()->index;
            QList<PolygonMesh::HE_face*> faces = vert2faceMap->values(vertIndex);
            im.value()->normal = calculateVertexNormal(faces);
            ++im;
        }
    }
    else
    {
        qDebug() << "Pre-calculated vertex normals";
        QMap<quint64,PolygonMesh::HE_vert*>::iterator im = vertMap->begin();
        while (im != vertMap->end()) {

            long normal_id = (long)normalMap->value(im.value()->index,-1);
            if(normal_id!=-1 && normal_id<=normalList.size())
            {
                im.value()->normal = normalList.at(normal_id-1);//As Obj's normal id's start from 1 but normalList start from 0
            }
            else
            {
                qDebug() << "Normal list size smaller " << normalList.size() << " than " << normal_id;
            }
            ++im;
        }
    }


    QMap<quint64,PolygonMesh::HE_edge*>::iterator ig = edgeMap->begin();
    while (ig != edgeMap->end()) {
        PolygonMesh::HE_edge* edge = ig.value();
        mesh->edgeVector->push_back(*edge);
        ++ig;
    }

    return mesh;
}

void OBJFileParser::scaleAndMoveToOrigin(
        QVector3D scaleV,
        QVector3D transV,
        QVector3D* vertV)
{

    //Convert to left hand coordinate system for unity
    QVector3D ov(vertV->x(),vertV->y(),vertV->z());
    QMatrix4x4 A(1.0,        0.0,        0.0,        0,
                 0.0,        1.0,        0.0,        0,
                 0.0,        0.0,        -1.0,        0,
                 0.0,        0.0,        0.0,        1.0);
    A.rotate(180,0.0,1.0,0.0);

    QVector3D fv = A * ov;

    vertV->setX(fv.x());
    vertV->setY(fv.y());
    vertV->setZ(fv.z());
    return;

    /*
    //Apply translation and scale to the point

    QMatrix4x4 A(1.0,        0.0,        0.0,        transV.x(),
                 0.0,        1.0,        0.0,        transV.y(),
                 0.0,        0.0,        1.0,        transV.z(),
                 0.0,        0.0,        0.0,        1.0);

    QMatrix4x4 B(scaleV.x(), 0.0,        0.0,        0.0,
                 0.0,        scaleV.y(), 0.0,        0.0,
                 0.0,        0.0,        scaleV.z(), 0.0,
                 0.0,        0.0,        0.0,        1.0);

    QVector3D ov(vertV->x(),vertV->y(),vertV->z());
    QVector3D tv = A * ov;
    QVector3D fv = B * tv;

    if(showDebug)
        qDebug() << "original: " << vertV->x() << "," << vertV->y() << ","  << vertV->z();

    vertV->setX(fv.x());
    vertV->setY(fv.y());
    vertV->setZ(fv.z());

    if(showDebug)
        qDebug() << "transformed: " << fv.x() << ","  << fv.y() << ","  << fv.z();
    */
}

/**
 * @brief calculateFaceNormal
 * @param TriangleMesh::HE_face
 * @return TriangleMesh::HE_normal
 */
PolygonMesh::Normal* OBJFileParser::calculateFaceNormal(PolygonMesh::HE_face* face)
{
    PolygonMesh::HE_vert* p1 = face->edge->vert;
    PolygonMesh::HE_vert* p2 = face->edge->next->vert;
    PolygonMesh::HE_vert* p3 = face->edge->prev->vert;

    PolygonMesh::HE_vert U;
    U.x = p2->x - p1->x;
    U.y = p2->y - p1->y;
    U.z = p2->z - p1->z;

    PolygonMesh::HE_vert V;
    V.x = p3->x - p1->x;
    V.y = p3->y - p1->y;
    V.z = p3->z - p1->z;

    PolygonMesh::Normal* normal = new PolygonMesh::Normal();
    normal->x = (U.y*V.z) - (U.z*V.y);
    normal->y = (U.z*V.x) - (U.x*V.z);
    normal->z = (U.x*V.y) - (U.y*V.x);

    return normal;
}

/**
 * @brief calculateFaceCentroid
 * @param TriangleMesh::HE_face
 * @return TriangleMesh::HE_vert
 */
PolygonMesh::HE_vert* OBJFileParser::calculateFaceCentroid(PolygonMesh::HE_face* face)
{
    PolygonMesh::HE_vert* p1 = face->edge->vert;
    PolygonMesh::HE_vert* p2 = face->edge->next->vert;
    PolygonMesh::HE_vert* p3 = face->edge->prev->vert;

    PolygonMesh::HE_vert* centroid = new PolygonMesh::HE_vert();
    centroid->x = (p3->x + p2->x + p1->x)/3;
    centroid->y = (p3->y + p2->y + p1->y)/3;
    centroid->z = (p3->z + p2->z + p1->z)/3;

    return centroid;
}


/**
 * @brief calculateVertexNormal
 * @param QList<TriangleMesh::HE_face*>
 * @return TriangleMesh::HE_normal
 */
PolygonMesh::Normal* OBJFileParser::calculateVertexNormal(QList<PolygonMesh::HE_face*> faces){
    PolygonMesh::Normal* normal = new PolygonMesh::Normal();
    normal->x = 0.0f;
    normal->y = 0.0f;
    normal->z = 0.0f;

    float numOfFaces =  (float)faces.size();
    QListIterator<PolygonMesh::HE_face*> iter(faces);
    while(iter.hasNext())
    {
        PolygonMesh::Normal* faceNormal = iter.next()->normal;
        normal->x += faceNormal->x;
        normal->y += faceNormal->y;
        normal->z += faceNormal->z;
    }

    normal->x = normal->x/numOfFaces;
    normal->y = normal->y/numOfFaces;
    normal->z = normal->z/numOfFaces;

    return normal;
}

