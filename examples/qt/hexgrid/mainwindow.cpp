#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <morph/qt/viswidget.h>
#include <morph/GraphVisual.h>
#include <morph/TriangleVisual.h>
#include <morph/HexGrid.h>
#include <morph/HexGridVisual.h>
#include <morph/ScatterVisual.h>
#include <morph/vec.h>
#include <morph/vvec.h>

#include <QVBoxLayout>
#include <QFrame>
#include <QGridLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->viswidget_init();

    // Call a function to set up a first VisualModel in the viswidget
    this->setupHexGridVisual();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setupHexGridVisual()
{
    // First set up the HexGrid
    this->hg = std::make_unique<morph::HexGrid> (0.01f, 3.0f, 0.0f);
    this->hg->setCircularBoundary (0.6f);

    // Make some dummy data (with sine waves) to make a non-flat surface
    this->data.resize (this->hg->num(), 0.0f);
    for (unsigned int ri=0; ri<this->hg->num(); ++ri) {
        this->data[ri] = 0.05f + 0.05f*std::sin(20.0f*hg->d_x[ri]) * std::sin(10.0f*hg->d_y[ri]) ; // Range 0->1
    }

    // Now create the HexGridVisual
    morph::vec<float, 3> offset = { 0.0f, -0.05f, 0.0f };
    auto hgv = std::make_unique<morph::HexGridVisual<float, morph::qt::gl_version>>(hg.get(), offset);

    // In general, you need to bindmodel before calling finalize() (esp. for
    // VisualModels that do text, like a GraphVisual). This gives the VisualModel access
    // to shader progs from the Visual environment, and allows the VisualModel to know
    // its parent Visual.
    static_cast<morph::qt::viswidget*>(this->p_vw)->v.bindmodel (hgv);

    // Give the HexGridVisual access to the scalar data for the surface
    hgv->setScalarData (&this->data);

    // Now add the HexGridVisual model to newvisualmodels. It has to be cast to a plain morph::VisualModel first:
    std::unique_ptr<morph::VisualModel<morph::qt::gl_version>> vmp = std::move (hgv);
    // The vector of VisualModels lives in viswidget, accessible via p_vw:
    static_cast<morph::qt::viswidget*>(this->p_vw)->newvisualmodels.push_back (std::move(vmp));
}

void MainWindow::viswidget_init()
{
    // Create widget. Seems to open in its own window with a new context.
    morph::qt::viswidget* vw = new morph::qt::viswidget (this->parentWidget());
    // Choose lighting effects if you want
    vw->v.lightingEffects();
    // Add the OpenGL widget to the UI.
    this->ui->verticalLayout->addWidget (vw);
    // Keep a copy of vw
    this->p_vw = vw;
}

void MainWindow::on_pushButton_clicked()
{
    std::cout << "Adding a GraphVisual...\n";

    auto gv = std::make_unique<morph::GraphVisual<double, morph::qt::gl_version>> (this->graphlocn);
    // Bind the new (Graph)VisualModel to the morph::Visual associated with the viswidget
    static_cast<morph::qt::viswidget*>(this->p_vw)->v.bindmodel (gv);

    gv->twodimensional = false;
    morph::vvec<double> x;
    x.linspace (-1.5, 1.5, 25);
    gv->setdata (x, x.pow(2));

    // Cast and add
    std::unique_ptr<morph::VisualModel<morph::qt::gl_version>> vmp = std::move (gv);
    static_cast<morph::qt::viswidget*>(this->p_vw)->newvisualmodels.push_back (std::move(vmp));

    // request a render, otherwise it won't appear until user interacts with window
    this->p_vw->update();

    // Change the graphlocn so that the next graph shows up in a different place
    this->graphlocn[1] += 1.2f;
}

void MainWindow::on_actionQuit_triggered() { close(); }
