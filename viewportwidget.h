#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QVector3D>
#include "trianglemesh.h"
#ifdef _WIN32
    #include <Windows.h>
    #include <GL/glu.h>
#elif __APPLE__
    #include <OpenGL/glu.h>
#endif

class ViewPortWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit ViewPortWidget(QWidget *parent = 0);
    ~ViewPortWidget();
    PolygonMesh* triangleMesh;
    //Render types
    enum RENDER_TYPE{
        POINTS,
        WIREFRAME,
        FLAT_SHADING,
        SMOOTH_SHADING
    };
    enum ORTHO_VIEW_TYPE{
        XY,
        XZ,
        YZ
    };
    void setXAxisRotation(float angle);
    void setYAxisRotation(float angle);
    void setZAxisRotation(float angle);
    void enableLight(bool checked);
    void enableMultilights(bool checked);
    void enableColorMaterial(bool checked);
    void setPerspectiveProjection(bool onOff);
    void showGround(bool show);
    void showAxis(bool show);
    void showBoundingBox(bool show);
    void setOrthoView(ORTHO_VIEW_TYPE view);
    void setRenderType(RENDER_TYPE type);
    void setAxisHeight(float height);
    void setLightPosition(float position);
    void savePathPointsToJson(QString fileName);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void setLightingParams();
    void changeCameraPositionOnXAxis(float change);
    void changeCameraPositionOnYAxis(float change);
    void changeCameraZoom(float change);
    void perspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
    void setProjection();

private:
    void draw();
    void drawAxis();
    void drawCone();
    void drawPlane();
    void drawBoundingBox();
    void drawObject();
    void drawFace(PolygonMesh::HE_edge* edge);
    void traverse_halfedge(PolygonMesh::HE_edge* edge);
    void normalizeAngle(float &angle);
    void normalizeMotion(float &x);
    void normalizeZoom(float &x);
    QVector3D getLookAtVector();
    int xAxisRotation;
    int yAxisRotation;
    int zAxisRotation;
    bool use_lighting;
    bool multipleLights;
    bool color_material;
    float eyeX;
    float eyeY;
    float eyeZ;
    ORTHO_VIEW_TYPE mCurrentOrthoView;
    bool perspective_projection;
    qreal width;
    qreal height;
    QPoint firstClickPosition;
    RENDER_TYPE mCurrRenderType;
    bool m_showAxis;
    bool m_showGround;
    bool m_showBoundingBox;
    float axis_height;
    float light_distance;
};

#endif // MYGLWIDGET_H

