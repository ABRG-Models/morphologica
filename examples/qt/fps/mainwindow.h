#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QOpenGLWidget>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#include <memory>
#include <morph/vvec.h>
#include <morph/HexGrid.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // In the constructor, a Qtimer is set up that does the animation of the Bessel function HexGrid
    MainWindow (QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // "actionQuit" is the name of the Quit action in the File menu. This is defined in
    // mainwindow.ui. This function will cause the application to exit.
    void on_actionQuit_triggered();

private:
    // Initialise your morph::viswidget. In this example, one widget is created and added to the UI.
    void viswidget_init();

    // A function which creates a HexGridVisual and adds it to viswidget's newvisualmodels stack.
    void setupHexGridVisual();

    // A pointer into your MainWindow UI
    Ui::MainWindow* ui = nullptr;
    // A pointer to your morph::qt::viswidget, which is a part of your overall Qt Window
    QOpenGLWidget* p_vw = nullptr;

    // The data you add to VisualModels has to be maintained in memory after you use
    // setScalarData() to add it to your VisualModels. That ensures that the information
    // is present when the VisualModel::finalize() function is called. This is also true
    // for the HexGrid that is used in the HexGridVisual that's created in the
    // constructor.
    morph::vvec<float> data;
    morph::vvec<float> r;
    float k = 0.0f;
    std::unique_ptr<morph::HexGrid> hg;

    // A location for a graph within the Visual scene inside the viswidget
    morph::vec<float> graphlocn = {1.5f, 0.0f, 0.0f};
};
#endif // MAINWINDOW_H
