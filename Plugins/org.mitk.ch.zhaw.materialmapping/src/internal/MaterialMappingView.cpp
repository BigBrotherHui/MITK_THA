#include <algorithm>
#include <cmath>

#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <QMessageBox>
#include <QFileDialog>
#include <QShortcut>
#include <QtConcurrentRun>
#include <QWidget>
#include <mitkImage.h>
#include <tinyxml.h>

#include <vtkImageCast.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>

#include "MaterialMappingView.h"
#include "MaterialMappingHelper.h"
#include "WorkbenchUtils.h"
#include "GuiHelpers.h"
#include "MaterialMappingFilter.h"
#include "PowerLawWidget.h"

const std::string MaterialMappingView::VIEW_ID = "org.mitk.views.materialmapping";
Ui::MaterialMappingViewControls *MaterialMappingView::controls = nullptr;

MaterialMappingView::~MaterialMappingView() {
    if(m_WorkerFuture.isRunning()){
        QMessageBox::warning(0, "", "Material mapping is still processing data. Waiting for task to finish...");
        m_WorkerFuture.waitForFinished();
    }
}

void MaterialMappingView::CreateQtPartControl(QWidget *parent) {
    m_Controls.setupUi(parent);
    // table
    auto table = m_Controls.calibrationTableView;
    table->setModel(m_CalibrationDataModel.getQItemModel());
    auto setResizeMode = [=](int _column, QHeaderView::ResizeMode _mode) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0) // renamed in 5.0
        table->horizontalHeader()->setResizeMode(_column, _mode);
#else
        table->horizontalHeader()->setSectionResizeMode(_column, _mode);
#endif
    };
    setResizeMode(0, QHeaderView::Stretch);
    setResizeMode(1, QHeaderView::Stretch);

    // data selectors
    m_Controls.unstructuredGridComboBox->SetDataStorage(this->GetDataStorage());
    m_Controls.unstructuredGridComboBox->SetAutoSelectNewItems(false);
    m_Controls.unstructuredGridComboBox->SetPredicate(WorkbenchUtils::createIsUnstructuredGridTypePredicate());

    m_Controls.greyscaleImageComboBox->SetDataStorage(this->GetDataStorage());
    m_Controls.greyscaleImageComboBox->SetAutoSelectNewItems(false);
    m_Controls.greyscaleImageComboBox->SetPredicate(WorkbenchUtils::createIsImageTypePredicate());

    // testing
    if (TESTING) {
        controls = &m_Controls;
        m_Controls.testingGroup->show();
        m_Controls.expectedResultComboBox->SetDataStorage(this->GetDataStorage());
        m_Controls.expectedResultComboBox->SetAutoSelectNewItems(false);
        m_Controls.expectedResultComboBox->SetPredicate(WorkbenchUtils::createIsUnstructuredGridTypePredicate());
        m_Controls.expectedResultComboBox_2->SetDataStorage(this->GetDataStorage());
        m_Controls.expectedResultComboBox_2->SetAutoSelectNewItems(false);
        m_Controls.expectedResultComboBox_2->SetPredicate(WorkbenchUtils::createIsUnstructuredGridTypePredicate());

        m_TestRunner = std::unique_ptr<Testing::Runner>(new Testing::Runner());
        connect(m_Controls.runUnitTestsButton, SIGNAL(clicked()), m_TestRunner.get(), SLOT(runUnitTests()));
        connect(m_Controls.compareGridsButton, SIGNAL(clicked()), this, SLOT(compareGrids()));
        connect(m_Controls.createEMorganButton, SIGNAL(clicked()), this, SLOT(createEMorganImage()));
    } else {
        m_Controls.testingGroup->hide();
    }

    // hide custom erosion parameter
    m_Controls.uParamCheckBox->hide();
    m_Controls.label_15->hide();

    // hide custom dilation
    m_Controls.eParamSpinBox->hide();
    m_Controls.label->hide();

    // delete key on table
    QShortcut *shortcut = new QShortcut(QKeySequence(QKeySequence::Delete), table);
    connect(shortcut, SIGNAL(activated()), this, SLOT(deleteSelectedRows()));

    // power law widgets
    m_PowerLawWidgetManager = std::unique_ptr<PowerLawWidgetManager>(new PowerLawWidgetManager(m_Controls.powerLawWidgets));

    // signals
    connect(m_Controls.startButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));
    connect(m_Controls.saveParametersButton, SIGNAL(clicked()), this, SLOT(saveParametersButtonClicked()));
    connect(m_Controls.loadParametersButton, SIGNAL(clicked()), this, SLOT(loadParametersButtonClicked()));
    connect(&m_CalibrationDataModel, SIGNAL(dataChanged()), this, SLOT(tableDataChanged()));
    connect(m_Controls.addPowerLawButton, SIGNAL(clicked()), m_PowerLawWidgetManager.get(), SLOT(addPowerLaw()));
    connect(m_Controls.removePowerLawButton, SIGNAL(clicked()), m_PowerLawWidgetManager.get(), SLOT(removePowerLaw()));
    connect(m_Controls.unitSelectionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(unitSelectionChanged(int)));

    m_Controls.unitSelectionComboBox->setCurrentIndex(0);
    unitSelectionChanged(0);

    for(auto *widget : m_Controls.scrollAreaWidgetContents->findChildren<QWidget*>()){
        widget->installEventFilter(this);
        widget->setFocusPolicy(Qt::StrongFocus); // prevents wheel from setting the focus
    }
}

void MaterialMappingView::deleteSelectedRows() {
    auto selection = m_Controls.calibrationTableView->selectionModel();
    auto selectedItems = selection->selectedRows();
    std::set<int> rowsToDelete;

    for(auto &item : selectedItems) {
        rowsToDelete.insert(item.row());
    }

    for (std::set<int>::reverse_iterator rit = rowsToDelete.rbegin(); rit != rowsToDelete.rend(); ++rit) {
        m_CalibrationDataModel.removeRow(*rit);
    }

    tableDataChanged();
}

void MaterialMappingView::startButtonClicked() {
    MITK_INFO("ch.zhaw.materialmapping") << "processing input";
    if (isValidSelection()) {
        mitk::DataNode *imageNode = m_Controls.greyscaleImageComboBox->GetSelectedNode();
        mitk::DataNode *ugridNode = m_Controls.unstructuredGridComboBox->GetSelectedNode();

        mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(imageNode->GetData());
        mitk::UnstructuredGrid::Pointer ugrid = dynamic_cast<mitk::UnstructuredGrid *>(ugridNode->GetData());

        auto work = [this, image, ugrid]() {
            m_Controls.scrollArea->setEnabled(false);

//            auto filter = MaterialMappingFilter::New();
//            filter->SetInput(ugrid);
//            filter->SetMethod(gui::getSelectedMappingMethod(m_Controls));
//            filter->SetIntensityImage(image);
//            filter->SetDensityFunctor(gui::createDensityFunctor(m_Controls, m_CalibrationDataModel));
//            filter->SetPowerLawFunctor(m_PowerLawWidgetManager->createFunctor());
//            filter->SetDoPeelStep(m_Controls.uParamCheckBox->isChecked());
//            filter->SetNumberOfExtendImageSteps(m_Controls.eParamSpinBox->value());
//            filter->SetMinElementValue(m_Controls.fParamSpinBox->value());
//            auto result = filter->GetOutput();
//            filter->Update();

            auto result = MaterialMappingHelper::Compute(ugrid,
                                                         image,
                                                         gui::getSelectedMappingMethod(m_Controls),
                                                         gui::createDensityFunctor(m_Controls, m_CalibrationDataModel),
                                                         m_PowerLawWidgetManager->createFunctor(),
                                                         m_Controls.fParamSpinBox->value());

            mitk::DataNode::Pointer newNode = mitk::DataNode::New();
            newNode->SetData(result);

            // set some node properties
            newNode->SetProperty("name", mitk::StringProperty::New("material mapped mesh"));
            newNode->SetProperty("layer", mitk::IntProperty::New(1));

            // add result to the storage
            this->GetDataStorage()->Add(newNode);

            m_Controls.scrollArea->setEnabled(true);
        };

        m_WorkerFuture = QtConcurrent::run(static_cast<std::function<void()>>(work));
    }
}

void MaterialMappingView::tableDataChanged() {
    // in case new data was loaded from a file, we need to update the combo box
    int index = static_cast<int>(m_CalibrationDataModel.getUnit());
    m_Controls.unitSelectionComboBox->setCurrentIndex(index);

    auto linearEqParams = m_CalibrationDataModel.getFittedLine();
    m_Controls.linEQSlopeSpinBox->setValue(linearEqParams.slope);
    m_Controls.linEQOffsetSpinBox->setValue(linearEqParams.offset);

    if (m_CalibrationDataModel.hasExpectedValueRange()) {
        m_Controls.unitWarningLabel->hide();
    } else {
        m_Controls.unitWarningLabel->show();
    }
}

bool MaterialMappingView::isValidSelection() {
    // get the nodes selected
    mitk::DataNode *imageNode = m_Controls.greyscaleImageComboBox->GetSelectedNode();
    mitk::DataNode *ugridNode = m_Controls.unstructuredGridComboBox->GetSelectedNode();

    // set the mandatory field based on whether or not the nodes are NULL
    gui::setMandatoryQSSField(m_Controls.greyscaleSelector, (imageNode == nullptr));
    gui::setMandatoryQSSField(m_Controls.meshSelector, (ugridNode == nullptr));

    if (imageNode && ugridNode) {
        mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(imageNode->GetData());
        mitk::UnstructuredGrid::Pointer ugrid = dynamic_cast<mitk::UnstructuredGrid *>(ugridNode->GetData());

        if (image && ugrid) {
            return true;
        } else {
            QString msg("Invalid data. Select an image and a unstructured grid.");
            QMessageBox::warning(NULL, "Error", msg);
        }
    }
    MITK_INFO("ch.zhaw.materialmapping") << "invalid data selection";
    return false;
}

void MaterialMappingView::unitSelectionChanged(int) {
    auto selectedText = m_Controls.unitSelectionComboBox->currentText();
    m_CalibrationDataModel.setUnit(selectedText);
    tableDataChanged();
}

void MaterialMappingView::compareGrids() {
    mitk::DataNode *expectedResultNode0 = m_Controls.expectedResultComboBox->GetSelectedNode();
    mitk::DataNode *expectedResultNode1 = m_Controls.expectedResultComboBox_2->GetSelectedNode();
    mitk::UnstructuredGrid::Pointer u0 = dynamic_cast<mitk::UnstructuredGrid *>(expectedResultNode0->GetData());
    mitk::UnstructuredGrid::Pointer u1 = dynamic_cast<mitk::UnstructuredGrid *>(expectedResultNode1->GetData());
    m_TestRunner->compareGrids(u0, u1);
}

void MaterialMappingView::createEMorganImage() {
    mitk::DataNode *imageNode = m_Controls.greyscaleImageComboBox->GetSelectedNode();
    mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(imageNode->GetData());

    auto densityFunctor = gui::createDensityFunctor(m_Controls, m_CalibrationDataModel);
    auto powerLawFunctor = m_PowerLawWidgetManager->createFunctor();

    //// http://bugs.mitk.org/show_bug.cgi?id=5050
    auto mitkOrigin = image->GetGeometry()->GetOrigin();
    auto vtkImage = vtkSmartPointer<vtkImageData>::New();
    vtkImage->ShallowCopy(const_cast<vtkImageData *>(image->GetVtkImageData()));
    vtkImage->SetOrigin(mitkOrigin[0], mitkOrigin[1], mitkOrigin[2]);
    auto imageCast = vtkSmartPointer<vtkImageCast>::New();
    imageCast->SetInputData(vtkImage);
    imageCast->SetOutputScalarTypeToFloat();
    imageCast->Update();
    vtkImage = imageCast->GetOutput();

    for (auto i = 0; i < vtkImage->GetNumberOfPoints(); i++) {
        auto ct_val = vtkImage->GetPointData()->GetScalars()->GetTuple1(i);
        auto rho = densityFunctor(ct_val);
        auto val = powerLawFunctor(rho);
        vtkImage->GetPointData()->GetScalars()->SetTuple1(i, val);
    }

    // save results
    mitk::Image::Pointer result = mitk::Image::New();
    result->Initialize(vtkImage);
    result->SetVolume(vtkImage->GetScalarPointer());
    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
    newNode->SetData(result);
    newNode->SetProperty("name", mitk::StringProperty::New("emorgan"));
    newNode->SetProperty("layer", mitk::IntProperty::New(1));
    this->GetDataStorage()->Add(newNode);
}

void MaterialMappingView::saveParametersButtonClicked() {
    auto filename = QFileDialog::getSaveFileName(0, tr("Save parameter file"), "", tr("parameter file (*.matmap)"));
    if (!filename.isNull()) {
        MITK_INFO << "saving parameters to file: " << filename.toUtf8().constData();

        TiXmlDocument doc;
        auto root = new TiXmlElement("MaterialMapping");
        root->SetAttribute("Version", WorkbenchUtils::getGemVersion());
        auto calibration = m_CalibrationDataModel.serializeToXml();
        auto bonedensity = gui::serializeDensityGroupStateToXml(m_Controls);
        auto powerlaws = m_PowerLawWidgetManager->serializeToXml();
        auto options = gui::serializeOptionsGroupStateToXml(m_Controls);
        root->LinkEndChild(calibration);
        root->LinkEndChild(bonedensity);
        root->LinkEndChild(powerlaws);
        root->LinkEndChild(options);

        doc.LinkEndChild(new TiXmlDeclaration("1.0", "utf-8", ""));
        doc.LinkEndChild(root);
        doc.SaveFile(filename.toUtf8().constData());
    } else {
        MITK_INFO << "canceled file save dialog.";
    }
}

void MaterialMappingView::loadParametersButtonClicked() {
    auto filename = QFileDialog::getOpenFileName(0, tr("Open parameter file"), "", tr("parameter file (*.matmap)"));
    if (!filename.isNull()) {
        MITK_INFO << "loading parameters from file: " << filename.toUtf8().constData();

        TiXmlDocument doc(filename.toUtf8().constData());
        TiXmlHandle hDoc(&doc);
        if (!doc.LoadFile()) {
            QMessageBox::warning(0, "", "could not read from file.");
            return;
        };
        auto root = hDoc.FirstChildElement().Element();
        auto calibration = root->FirstChildElement("Calibration");
        if (calibration) {
            MITK_INFO << "loading calibration...";
            m_CalibrationDataModel.loadFromXml(calibration);
        }
        auto bonedensity = root->FirstChildElement("BoneDensityParameters");
        if (bonedensity) {
            MITK_INFO << "loading bone density parameters...";
            gui::loadDensityGroupStateFromXml(m_Controls, bonedensity);
        }
        auto powerlaws = root->FirstChildElement("PowerLaws");
        if (powerlaws) {
            MITK_INFO << "loading power laws...";
            m_PowerLawWidgetManager->loadFromXml(powerlaws);
        }
        auto options = root->FirstChildElement("Options");
        if (options) {
            MITK_INFO << "loading options...";
            gui::loadOptionsGroupStateFromXml(m_Controls, options);
        }
    } else {
        MITK_INFO << "canceled file open dialog.";
    }
}

bool MaterialMappingView::eventFilter(QObject *_obj, QEvent *_ev) {
    // ignores scroll wheel events on all spin boxes
    if(_ev->type() == QEvent::Wheel && qobject_cast<QAbstractSpinBox*>(_obj)){
        _ev->ignore();
        return true;
    }
    return QObject::eventFilter(_obj, _ev);
}
