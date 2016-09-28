#include <QtWidgets>
#include "window.h"
#include "ui_window.h"
#include "trianglemesh.h"

#include "viewportwidget.h"
#include "mfileparser.h"

Window::Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);

    o_mesh = NULL;
    use_multi_threading = true;
    connect(this,SIGNAL(startParsing()),&mParseWorker,SLOT(parse()));
    connect(&mParseWorker,SIGNAL(parseComplete(QSharedPointer<PolygonMesh>)),this,SLOT(render(QSharedPointer<PolygonMesh>)));

    createActions();
    createMenus();
}

Window::~Window()
{
    delete ui;
}

void Window::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(e);
}

void Window::createActions()
{
    openAct = new QAction(tr("&Open"), this);
    openAct->setShortcuts(QKeySequence::New);
    openAct->setStatusTip(tr("Open a new 3D Mesh"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
}

void Window::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
}

//bool use_multi_threading = true;
PolygonMesh* out_mesh = NULL;
QLabel *lbl  = NULL;
void Window::open(){
    QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Open 3D Mesh"),
                QDir::currentPath(),
                tr("Wavefront (*.obj)") );
    if( !filename.isEmpty() )
    {
        lbl = new QLabel;
        lbl->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        QMovie *movie = new QMovie(":/images/loading.gif");
        lbl->setMovie(movie);
        lbl->setAttribute(Qt::WA_TranslucentBackground);
        lbl->setFixedSize(350,290);
        lbl->setWindowModality(Qt::ApplicationModal);
        lbl->setWindowFlags(Qt::FramelessWindowHint);
        lbl->show();
        movie->start();


        if(!use_multi_threading){

            OBJFileParser mFileParser;
            out_mesh = mFileParser.parseFile( filename );
            if(out_mesh!=NULL)
            {
                ui->viewPortWidget->triangleMesh = out_mesh;
                ui->viewPortWidget->updateGL();
            }
            else
            {
                qDebug() << "Mesh is NULL";
            }
            lbl->close();
        }
        else
        {

            mParseWorker.setFileName(filename);
            emit startParsing();

        }

    }
}

void Window::render(QSharedPointer<PolygonMesh> sp){
    PolygonMesh* sInMesh = sp.data();
    if(sInMesh!=NULL){

        ui->viewPortWidget->triangleMesh = sInMesh;
        ui->viewPortWidget->updateGL();
    }
    else
    {
        qDebug() << "Pointer location NULL";
    }
    lbl->close();
}

void Window::on_enableLightBtn_clicked(bool checked)
{
    ui->viewPortWidget->enableLight(checked);
    if(!checked)
    {
        ui->multilightsBtn->setChecked(false);
        ui->multilightsBtn->setEnabled(false);
        ui->lightSlider->setEnabled(false);
        ui->colorMatBtn->setChecked(false);
        ui->colorMatBtn->setEnabled(false);
    }
    else
    {
        ui->multilightsBtn->setEnabled(true);
        ui->colorMatBtn->setEnabled(true);
        ui->lightSlider->setEnabled(true);
    }
}

void Window::on_multilightsBtn_clicked(bool checked)
{
        ui->viewPortWidget->enableMultilights(checked);
}

void Window::on_colorMatBtn_clicked(bool checked)
{
    ui->viewPortWidget->enableColorMaterial(checked);
}

void Window::on_perspectiveBtn_clicked(bool checked)
{
        ui->orthographicBtn->setChecked(!checked);
        ui->viewPortWidget->setPerspectiveProjection(checked);
        if(!checked){
            ui->xyRb->setEnabled(true);
            ui->xzRb->setEnabled(true);
            ui->yzRb->setEnabled(true);
        } else {
            ui->xyRb->setEnabled(false);
            ui->xzRb->setEnabled(false);
            ui->yzRb->setEnabled(false);
        }
}

void Window::on_orthographicBtn_clicked(bool checked)
{
        ui->perspectiveBtn->setChecked(!checked);
        ui->viewPortWidget->setPerspectiveProjection(!checked);
        if(checked){
            ui->xyRb->setEnabled(true);
            ui->xzRb->setEnabled(true);
            ui->yzRb->setEnabled(true);
        } else {
            ui->xyRb->setEnabled(false);
            ui->xzRb->setEnabled(false);
            ui->yzRb->setEnabled(false);
        }
}

void Window::on_xyRb_toggled(bool checked)
{
    if(checked)
        ui->viewPortWidget->setOrthoView(ViewPortWidget::XY);
}

void Window::on_xzRb_toggled(bool checked)
{
    if(checked)
        ui->viewPortWidget->setOrthoView(ViewPortWidget::XZ);
}

void Window::on_yzRb_toggled(bool checked)
{
    if(checked)
        ui->viewPortWidget->setOrthoView(ViewPortWidget::YZ);
}

void Window::on_boundBoxBtn_clicked(bool checked)
{
    ui->viewPortWidget->showBoundingBox(checked);
}

void Window::on_groundBtn_clicked(bool checked)
{
    ui->viewPortWidget->showGround(checked);
}

void Window::on_axisBtn_clicked(bool checked)
{
    ui->viewPortWidget->showAxis(checked);
    if(checked)
        ui->axisLengthSlider->setEnabled(true);
    else
        ui->axisLengthSlider->setEnabled(false);
}

void Window::on_smoothShadingRb_toggled(bool checked)
{
    if(checked)
        ui->viewPortWidget->setRenderType(ViewPortWidget::SMOOTH_SHADING);
}

void Window::on_flatShadingRb_toggled(bool checked)
{
    if(checked)
        ui->viewPortWidget->setRenderType(ViewPortWidget::FLAT_SHADING);
}

void Window::on_wireframeRb_toggled(bool checked)
{
    if(checked)
        ui->viewPortWidget->setRenderType(ViewPortWidget::WIREFRAME);
}

void Window::on_pointsRb_toggled(bool checked)
{
    if(checked)
        ui->viewPortWidget->setRenderType(ViewPortWidget::POINTS);
}

void Window::on_axisLengthSlider_sliderMoved(int value)
{
    float height = value/10.0f;
    ui->viewPortWidget->setAxisHeight(height);
}

void Window::on_lightSlider_sliderMoved(int position)
{
    float z_position = position/2.0f;
    ui->viewPortWidget->setLightPosition(z_position);
}
