#include <QtWidgets>
#include <QtOpenGL>
#include <QList>

#include "viewportwidget.h"

const static bool showDebug = false;

ViewPortWidget::ViewPortWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    triangleMesh=NULL;
    xAxisRotation = 0;
    yAxisRotation = 0;
    zAxisRotation = 0;
    use_lighting = true;
    multipleLights = true;
    color_material = true;
    eyeX = 0.0f;
    eyeY = 0.0f;
    eyeZ = -2.0f;
    mCurrentOrthoView = XY;
    m_showAxis = true;
    m_showBoundingBox = true;
    m_showGround = true;
    mCurrRenderType = SMOOTH_SHADING;
    perspective_projection = true;
    axis_height = 1.0f;
    light_distance = 1.0f;
}

ViewPortWidget::~ViewPortWidget()
{}

void ViewPortWidget::initializeGL()
{
    qglClearColor(Qt::gray);
}

void ViewPortWidget::resizeGL(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, width, height);
    setProjection();
}

void ViewPortWidget::setProjection()
{

    if(perspective_projection)
    {
        eyeX = 0.0f;
        eyeY = 0.0f;
        eyeZ = -5.0f;
        xAxisRotation = 22;
        yAxisRotation = 0;
        zAxisRotation = 0;
    }
    else
    {
        eyeX = 0.0f;
        eyeY = 0.0f;
        eyeZ = 0.0f;
        xAxisRotation = 0;
        yAxisRotation = 0;
        zAxisRotation = 0;
        switch(mCurrentOrthoView)
        {
        case XY:
            zAxisRotation = 0;
            eyeZ = -2.0f;
            break;
        case XZ:
            xAxisRotation = -90;
            eyeZ = 0.0f;
            break;
        case YZ:
            yAxisRotation = 90;
            eyeZ = 0.0f;
            break;
        default:
            zAxisRotation = 0;
            eyeZ = -2.0f;
            break;
        }
    }


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if(perspective_projection)
    {
        GLfloat aspect = GLfloat(width) / GLfloat(height ? height : 1);

        const GLfloat zNear = 0.001f, zFar = 1000.0f, fov = 45.0f;
        perspective(fov, aspect, zNear, zFar);
    }
    else
    {
        glOrtho(-1, +1, -1, +1, 0.0001, 100);
    }

}

void ViewPortWidget::paintGL()
{
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();    
    glTranslatef(0.0f, 0.0f, 0.0f);
    glTranslatef(eyeX, eyeY, eyeZ);
    draw();

    if(showDebug) {
        qDebug() << "X: " << eyeX << "Y: " << eyeY << "Z: " << eyeZ;
        qDebug() << "X_Rotation: " << xAxisRotation << "Y_Rotation: " << yAxisRotation << "Z-Rotation: " << zAxisRotation;
    }
}

void ViewPortWidget::mousePressEvent(QMouseEvent *event)
{
    if(showDebug)
        qDebug() << "Received mouse click";
    firstClickPosition = event->pos();
}

void ViewPortWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(showDebug)
        qDebug() << "Received mouse event";

    int dx = event->x() - firstClickPosition.x();
    int dy = event->y() - firstClickPosition.y();

    if((event->buttons() & Qt::LeftButton) && (event->buttons() & Qt::RightButton))
    {
        setZAxisRotation(zAxisRotation + dx);
    }
    else if(event->buttons() & Qt::LeftButton)
    {
        setXAxisRotation(xAxisRotation + dy);
        setYAxisRotation(yAxisRotation + dx);
    }
    else if(event->buttons() & Qt::RightButton)
    {
        changeCameraZoom(-dy);
    }
    else if(event->buttons() & Qt::MiddleButton)
    {
        changeCameraPositionOnXAxis(-dx);
        changeCameraPositionOnYAxis(-dy);
    }
    firstClickPosition = event->pos();
}

void ViewPortWidget::draw()
{
    glClear(GL_COLOR_BUFFER_BIT);
    qglColor(Qt::lightGray);

    setLightingParams();

    glRotatef(xAxisRotation, 1.0, 0.0, 0.0);
    glRotatef(yAxisRotation, 0.0, 1.0, 0.0);
    glRotatef(zAxisRotation, 0.0, 0.0, 1.0);

    drawObject();

    glDisable(GL_LIGHTING);

    if(m_showGround){
        drawPlane();
    }

    if(m_showAxis){
        drawAxis();
        drawCone();
    }
}

void ViewPortWidget::drawPlane(){
    glBegin(GL_LINES);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(0.5f, 0.5f, 0.8f);

    float x;
    for(x=-10; x<=10; x+=0.5){
        glVertex3f(-10.0, 0.0, 0.0+x);
        glVertex3f(10.0, 0.0, 0.0+x);

        glVertex3f(0.0+x, 0.0, -10.0);
        glVertex3f(0.0+x, 0.0, 10.0);
    }
    glPopMatrix();
    glEnd();
}

void ViewPortWidget::drawObject(){
    glEnable(GL_DEPTH_TEST);
    glPushMatrix();
    if(mCurrRenderType == SMOOTH_SHADING)
    {
        glShadeModel(GL_SMOOTH);
    }
    else if(mCurrRenderType == FLAT_SHADING)
    {
        glShadeModel(GL_SMOOTH);
    }
    else if(mCurrRenderType == WIREFRAME )
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }
    else if(mCurrRenderType == POINTS )
    {
        glPointSize(2.0f);
    }

    if(triangleMesh!=NULL)
    {

        std::vector<PolygonMesh::HE_edge>::iterator iv = triangleMesh->edgeVector->begin();
        while (iv != triangleMesh->edgeVector->end()) {

            drawFace(iv->next);
            ++iv;

            /*
            traverse_halfedge(iv->next);
            break;
            */
        }

        if(m_showBoundingBox){
            drawBoundingBox();
        }
    }

    if(mCurrRenderType == SMOOTH_SHADING)
    {

    }
    else if(mCurrRenderType == FLAT_SHADING)
    {

    }
    else if(mCurrRenderType == WIREFRAME )
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }
    else if(mCurrRenderType == POINTS )
    {
        glPointSize(1.0f);
    }
    glPopMatrix();
}

void ViewPortWidget::drawFace(PolygonMesh::HE_edge* edge){
    if(edge!=NULL && edge->face!=NULL){
        PolygonMesh::HE_face* face = edge->face;
        PolygonMesh::Normal* face_normal = face->normal;
        PolygonMesh::HE_edge* first_edge = nullptr;
        PolygonMesh::HE_edge* curr_edge = edge;


        if(mCurrRenderType == POINTS )
        {
            glBegin(GL_POINTS);
        } else {
            glBegin(GL_TRIANGLES);
        }

        glColor3f(0.5f,0.5f,0.5f);


        //qDebug() << "\n" << "face: " << face;
        while(curr_edge != first_edge)
        {
            //qDebug() << "edge: " << curr_edge;
            if(first_edge==nullptr)
            {
                first_edge = curr_edge;
            }

            PolygonMesh::HE_vert* vert = curr_edge->vert;
            if(mCurrRenderType == FLAT_SHADING)
            {
                glNormal3f(face_normal->x,face_normal->y,face_normal->z);
            }
            else if(mCurrRenderType == SMOOTH_SHADING)
            {
                glNormal3f(vert->normal->x,vert->normal->y,vert->normal->z);
            }
            glVertex3f(vert->x,vert->y,vert->z);

            curr_edge = curr_edge->next;
        }

       glEnd();
    }

    if (edge->pair == NULL) {
         if(showDebug)
            qDebug() << "      Current's' pair is null" << QString::number(edge->index);
    }
}

//Not used - only for testing
void ViewPortWidget::traverse_halfedge(PolygonMesh::HE_edge* edge){
    PolygonMesh::HE_edge* outgoing_he = edge;
    PolygonMesh::HE_edge* curr = outgoing_he;

    this->drawFace (curr->pair);
    if(showDebug) {
        qDebug() << "edge index: "<< QString::number(curr->index);
    }

    while (curr->pair != NULL && curr->pair->next != outgoing_he)
    {
        curr = curr->pair->next;
        if(showDebug) {
            qDebug() << "edge index: "<< QString::number(curr->index);
        }
        this->drawFace (curr->pair);
    }

    qDebug() << "Out of while loop: ";
    if (curr->pair == NULL) {
        if(showDebug) {
            qDebug() << "      Current's' pair is null";
        }
    }
    else if (curr->pair != NULL && curr->pair->next != outgoing_he) {
        qDebug() << "      Current's' pair is equal to outgoing_he";
    }

}

void ViewPortWidget::drawAxis()
{
    glBegin(GL_POLYGON);
    glPushMatrix();
    glLoadIdentity();
    glEnable(GL_BLEND);

    //draw line for z axis
    glColor3f(1.0, 0.0, 0.0);
    GLUquadricObj * q1 = gluNewQuadric();
    glTranslatef(0.0, 0.0, 0.0);
    //glRotatef(0, 1.0f, 0.0f, 0.0f);
    gluQuadricNormals(q1, GLU_SMOOTH);
    gluQuadricDrawStyle(q1, GLU_FILL);
    gluQuadricOrientation(q1, GLU_OUTSIDE);
    gluQuadricTexture(q1, GL_TRUE);
    gluCylinder(q1, 0.02, 0.02, axis_height, 16, 16);
    gluDeleteQuadric(q1);

    // draw line for x axis
    glColor3f(0.0, 0.0, 1.0);
    GLUquadricObj * q2 = gluNewQuadric();
    glTranslatef(0.0, 0.0 , 0.0);
    glRotatef(90, 0.0f, 1.0f, 0.0f);
    gluQuadricNormals(q2, GLU_SMOOTH);
    gluQuadricDrawStyle(q2, GLU_FILL);
    gluQuadricOrientation(q2, GLU_INSIDE);
    gluQuadricTexture(q2, GL_TRUE);
    gluCylinder(q2, 0.02, 0.02, axis_height, 16, 16);
    gluDeleteQuadric(q2);


    // draw line for y axis
    glColor3f(0.0, 1.0, 0.0);
    GLUquadricObj * q3 = gluNewQuadric();
    glTranslatef(0.0, 0.0 , 0.0);
    glRotatef(-90, 1.0f, 0.0f, 0.0f);
    gluQuadricNormals(q3, GLU_SMOOTH);
    gluQuadricDrawStyle(q3, GLU_FILL);
    gluQuadricOrientation(q3, GLU_INSIDE);
    gluQuadricTexture(q3, GL_TRUE);
    gluCylinder(q3, 0.02, 0.02, axis_height, 16, 16);
    gluDeleteQuadric(q3);
    glPopMatrix();

    //reset position
    glRotatef(90, 1.0f, 0.0f, 0.0f);
    glRotatef(-90, 0.0f, 1.0f, 0.0f);

    glDisable(GL_BLEND);
    glEnd();
}

void ViewPortWidget::drawCone()
{
    glBegin(GL_POLYGON);
    glPushMatrix();
    glLoadIdentity();
    glEnable(GL_BLEND);

    glColor3f(1.0, 0.0, 0.0);
    GLUquadricObj * q0 = gluNewQuadric();
    glRotatef(0, 1.0f, 0.0f, 0.0f);
    gluQuadricNormals(q0, GLU_SMOOTH);
    gluQuadricDrawStyle(q0, GLU_FILL);
    gluQuadricOrientation(q0, GLU_OUTSIDE);
    gluQuadricTexture(q0, GL_TRUE);
    gluCylinder(q0, 0.0004, 0.0, 0.5, 16, 16);
    gluDeleteQuadric(q0);

    //draw line for z axis
    glColor3f(1.0, 0.0, 0.0);
    GLUquadricObj * q1 = gluNewQuadric();
    glRotatef(0.0f, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, axis_height);
    gluQuadricNormals(q1, GLU_SMOOTH);
    gluQuadricDrawStyle(q1, GLU_FILL);
    gluQuadricOrientation(q1, GLU_OUTSIDE);
    gluQuadricTexture(q1, GL_TRUE);
    gluCylinder(q1, 0.1, 0.0, 0.5, 16, 16);
    gluDeleteQuadric(q1);

    // draw line for x axis
    glColor3f(0.0, 0.0, 1.0);
    glTranslatef(0.0f, 0.0f, 0.0f);
    GLUquadricObj * q2 = gluNewQuadric();
    glTranslatef(axis_height, 0.0f, -axis_height);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    gluQuadricNormals(q2, GLU_SMOOTH);
    gluQuadricDrawStyle(q2, GLU_FILL);
    gluQuadricOrientation(q2, GLU_INSIDE);
    gluQuadricTexture(q2, GL_TRUE);
    gluCylinder(q2, 0.1, 0.0, 0.5, 16, 16);
    gluDeleteQuadric(q2);

    // draw line for y axis
    glColor3f(0.0, 1.0, 0.0);
    GLUquadricObj * q3 = gluNewQuadric();
    glTranslatef(0.0f, axis_height, -axis_height);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluQuadricNormals(q3, GLU_SMOOTH);
    gluQuadricDrawStyle(q3, GLU_FILL);
    gluQuadricOrientation(q3, GLU_INSIDE);
    gluQuadricTexture(q3, GL_TRUE);
    gluCylinder(q3, 0.1, 0.0, 0.5, 16, 16);
    gluDeleteQuadric(q3);
    glPopMatrix();

    //reset position
    glRotatef(90, 1.0f, 0.0f, 0.0f);
    glRotatef(-90, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, -2.41f , 0.0f);

    glDisable(GL_BLEND);
    glEnd();

}

void ViewPortWidget::drawBoundingBox(){
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.3f, 0.3f);

    QVector3D* max = triangleMesh->maxVector;
    QVector3D* min = triangleMesh->minVector;

    glVertex3f(min->x(),min->y(),min->z());
    glVertex3f(-min->x(),min->y(),min->z());

    glVertex3f(min->x(),min->y(),min->z());
    glVertex3f(min->x(),-min->y(),min->z());

    glVertex3f(min->x(),min->y(),min->z());
    glVertex3f(min->x(),min->y(),-min->z());

    //maxes
    glVertex3f(max->x(),max->y(),max->z());
    glVertex3f(-max->x(),max->y(),max->z());

    glVertex3f(max->x(),-max->y(),max->z());
    glVertex3f(-max->x(),-max->y(),max->z());

    glVertex3f(max->x(),max->y(),-max->z());
    glVertex3f(-max->x(),max->y(),-max->z());

    glVertex3f(max->x(),max->y(),max->z());
    glVertex3f(max->x(),-max->y(),max->z());

    glVertex3f(-max->x(),max->y(),max->z());
    glVertex3f(-max->x(),-max->y(),max->z());

    glVertex3f(max->x(),max->y(),-max->z());
    glVertex3f(max->x(),-max->y(),-max->z());

    glVertex3f(max->x(),max->y(),max->z());
    glVertex3f(max->x(),max->y(),-max->z());

    glVertex3f(max->x(),-max->y(),max->z());
    glVertex3f(max->x(),-max->y(),-max->z());

    glVertex3f(-max->x(),max->y(),max->z());
    glVertex3f(-max->x(),max->y(),-max->z());

    glEnd();
}

void ViewPortWidget::setLightingParams(){
    glEnable(GL_NORMALIZE);
    if(color_material) {
        GLfloat materialColor[] = {0.5f, 0.5f, 0.5f, 0.30f};
        GLfloat materialSpecular[] = {0,0,1,1};
        GLfloat materialEmission[] = {0.1f,0.1f,0.1f, 0.1f};
        GLfloat shininess = 10;

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, materialEmission);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
        glEnable(GL_COLOR_MATERIAL);
    }
    else
    {
        GLfloat materialColor[] = {0.5f, 0.5f, 0.5f, 0.30f};
        GLfloat materialSpecular[] = {0,0,1,1};
        GLfloat materialEmission[] = {0.1f,0.1f,0.1f, 0.1f};
        GLfloat shininess = 0;

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, materialEmission);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
        glEnable(GL_COLOR_MATERIAL);
    }

    if(use_lighting)    {
        GLfloat light_Position[] = {1.0f, 1.0f, 0.0f + light_distance, 0.0f };
        GLfloat white_light[] = {1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat lmodel_ambient[] = {1.0f, 0.1f, 0.1f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, light_Position);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
        glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT,lmodel_ambient);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        if(multipleLights){
            GLfloat light_Position0[] = {1.0f, -10.0f, -5.0f, 0.0f };
            GLfloat white_light0[] = {0.1f, 0.1f, 0.4f, 0.1f };
            glLightfv(GL_LIGHT1, GL_POSITION, light_Position0);
            glLightfv(GL_LIGHT1, GL_DIFFUSE, white_light0);
            glLightfv(GL_LIGHT1, GL_SPECULAR, white_light0);
            glEnable(GL_LIGHT1);

            GLfloat light_Position[] = {3.0f, 10.0f, 3.0f, 0.0f };
            GLfloat white_light[] = {0.4f, 0.1f, 0.1f, 0.1f };
            glLightfv(GL_LIGHT3, GL_POSITION, light_Position);
            glLightfv(GL_LIGHT3, GL_DIFFUSE, white_light);
            glLightfv(GL_LIGHT3, GL_SPECULAR, white_light);
            glEnable(GL_LIGHT3);

            GLfloat light_Position2[] = {-5.0f, -10.0f, 5.0f, 0.0f };
            GLfloat white_light2[] = {0.1f, 0.2f, 0.1f, 0.7f };
            glLightfv(GL_LIGHT4, GL_POSITION, light_Position2);
            glLightfv(GL_LIGHT4, GL_DIFFUSE, white_light2);
            glEnable(GL_LIGHT4);
        }
        else
        {
            glDisable(GL_LIGHT1);
            glDisable(GL_LIGHT3);
            glDisable(GL_LIGHT4);
        }
    }
    else
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHT1);
        glDisable(GL_LIGHT3);
        glDisable(GL_LIGHT4);
    }
}

void ViewPortWidget::enableLight(bool checked)
{
    use_lighting = checked;
    updateGL();
}

void ViewPortWidget::enableMultilights(bool checked)
{
    multipleLights = checked;
    updateGL();
}

void ViewPortWidget::enableColorMaterial(bool checked) {
    color_material = checked;
    updateGL();
}

void ViewPortWidget::normalizeAngle(float &angle)
{
    while (angle < 0.0f)
        angle += 360.0f;
    while (angle > 360.0f)
        angle -= 360.0f;
}

void ViewPortWidget::normalizeMotion(float &x)
{
    x = x/10;
    float absD2x = fabs(x);
    if(absD2x>0.8)
        x=0.8*(x/absD2x);
}

void ViewPortWidget::normalizeZoom(float &x)
{
    x = x/10;
    float absD2x = fabs(x);
    if(absD2x>0.5)
        x=0.5*(x/absD2x);
}

void ViewPortWidget::setXAxisRotation(float angle)
{
    normalizeAngle(angle);
    xAxisRotation = angle;
    updateGL();
}

void ViewPortWidget::setYAxisRotation(float angle)
{
    normalizeAngle(angle);
    yAxisRotation = angle;
    updateGL();
}

void ViewPortWidget::setZAxisRotation(float angle)
{
    normalizeAngle(angle);
    zAxisRotation = angle;
    updateGL();
}

void ViewPortWidget::changeCameraPositionOnXAxis(float change)
{
    normalizeMotion(change);
    eyeX -= change;
    updateGL();
}

void ViewPortWidget::changeCameraPositionOnYAxis(float change)
{
    normalizeMotion(change);
    eyeY += change;
    updateGL();
}

void ViewPortWidget::changeCameraZoom(float change)
{
    normalizeZoom(change);
    QVector3D lookAt = change * getLookAtVector();

    eyeX += lookAt.x();
    eyeY += lookAt.y();
    eyeZ += lookAt.z();

    updateGL();
}

void ViewPortWidget::perspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
    GLdouble xmin, xmax, ymin, ymax;

    ymax = zNear * tan( fovy * M_PI / 360.0 );
    ymin = -ymax;
    xmin = ymin * aspect;
    xmax = ymax * aspect;

    glFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
}

void ViewPortWidget::setPerspectiveProjection(bool onOff)
{
    perspective_projection = onOff;
    setProjection();
    updateGL();
}

void ViewPortWidget::showGround(bool show)
{
    m_showGround = show;
    updateGL();
}

void ViewPortWidget::showAxis(bool show)
{
    m_showAxis = show;
    updateGL();
}

void ViewPortWidget::showBoundingBox(bool show)
{
    m_showBoundingBox = show;
    updateGL();
}

void ViewPortWidget::setRenderType(RENDER_TYPE type)
{
    mCurrRenderType = type;
    updateGL();
}

void ViewPortWidget::setOrthoView(ORTHO_VIEW_TYPE view)
{
    mCurrentOrthoView = view;
    setProjection();
    updateGL();
}

void ViewPortWidget::setAxisHeight(float height){
    axis_height = height;
    updateGL();
}

void ViewPortWidget::setLightPosition(float position){
    light_distance = position;
    updateGL();
}

QVector3D ViewPortWidget::getLookAtVector()
{
    GLfloat mat[16];
    glGetFloatv (GL_PROJECTION_MATRIX, mat);
    QVector3D lookAtV(mat[2],mat[6],mat[10]);

    return lookAtV;
}
