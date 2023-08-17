#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <morph/qt/viswidget.h>
#include <morph/GraphVisual.h>
#include <morph/TriangleVisual.h>
#include <morph/HexGrid.h>
#include <morph/HexGridVisual.h>
#include <morph/ScatterVisual.h>
#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/vvec.h>

#include <QVBoxLayout>
#include <QFrame>
#include <QGridLayout>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->viswidget_init();

    // Call a function to set up a first VisualModel in the viswidget
    this->setupHexGridVisual();

    // Example timer to carry out computations. Here, a lambda function modifies the data that's visualised on the HexGrid
    auto timer = new QTimer(parent);
    connect(timer, &QTimer::timeout, [this]{
                                         for (unsigned int hi=0; hi<this->hg->num(); ++hi) {
                                             this->r[hi] = std::sqrt (hg->d_x[hi]*hg->d_x[hi] + hg->d_y[hi]*hg->d_y[hi]);
                                             this->data[hi] = std::sin(k*r[hi])/k*r[hi];
                                         }
                                         this->k += 0.02f;
                                         if (this->k > 8.0f) { this->k = 1.0f; }
                                         // Somehow access the pointer for the model. Like this:
                                         static_cast<morph::qt::viswidget*>(this->p_vw)->needs_reinit = 0;
                                         this->p_vw->update();
                                     });
    timer->start();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setupHexGridVisual()
{
    // First set up the HexGrid
    this->hg = std::make_unique<morph::HexGrid> (0.02f, 15.0f, 0.0f);
    this->hg->setCircularBoundary (4.0f);

    // Make some dummy data (a radially symmetric Bessel fn) to make an interesting surface
    this->data.resize (this->hg->num(), 0.0f);
    this->r.resize (this->hg->num(), 0.0f);
    this->k = 1.0f;
    for (unsigned int hi=0; hi<this->hg->num(); ++hi) {
        this->r[hi] = std::sqrt (hg->d_x[hi]*hg->d_x[hi] + hg->d_y[hi]*hg->d_y[hi]);
        this->data[hi] = std::sin(k*r[hi])/k*r[hi];
    }

    // Now create the HexGridVisual
    morph::vec<float, 3> offset = { 0.0f, -0.05f, 0.0f };
    auto hgv = std::make_unique<morph::HexGridVisual<float>>(hg.get(), offset);

    // In general, you need to bindmodel before calling finalize() (esp. for
    // VisualModels that do text, like a GraphVisual). This gives the VisualModel access
    // to shader progs from the Visual environment, and allows the VisualModel to know
    // its parent Visual.
    static_cast<morph::qt::viswidget*>(this->p_vw)->v.bindmodel (hgv);

    // Give the HexGridVisual access to the scalar data for the surface
    hgv->setScalarData (&this->data);

    // Now add the HexGridVisual model to newvisualmodels. It has to be cast to a plain morph::VisualModel first:
    std::unique_ptr<morph::VisualModel> vmp = std::move (hgv);
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

    auto gv = std::make_unique<morph::GraphVisual<double>> (this->graphlocn);
    // Bind the new (Graph)VisualModel to the morph::Visual associated with the viswidget
    static_cast<morph::qt::viswidget*>(this->p_vw)->v.bindmodel (gv);

    gv->twodimensional = false;
    morph::vvec<double> x;
    x.linspace (-1.5, 1.5, 25);
    gv->setdata (x, x.pow(2));

    // Cast and add
    std::unique_ptr<morph::VisualModel> vmp = std::move (gv);
    static_cast<morph::qt::viswidget*>(this->p_vw)->newvisualmodels.push_back (std::move(vmp));

    // request a render, otherwise it won't appear until user interacts with window
    this->p_vw->update();

    // Change the graphlocn so that the next graph shows up in a different place
    this->graphlocn[1] += 1.2f;
}
