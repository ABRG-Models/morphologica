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

namespace morph { class Visual; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow (QWidget* parent = nullptr);
    ~MainWindow();
    void viswidget_init();

private slots:
    void on_pushButton_clicked();

private:
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
    std::unique_ptr<morph::HexGrid> hg;

    // A function which creates a HexGridVisual and adds it to viswidget's newvisualmodels stack.
    void setupHexGridVisual();

    // A location for a graph within the Visual scene inside the viswidget
    morph::vec<float> graphlocn = {1.5f, 0.0f, 0.0f};
};
#endif // MAINWINDOW_H
