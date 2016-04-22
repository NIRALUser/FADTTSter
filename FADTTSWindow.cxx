#include "FADTTSWindow.h"
#include "ui_FADTTSWindow.h"

#include <QDebug>


/****************************************************************/
/******************** Configuration && Events *******************/
/****************************************************************/
const QColor FADTTSWindow::m_green = QColor( 0,255,0,127 );
const QColor FADTTSWindow::m_red = QColor( 255,0,0,127 );
const QColor FADTTSWindow::m_grey = QColor( 220,220,220,255 );
const QColor FADTTSWindow::m_yellow = QColor( 255,255,0,127 );
const QColor FADTTSWindow::m_lightBlack = QColor( 0,0,0,191 );

const QString FADTTSWindow::m_csvSeparator = QLocale().groupSeparator();

const int FADTTSWindow::m_iconSize = 12;


/********************** Public  functions ***********************/
FADTTSWindow::FADTTSWindow( QWidget *parent, Qt::WindowFlags f ) :
    QMainWindow( parent, f )
{
    this->setupUi( this );

    m_okPixmap = QPixmap( ":/Icons/Resources/Icons/okIcon.xpm" );
    m_koPixmap = QPixmap( ":/Icons/Resources/Icons/koIcon.xpm" );
    m_warningPixmap = QPixmap( ":/Icons/Resources/Icons/warningIcon.xpm" );

    InitFADTTSWindow();
}

FADTTSWindow::~FADTTSWindow()
{
    m_editInputDialog.clear();
}


void FADTTSWindow::LoadParaConfiguration( QString filename )
{
    QString text;
    QFile refJSON( filename );
    refJSON.open( QIODevice::ReadOnly | QIODevice::Text );
    text = refJSON.readAll();

    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson( text.toUtf8(), &jsonError );
    if( jsonError.error == QJsonParseError::NoError )
    {
        QJsonObject jsonObject_param = jsonDoc.object().value( "paraConfiguration" ).toObject();

        /****** 1st tab: Inputs ******/
        QJsonObject inputTab = jsonObject_param.value( "inputTab" ).toObject();
        QJsonObject inputFiles = inputTab.value( "inputFiles" ).toObject();
        para_inputTab_adFile_lineEdit->setText( inputFiles.value( "AD" ).toString() );
        para_inputTab_rdFile_lineEdit->setText( inputFiles.value( "RD" ).toString() );
        para_inputTab_mdFile_lineEdit->setText( inputFiles.value( "MD" ).toString() );
        para_inputTab_faFile_lineEdit->setText( inputFiles.value( "FA" ).toString() );
        para_inputTab_subMatrixFile_lineEdit->setText( inputFiles.value( "SUBMATRIX" ).toObject().value( "path" ).toString() );
        OnUpdatingSubjectColumnID( inputFiles.value( "SUBMATRIX" ).toObject().value( "subjectColumnID" ).toInt( 0 ) );

        QJsonObject covariates = inputTab.value( "covariates" ).toObject();
        for( int i = 0; i < std::min( para_inputTab_covariates_listWidget->count(), covariates.size() ) ; i++ )
        {
            QListWidgetItem *currentItem = para_inputTab_covariates_listWidget->item( i );
            currentItem->setCheckState( covariates.value( currentItem->text() ).toBool() ? Qt::Checked : Qt::Unchecked );
        }

        /****** 2nd tab: Subjects ******/
        QJsonObject subjectTab = jsonObject_param.value( "subjectTab" ).toObject();
        QJsonObject filesUsed = subjectTab.value( "filesUsed" ).toObject();
        para_subjectTab_adFile_checkBox->setChecked( para_subjectTab_adFile_checkBox->isEnabled() ? filesUsed.value( "AD" ).toBool() : false );
        para_subjectTab_rdFile_checkBox->setChecked( para_subjectTab_rdFile_checkBox->isEnabled() ? filesUsed.value( "RD" ).toBool() : false );
        para_subjectTab_mdFile_checkBox->setChecked( para_subjectTab_mdFile_checkBox->isEnabled() ? filesUsed.value( "MD" ).toBool() : false );
        para_subjectTab_faFile_checkBox->setChecked( para_subjectTab_faFile_checkBox->isEnabled() ? filesUsed.value( "FA" ).toBool() : false );
        para_subjectTab_subMatrixFile_checkBox->setChecked( para_subjectTab_subMatrixFile_checkBox->isEnabled() ? filesUsed.value( "SUBMATRIX" ).toBool() : false );

        para_subjectTab_qcThreshold_doubleSpinBox->setValue( subjectTab.value( "qcThreshold" ).toDouble( 0.85 ) );
        para_subjectTab_subjectFile_lineEdit->setText( subjectTab.value( "subjectListPath" ).toString() );

        /****** 3rd tab: Execution ******/
        QJsonObject executionTab = jsonObject_param.value( "executionTab" ).toObject();
        QJsonObject settings = executionTab.value( "Configuration" ).toObject();
        para_executionTab_fiberName_lineEdit->setText( settings.value( "fiberName" ).toString() );
        para_executionTab_nbrPermutations_spinBox->setValue( settings.value( "nbrPermutations" ).toInt( 100 ) );
        para_executionTab_confidenceBandsThreshold_doubleSpinBox->setValue( settings.value( "confidenceBandThreshold" ).toDouble( 0.05 ) );
        para_executionTab_pvalueThreshold_doubleSpinBox->setValue( settings.value( "pvalueThreshold" ).toDouble( 0.05 ) );
        para_executionTab_omnibus_checkBox->setChecked( settings.value( "omnibus" ).toBool() );
        para_executionTab_postHoc_checkBox->setChecked( settings.value( "posthoc" ).toBool() );

        para_executionTab_mvcm_lineEdit->setText( executionTab.value( "matlabSpecifications" ).toObject().value( "fadttsDir" ).toString() );
        para_executionTab_outputDir_lineEdit->setText( executionTab.value( "outputDir" ).toString() );

        /****** 4th tab: Plotting ******/
        QJsonObject plottingTab = jsonObject_param.value( "plottingTab" ).toObject();
        para_plottingTab_loadSetDataTab_browsePlotDirectory_lineEdit->setText( plottingTab.value( "plotdir" ).toString() );
        para_plottingTab_loadSetDataTab_fibername_lineEdit->setText( plottingTab.value( "fiberName" ).toString() );
    }
}

void FADTTSWindow::LoadSoftConfiguration( QString filename )
{
    QString text;
    QFile refJSON( filename );
    refJSON.open( QIODevice::ReadOnly | QIODevice::Text );
    text = refJSON.readAll();

    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson( text.toUtf8(), &jsonError );
    if( jsonError.error == QJsonParseError::NoError )
    {
        QJsonObject jsonObject_soft = jsonDoc.object().value( "softConfiguration" ).toObject();

        /****** 3rd tab: Execution ******/
        QJsonObject executionTab = jsonObject_soft.value( "executionTab" ).toObject();
        QJsonObject matlabSpecifications = executionTab.value( "matlabSpecifications" ).toObject();

        soft_executionTab_runMatlab_checkBox->setChecked( matlabSpecifications.value( "run" ).toBool() );
        soft_executionTab_matlabExe_lineEdit->setText( matlabSpecifications.value( "matlabExe" ).toString() );
        soft_executionTab_nbrCompThreads_spinBox->setValue( matlabSpecifications.value( "nbrCompThreads" ).toInt( 1 ) );

        //            m_soft_m->setsoft_executionTab_runMatlabSystem_radioButton( soft_executionTab_runMatlabSystem_radioButton->isChecked() );
        //            m_soft_m->setsoft_executionTab_runMatlabKD_radioButton( soft_executionTab_runMatlabKD_radioButton->isChecked() );
        //            m_soft_m->setsoft_executionTab_killDevilQueue_comboBox( soft_executionTab_killDevilQueue_comboBox->currentText() );
        //            m_soft_m->setsoft_executionTab_killDevilAllocatedMemory_spinBox( soft_executionTab_killDevilAllocatedMemory_spinBox->value() );
    }
}



/*********************** Private slots ***********************/
void FADTTSWindow::OnLoadParaConfiguration()
{
    QString dir;
    QString filename = QFileDialog::getOpenFileName( this, tr( "Load Parameters Configuration" ), dir, tr( ".json( *.json ) ;; .*( * )" ) );
    if( !filename.isEmpty() )
    {
        LoadParaConfiguration( filename );
    }
}

void FADTTSWindow::OnSaveParaConfiguration()
{
    QString filename = QFileDialog::getSaveFileName( this, tr( "Save Parameters Configuration" ), "newParamConfiguration.json", tr( ".json( *.json ) ;; .*( * )" ) );
        if( !filename.isEmpty() )
    {
        SaveParaConfiguration( filename );
    }
}

void FADTTSWindow::OnLoadSoftConfiguration()
{
    QString dir;
    QString filename = QFileDialog::getOpenFileName( this , tr( "Load Software Configuration" ) , dir , tr( ".json( *.json ) ;; .*( * )" ) );
    if( !filename.isEmpty() )
    {
        LoadSoftConfiguration( filename );
    }
}

void FADTTSWindow::OnSaveSoftConfiguration()
{
    QString filename = QFileDialog::getSaveFileName( this, tr( "Save Software Configuration" ), "newSoftConfiguration.json", tr( ".json( *.json ) ;; .*( * )" ) );
        if( !filename.isEmpty() )
    {
        SaveSoftConfiguration( filename );
    }
}

void FADTTSWindow::OnSaveNoGUIConfiguration()
{
    QString filename = QFileDialog::getSaveFileName( this, tr( "Save noGUI Configuration" ), "newNoGUIConfiguration.json", tr( ".json( *.json ) ;; .*( * )" ) );
        if( !filename.isEmpty() )
    {
        SaveNoGUIConfiguration( filename );
    }
}


void FADTTSWindow::OnDisplayAbout()
{
    QString messageBoxTitle = "About " + QString( FADTTS_TITLE );
    QString aboutFADTTS;
    aboutFADTTS = "<b>Version:</b> " + QString( FADTTS_VERSION ) + "<br>"
            "<b>Description:</b> " + QString( FADTTS_DESCRIPTION ) + "<br>"
            "<b>Contributors:</b> " + QString( FADTTS_CONTRIBUTORS );
    QMessageBox::information( this, tr( qPrintable( messageBoxTitle ) ), tr( qPrintable( aboutFADTTS ) ), QMessageBox::Ok );
}


void FADTTSWindow::closeEvent(QCloseEvent *event)
{
    if( m_matlabThread->isRunning() )
    {
        QMessageBox::StandardButton closeBox =
                QMessageBox::question( this, tr( "FADTTSter" ), tr( "Data are still being processed.<br>Are you sure you want to exit FADTTSter?" ),
                                       QMessageBox::No | QMessageBox::Yes, QMessageBox::No );
        switch( closeBox )
        {
        case QMessageBox::No:
            event->ignore();
            break;
        case QMessageBox::Yes:
        {
            m_log->AddText( "\nWarning! Thread terminated by user before completed matlab script.\n" );
            m_matlabThread->terminate();
            m_log->CloseLogFile();
            event->accept();
            break;
        }
        default:
            event->accept();
            break;
        }
    }
}


/*********************** Private function **********************/
void FADTTSWindow::SaveParaConfiguration( QString filename )
{
    QString text;
    QFile refJSON( QString( ":/ConfigurationFiles/Resources/ConfigurationFiles/paramConfiguration.json" ) );
    refJSON.open( QIODevice::ReadOnly | QIODevice::Text );
    text = refJSON.readAll();

    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson( text.toUtf8(), &jsonError );
    if( jsonError.error == QJsonParseError::NoError )
    {
        QJsonObject jsonObject = jsonDoc.object();
        QJsonObject jsonObject_param = jsonObject[ "paraConfiguration" ].toObject();
        QJsonObject inputTab = jsonObject_param[ "inputTab" ].toObject();
        QJsonObject subjectTab = jsonObject_param[ "subjectTab" ].toObject();
        QJsonObject executionTab = jsonObject_param[ "executionTab" ].toObject();
        QJsonObject plottingTab = jsonObject_param[ "plottingTab" ].toObject();

        /****** 1st tab: Inputs ******/
        QJsonObject inputFiles = inputTab[ "inputFiles" ].toObject();
        inputFiles[ "AD" ] = para_inputTab_adFile_lineEdit->text();
        inputFiles[ "RD" ] = para_inputTab_rdFile_lineEdit->text();
        inputFiles[ "MD" ] = para_inputTab_mdFile_lineEdit->text();
        inputFiles[ "FA" ] = para_inputTab_faFile_lineEdit->text();
        QJsonObject submatrixFile = inputFiles[ "SUBMATRIX" ].toObject();
        submatrixFile[ "path" ] = para_inputTab_subMatrixFile_lineEdit->text();
        submatrixFile[ "subjectColumnID" ] = m_data.GetSubjectColumnID();
        inputFiles[ "SUBMATRIX" ] = submatrixFile;
        inputTab[ "inputFiles" ] = inputFiles;

        QJsonObject covariates;
        for( int i = 0; i < m_covariateListWidget->count(); i++ )
        {
            covariates.insert( m_covariateListWidget->item( i )->text(), ( m_covariateListWidget->item( i )->checkState() == Qt::Checked ) ? true : false );
        }
        inputTab[ "covariates" ] = covariates;

        /****** 2nd tab: Subjects ******/
        QJsonObject filesUsed = subjectTab[ "filesUsed" ].toObject();
        filesUsed[ "AD" ] = para_subjectTab_adFile_checkBox->isChecked();
        filesUsed[ "RD" ] = para_subjectTab_rdFile_checkBox->isChecked();
        filesUsed[ "MD" ] = para_subjectTab_mdFile_checkBox->isChecked();
        filesUsed[ "FA" ] = para_subjectTab_faFile_checkBox->isChecked();
        filesUsed[ "SUBMATRIX" ] = para_subjectTab_subMatrixFile_checkBox->isChecked();
        subjectTab[ "filesUsed" ] = filesUsed;

        subjectTab[ "qcThreshold" ] = para_subjectTab_qcThreshold_doubleSpinBox->value();
        subjectTab[ "subjectListPath" ] = para_subjectTab_subjectFile_lineEdit->text();

        /****** 3rd tab: Execution ******/
        QJsonObject settings = executionTab[ "settings" ].toObject();
        settings[ "fiberName" ] = para_executionTab_fiberName_lineEdit->text();
        settings[ "nbrPermutations" ] = para_executionTab_nbrPermutations_spinBox->value();
        settings[ "confidenceBandThreshold" ] = para_executionTab_confidenceBandsThreshold_doubleSpinBox->value();
        settings[ "pvalueThreshold" ] = para_executionTab_pvalueThreshold_doubleSpinBox->value();
        settings[ "omnibus" ] = para_executionTab_omnibus_checkBox->isChecked();
        settings[ "posthoc" ] = para_executionTab_postHoc_checkBox->isChecked();
        executionTab[ "settings" ] = settings;

        QJsonObject matlabSpecifications = executionTab[ "matlabSpecifications" ].toObject();
        matlabSpecifications[ "fadttsDir" ] = para_executionTab_mvcm_lineEdit->text();
        executionTab[ "matlabSpecifications" ] = matlabSpecifications;

        executionTab[ "outputDir" ] = para_executionTab_outputDir_lineEdit->text();

        /****** 4th tab: Plotting ******/
        plottingTab[ "plotdir" ] = para_plottingTab_loadSetDataTab_browsePlotDirectory_lineEdit->text();
        plottingTab[ "fiberName" ] = para_plottingTab_loadSetDataTab_fibername_lineEdit->text();


        jsonObject_param[ "inputTab" ] = inputTab;
        jsonObject_param[ "subjectTab" ] = subjectTab;
        jsonObject_param[ "executionTab" ] = executionTab;
        jsonObject_param[ "plottingTab" ] = plottingTab;
        jsonObject[ "paramSettings" ] = jsonObject_param;

        jsonDoc.setObject( jsonObject );
        QFile exportedJSON( filename );
        exportedJSON.open( QIODevice::WriteOnly | QIODevice::Text );
        exportedJSON.write( jsonDoc.toJson( QJsonDocument::Indented ) );
        exportedJSON.flush();
        exportedJSON.close();
    }
}

void FADTTSWindow::SaveSoftConfiguration( QString filename )
{
    QString text;
    QFile refJSON( QString( ":/ConfigurationFiles/Resources/ConfigurationFiles/softConfiguration.json" ) );
    refJSON.open( QIODevice::ReadOnly | QIODevice::Text );
    text = refJSON.readAll();

    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson( text.toUtf8(), &jsonError );
    if( jsonError.error == QJsonParseError::NoError )
    {
        QJsonObject jsonObject = jsonDoc.object();
        QJsonObject jsonObject_soft = jsonObject[ "softConfiguration" ].toObject();
        QJsonObject executionTab = jsonObject_soft[ "executionTab" ].toObject();

        /****** 3rd tab: Execution ******/
        QJsonObject matlabSpecifications = executionTab[ "matlabSpecifications" ].toObject();
        matlabSpecifications[ "runMatlab" ] = soft_executionTab_runMatlab_checkBox->isChecked();
        matlabSpecifications[ "matlabExe" ] = soft_executionTab_matlabExe_lineEdit->text();
        matlabSpecifications[ "nbrCompThreads" ] = soft_executionTab_nbrCompThreads_spinBox->value();
        executionTab[ "matlabSpecifications" ] = matlabSpecifications;


        jsonObject_soft[ "executionTab" ] = executionTab;
        jsonObject[ "softSettings" ] = jsonObject_soft;

        jsonDoc.setObject( jsonObject );
        QFile exportedJSON( filename );
        exportedJSON.open( QIODevice::WriteOnly | QIODevice::Text );
        exportedJSON.write( jsonDoc.toJson( QJsonDocument::Indented ) );
        exportedJSON.flush();
        exportedJSON.close();
    }
}

void FADTTSWindow::SaveNoGUIConfiguration( QString filename )
{
    QString text;
    QFile refJSON( QString( ":/ConfigurationFiles/Resources/ConfigurationFiles/noGUIConfiguration.json" ) );
    refJSON.open( QIODevice::ReadOnly | QIODevice::Text );
    text = refJSON.readAll();

    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson( text.toUtf8(), &jsonError );
    if( jsonError.error == QJsonParseError::NoError )
    {
        QJsonObject jsonObject = jsonDoc.object();
        QJsonObject jsonObject_noGUI = jsonObject[ "noGUIConfiguration" ].toObject();
        QJsonObject inputFiles = jsonObject_noGUI[ "inputFiles" ].toObject();
        QJsonObject covariates = jsonObject_noGUI[ "covariates" ].toObject();
        QJsonObject subjects = jsonObject_noGUI[ "subjects" ].toObject();
        QJsonObject settings = jsonObject_noGUI[ "settings" ].toObject();
        QJsonObject matlabSpecifications = jsonObject_noGUI[ "matlabSpecifications" ].toObject();


        /****** Input Files ******/
        foreach( QString key, inputFiles.keys() )
        {
            QJsonObject currentPropertyObject = inputFiles[ key ].toObject();
            int currentPropertyIndex = currentPropertyObject.value( "index" ).toInt();
            bool isFileSelected = m_paramTabFileCheckBoxMap.value( currentPropertyIndex )->isChecked()
                    && m_paramTabFileCheckBoxMap.value( currentPropertyIndex )->isEnabled();
            currentPropertyObject[ "path" ] = isFileSelected ? m_inputTabInputFileLineEditMap.value( currentPropertyIndex )->text() : false;
            if( key == "SUBMATRIX" )
            {
                currentPropertyObject[ "subjectColumnID" ] = m_data.GetSubjectColumnID();
            }
            inputFiles[ key ] = currentPropertyObject;
        }


        /******  Covariates  ******/
        for( int i = 0; i < m_covariateListWidget->count(); i++ )
        {
            QListWidgetItem *currentItem = m_covariateListWidget->item( i );
            QString currentCovariateName = currentItem->text();
            bool currentCovariateSelected = currentItem->checkState() == Qt::Checked ? true : false;
            int currentCovariateIndex = m_data.GetCovariates().key( currentCovariateName );

            QJsonObject currentCovariateObject;
            currentCovariateObject.insert( "index", currentCovariateIndex );
            currentCovariateObject.insert( "selected", currentCovariateSelected );

            covariates.insert( currentCovariateName, currentCovariateObject );
        }

        /******   Subjects   ******/
        subjects[ "subjectListPath" ] = para_subjectTab_subjectFile_lineEdit->text();

        QJsonObject qcThreshold = subjects[ "qcThreshold" ].toObject();
        qcThreshold[ "apply" ] = !m_failedQCThresholdSubjects.isEmpty();
        qcThreshold[ "value" ] = para_subjectTab_qcThreshold_doubleSpinBox->value();
        qcThreshold[ "propertyRefID" ] = "FA" ;
        subjects[ "qcThreshold" ] = qcThreshold;

        /******   Settings   ******/
        settings[ "fiberName" ] = para_executionTab_fiberName_lineEdit->text();
        settings[ "nbrPermutations" ] = para_executionTab_nbrPermutations_spinBox->value();
        settings[ "confidenceBandThreshold" ] = para_executionTab_confidenceBandsThreshold_doubleSpinBox->value();
        settings[ "pvalueThreshold" ] = para_executionTab_pvalueThreshold_doubleSpinBox->value();
        settings[ "omnibus" ] = para_executionTab_omnibus_checkBox->isChecked();
        settings[ "posthoc" ] = para_executionTab_postHoc_checkBox->isChecked();

        /******  Output Dir  ******/
        jsonObject_noGUI[ "outputDir" ] = para_executionTab_outputDir_lineEdit->text();

        /* Matlab  Specifications */
        matlabSpecifications[ "fadttsDir" ] = para_executionTab_mvcm_lineEdit->text();
        matlabSpecifications[ "runMatlab" ] = soft_executionTab_runMatlab_checkBox->isChecked();
        matlabSpecifications[ "matlabExe" ] = soft_executionTab_matlabExe_lineEdit->text();
        matlabSpecifications[ "nbrCompThreads" ] = soft_executionTab_nbrCompThreads_spinBox->value();


        jsonObject_noGUI[ "inputFiles" ] = inputFiles;
        jsonObject_noGUI[ "covariates" ] = covariates;
        jsonObject_noGUI[ "subjects" ] = subjects;
        jsonObject_noGUI[ "settings" ] = settings;
        jsonObject_noGUI[ "matlabSpecifications" ] = matlabSpecifications;
        jsonObject[ "noGUI" ] = jsonObject_noGUI;

        jsonDoc.setObject( jsonObject );
        QFile exportedJSON( filename );
        exportedJSON.open( QIODevice::WriteOnly | QIODevice::Text );
        exportedJSON.write( jsonDoc.toJson( QJsonDocument::Indented ) );
        exportedJSON.flush();
        exportedJSON.close();
    }
}



void FADTTSWindow::InitMenuBar()
{
    /** Load parameters (para_) from a .json file into the GUI interface **/
    connect( this->actionLoad_Para_Configuration, SIGNAL( triggered() ), SLOT( OnLoadParaConfiguration() ) );
    /** Save parameters (para_) from the GUI interface into a .json file **/
    connect( this->actionSave_Para_Configuration, SIGNAL( triggered() ), SLOT( OnSaveParaConfiguration() ) );

    /** Load software parameters (soft_) from a .json file into the GUI interface **/
    connect( this->actionLoad_Soft_Configuration, SIGNAL( triggered() ), SLOT( OnLoadSoftConfiguration() ) );
    /** Save software parameters (soft_) from the GUI interface into a .json file **/
    connect( this->actionSave_Soft_Configuration, SIGNAL( triggered() ), SLOT( OnSaveSoftConfiguration() ) );

    /** Save parameters (para_) and software parameters (soft_) from the GUI interface into a .json file for the FADTTSter --noGUI mode**/
    connect( this->actionSave_noGUI_Configuration, SIGNAL( triggered() ), SLOT( OnSaveNoGUIConfiguration() ) );


    connect( this->actionAbout, SIGNAL( triggered() ), SLOT( OnDisplayAbout() ) );
}

void FADTTSWindow::InitInputTab()
{
    connect( this->inputTab_addMultipleInputFiles_pushButton, SIGNAL( clicked() ), this, SLOT( OnAddInputFiles() ) );

    /** Map of PushButtons to add each file separetely and
     *  SignalMapper to link them to the slot AddFile() **/
    m_inputTabAddInputFilePushButtonMap.insert( m_data.GetAxialDiffusivityIndex(), this->inputTab_addADFile_pushButton );
    m_inputTabAddInputFilePushButtonMap.insert( m_data.GetRadialDiffusivityIndex(), this->inputTab_addRDFile_pushButton );
    m_inputTabAddInputFilePushButtonMap.insert( m_data.GetMeanDiffusivityIndex(), this->inputTab_addMDFile_pushButton );
    m_inputTabAddInputFilePushButtonMap.insert( m_data.GetFractionalAnisotropyIndex(), this->inputTab_addFAFile_pushButton );
    m_inputTabAddInputFilePushButtonMap.insert( m_data.GetSubMatrixIndex(), this->inputTab_addSubMatrixFile_pushButton );
    QSignalMapper *signalMapperAddFile = new QSignalMapper( this );
    connect( signalMapperAddFile, SIGNAL( mapped( int ) ), this, SLOT( OnAddInputFile( int ) ) );
    for( int i = 0; i < m_data.GetDiffusionPropertiesIndices().size(); ++i )
    {
        connect( m_inputTabAddInputFilePushButtonMap[ m_data.GetDiffusionPropertiesIndices().at( i ) ], SIGNAL( clicked() ), signalMapperAddFile,SLOT(map() ) );
        signalMapperAddFile->setMapping( m_inputTabAddInputFilePushButtonMap[ m_data.GetDiffusionPropertiesIndices().at( i ) ], m_data.GetDiffusionPropertiesIndices().at( i ) );
    }

    /** Map of LineEdits where the file path of each file is set and
     *  SignalMapper to link them to the slot UpdateInputLineEdit() **/
    m_inputTabInputFileLineEditMap.insert( m_data.GetAxialDiffusivityIndex(), this->para_inputTab_adFile_lineEdit );
    m_inputTabInputFileLineEditMap.insert( m_data.GetRadialDiffusivityIndex(), this->para_inputTab_rdFile_lineEdit );
    m_inputTabInputFileLineEditMap.insert( m_data.GetMeanDiffusivityIndex(), this->para_inputTab_mdFile_lineEdit );
    m_inputTabInputFileLineEditMap.insert( m_data.GetFractionalAnisotropyIndex(), this->para_inputTab_faFile_lineEdit );
    m_inputTabInputFileLineEditMap.insert( m_data.GetSubMatrixIndex(), this->para_inputTab_subMatrixFile_lineEdit );
    QSignalMapper *signalMapperUpdateLineEdit = new QSignalMapper( this );
    connect( signalMapperUpdateLineEdit, SIGNAL( mapped( int ) ), this, SLOT( OnSettingInputFile( int ) ) );
    for( int i = 0; i < m_data.GetDiffusionPropertiesIndices().size(); ++i )
    {
        connect( m_inputTabInputFileLineEditMap[ m_data.GetDiffusionPropertiesIndices().at( i ) ], SIGNAL( textChanged( const QString& ) ), signalMapperUpdateLineEdit,SLOT(map() ) );
        signalMapperUpdateLineEdit->setMapping( m_inputTabInputFileLineEditMap[ m_data.GetDiffusionPropertiesIndices().at( i ) ], m_data.GetDiffusionPropertiesIndices().at( i ) );
    }

    /** Map of Labels to set the icon information of each file entered in a LineEdit **/
    m_inputTabIconLabelMap.insert( m_data.GetAxialDiffusivityIndex(), this->inputTab_iconADFile_label );
    m_inputTabIconLabelMap.insert( m_data.GetRadialDiffusivityIndex(), this->inputTab_iconRDFile_label );
    m_inputTabIconLabelMap.insert( m_data.GetMeanDiffusivityIndex(), this->inputTab_iconMDFile_label );
    m_inputTabIconLabelMap.insert( m_data.GetFractionalAnisotropyIndex(), this->inputTab_iconFAFile_label );
    m_inputTabIconLabelMap.insert( m_data.GetSubMatrixIndex(), this->inputTab_iconSubMatrixFile_label );

    m_editInputDialog = QSharedPointer< EditInputDialog >( new EditInputDialog( this ) );
    m_editInputDialog->SetData( &m_data );
    connect( m_editInputDialog.data(), SIGNAL( UpdateInputFile( const int&, const QString& ) ), this, SLOT( OnUpdatingInputFile( const int&, const QString& ) ) );
    connect( m_editInputDialog.data(), SIGNAL( UpdateSubjectColumnID( int ) ), this, SLOT( OnUpdatingSubjectColumnID( int ) ) );

    /** Map of PushButtons to edit the files and
     *  SignalMapper to link them to the slot EditFile() **/
    m_inputTabEditInputFilePushButtonMap.insert( m_data.GetAxialDiffusivityIndex(), this->inputTab_editADFile_pushButton );
    m_inputTabEditInputFilePushButtonMap.insert( m_data.GetRadialDiffusivityIndex(), this->inputTab_editRDFile_pushButton );
    m_inputTabEditInputFilePushButtonMap.insert( m_data.GetMeanDiffusivityIndex(), this->inputTab_editMDFile_pushButton );
    m_inputTabEditInputFilePushButtonMap.insert( m_data.GetFractionalAnisotropyIndex(), this->inputTab_editFAFile_pushButton );
    m_inputTabEditInputFilePushButtonMap.insert( m_data.GetSubMatrixIndex(), this->inputTab_editSubMatrixFile_pushButton );
    QSignalMapper *signalMapperEditFile = new QSignalMapper( this );
    connect( signalMapperEditFile, SIGNAL( mapped( int ) ), this, SLOT( OnEditInputFile( int ) ) );
    for( int i = 0; i < m_data.GetDiffusionPropertiesIndices().size(); ++i )
    {
        connect( m_inputTabEditInputFilePushButtonMap[ m_data.GetDiffusionPropertiesIndices().at( i ) ], SIGNAL( clicked() ), signalMapperEditFile, SLOT(map() ) );
        signalMapperEditFile->setMapping( m_inputTabEditInputFilePushButtonMap[ m_data.GetDiffusionPropertiesIndices().at( i ) ], m_data.GetDiffusionPropertiesIndices().at( i ) );
    }

    /** Map of Labels to edit the file information and
     *  SignalMapper to link them to the slot EditFile() **/
    m_inputFileInformationLabelMap.insert( m_data.GetAxialDiffusivityIndex(), this->inputTab_adFileInfo_label );
    m_inputFileInformationLabelMap.insert( m_data.GetRadialDiffusivityIndex(), this->inputTab_rdFileInfo_label );
    m_inputFileInformationLabelMap.insert( m_data.GetMeanDiffusivityIndex(), this->inputTab_mdFileInfo_label );
    m_inputFileInformationLabelMap.insert( m_data.GetFractionalAnisotropyIndex(), this->inputTab_faFileInfo_label );
    m_inputFileInformationLabelMap.insert( m_data.GetSubMatrixIndex(), this->inputTab_subMatrixFileInfo_label );

    m_covariateListWidget = new QListWidget();
    m_covariateListWidget = this->para_inputTab_covariates_listWidget;
    connect( m_covariateListWidget, SIGNAL( itemClicked( QListWidgetItem * ) ), this, SLOT( OnCovariateClicked( QListWidgetItem * ) ) );
    connect( this->inputTab_covariatesCheckAll_pushButton, SIGNAL( clicked() ), this, SLOT( OnCheckAllCovariates() ) );
    connect( this->inputTab_covariatesUncheckAll_pushButton, SIGNAL( clicked() ), this, SLOT( OnUnCheckAllCovariates() ) );

    DisplayFileInformation();
}

void FADTTSWindow::InitSubjectCovariateTab()
{
    /*** Inputs Available ***/
    /** Map of CheckBoxes to select the files we want to work on and
     *  SignalMapper to link them to the slot SortSubjects() **/
    m_paramTabFileCheckBoxMap.insert( m_data.GetAxialDiffusivityIndex(), this->para_subjectTab_adFile_checkBox );
    m_paramTabFileCheckBoxMap.insert( m_data.GetRadialDiffusivityIndex(), this->para_subjectTab_rdFile_checkBox );
    m_paramTabFileCheckBoxMap.insert( m_data.GetMeanDiffusivityIndex(), this->para_subjectTab_mdFile_checkBox );
    m_paramTabFileCheckBoxMap.insert( m_data.GetFractionalAnisotropyIndex(), this->para_subjectTab_faFile_checkBox );
    m_paramTabFileCheckBoxMap.insert( m_data.GetSubMatrixIndex(), this->para_subjectTab_subMatrixFile_checkBox );
    QSignalMapper *signalMapperSelectFile = new QSignalMapper( this );
    connect( signalMapperSelectFile, SIGNAL( mapped( int ) ), this, SLOT( OnInputToggled() ) );
    for ( int i = 0; i < m_data.GetDiffusionPropertiesIndices().size(); ++i )
    {
        connect( m_paramTabFileCheckBoxMap[ m_data.GetDiffusionPropertiesIndices().at( i ) ], SIGNAL( toggled( bool ) ), signalMapperSelectFile,SLOT( map() ) );
        signalMapperSelectFile->setMapping( m_paramTabFileCheckBoxMap[ m_data.GetDiffusionPropertiesIndices().at( i ) ], m_data.GetDiffusionPropertiesIndices().at( i ) );
    }

    /*** QC Threshold ***/
    m_qcThresholdDialog = QSharedPointer< QCThresholdDialog >( new QCThresholdDialog( this ) );
    connect( this->subjectTab_applyQCThreshold_pushButton, SIGNAL( clicked() ), this, SLOT( OnApplyQCThreshold() ) );
    connect( m_qcThresholdDialog.data(), SIGNAL( ApplyQCThreshold( const QStringList&, const QStringList&, double ) ), this, SLOT( OnApplyingQCThreshold( const QStringList&, const QStringList&, double ) ) );

    /*** Subjects Lists ***/
    m_areSubjectsLoaded = false;

    m_matchedSubjectListWidget = new QListWidget();
    m_matchedSubjectListWidget = this->subjectTab_matchedSubjectsInformation_listWidget;
    connect( m_matchedSubjectListWidget, SIGNAL( itemClicked( QListWidgetItem * ) ), this, SLOT( OnSubjectClicked( QListWidgetItem * ) ) );
    connect( this->subjectTab_checkAllVisible_pushButton, SIGNAL( clicked() ), this, SLOT( OnCheckAllVisibleSubjects() ) );
    connect( this->subjectTab_unCheckAllVisible_pushButton, SIGNAL( clicked() ), this, SLOT( OnUnCheckAllVisibleSubjects() ) );

    m_unmatchedSubjectListWidget = new QListWidget();
    m_unmatchedSubjectListWidget = this->subjectTab_unmatchedSubjectsInformation_listWidget;

    /*** Search Subjects ***/
    m_caseSensitivity = Qt::CaseInsensitive;
    connect( this->subjectTab_search_lineEdit, SIGNAL( textEdited( const QString& ) ), this, SLOT( OnSearch() ) );
    connect( this->subjectTab_caseSensitive_checkBox, SIGNAL( toggled( bool ) ), this, SLOT( OnSetCaseSensitivityToggled( bool ) ) );

    /*** Subjects File ***/
    m_subjectFileLineEdit = new QLineEdit();
    m_subjectFileLineEdit = this->para_subjectTab_subjectFile_lineEdit;
    connect( m_subjectFileLineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( OnSettingSubjectList( const QString&  ) ) );
    connect( this->subjectTab_loadSubjectFile_PushButton, SIGNAL( clicked() ), this, SLOT( OnLoadSubjectList() ) );
    connect( this->subjectTab_resetSubjectFile_pushButton, SIGNAL( clicked() ), this, SLOT( OnResetSubjectList() ) );
    connect( this->subjectTab_saveCheckedSubjects_pushButton, SIGNAL( clicked() ), this, SLOT( OnSaveCheckedSubjects() ) );

    /** ATLAS QCTHRESHOLD **/
//    para_subjectTab_qcThresoldOnAtlas_radioButton->setDisabled( true );
    para_subjectTab_qcThresoldOnPopulation_radioButton->setDisabled( true );
    subjectTab_addQCThresholdAtlas_PushButton->setDisabled( true );
    para_subjectTab_qcThresoldAtlas_lineEdit->setDisabled( true );
    subjectTab_iconQCThresoldAltlas_label->setDisabled( true );

}

void FADTTSWindow::InitExecutionTab()
{
    /*** Settings ***/
    this->para_executionTab_nbrPermutations_spinBox->setMaximum( 2000 );

    /*** Output directory ***/
    connect( this->executionTab_outputDir_pushButton, SIGNAL( clicked() ), this, SLOT( OnBrowsingOutputDir() ) );
    connect( this->para_executionTab_outputDir_lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( OnSettingOutputDir( const QString& ) ) );

    /*** Matlab Specification ***/
    m_isMatlabExeFound = false;

    connect( this->executionTab_mvcm_pushButton, SIGNAL( clicked() ), this, SLOT( OnBrowsingMVCMPath() ) );
    connect( this->para_executionTab_mvcm_lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( OnSettingMVCMPath( const QString& ) ) );

    connect( this->soft_executionTab_runMatlab_checkBox, SIGNAL( toggled( bool ) ), this, SLOT( OnRunMatlabToggled( bool ) ) );
    connect( this->executionTab_matlabExe_pushButton, SIGNAL( clicked() ), this, SLOT( OnBrowsingMatlabExe() ) );
    connect( this->soft_executionTab_matlabExe_lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( OnSettingMatlabExe( const QString& ) ) );

    this->soft_executionTab_nbrCompThreads_spinBox->setMaximum( QThread::idealThreadCount() );

    /*** Run ***/
    m_progressBar = new QProgressBar();
    m_progressBar = this->executionTab_progressBar;
    m_progressBar->setMinimum( 0 );
    m_progressBar->setMaximum( 0 );
    m_progressBar->hide();

    m_matlabThread = new MatlabThread();
    connect( m_matlabThread, SIGNAL( finished() ), this, SLOT( OnMatlabThreadFinished() ) );

    connect( this->executionTab_run_pushButton, SIGNAL( clicked() ), this, SLOT( OnRun() ) );
    connect( this->executionTab_stop_pushButton, SIGNAL( clicked() ), this, SLOT( OnStop() ) );
    this->executionTab_run_pushButton->setEnabled( true );
    this->executionTab_stop_pushButton->setEnabled( false );

    /*** Log ***/
    m_log = new Log;
    m_log->SetMatlabScript( m_matlabThread );
    connect( m_log, SIGNAL( UpdateLogActivity( const QString& ) ), this, SLOT( OnUpdatingLogActivity( const QString& ) ) );

    m_logWindow = new QTextEdit();
    m_logWindow = this->executionTab_log_textEdit;
    m_logWindow->setReadOnly( true );

    connect( this->executionTab_clearLog_pushButton, SIGNAL( clicked() ), this, SLOT( OnClearLog() ) );


    OnRunMatlabToggled( false );


    /** Run on KD **/
    soft_executionTab_runMatlabKillDevil_radioButton->setDisabled( true );
    executionTab_killDevilQueue_label->setDisabled( true );
    soft_executionTab_killDevilQueue_comboBox->setDisabled( true );
    executionTab_killDevilAllocatedMemory_label->setDisabled( true );
    soft_executionTab_killDevilAllocatedMemory_spinBox->setDisabled( true );
    executionTab_killDevilAllocatedMemoryInfo_label->setDisabled( true );
}

void FADTTSWindow::InitPlottingTab()
{
    m_qvtkWidget = QSharedPointer< QVTKWidget >( this->plottingTab_plot_qvtkWidget );

    m_plot = new Plot();
    m_plot->SetQVTKWidget( m_qvtkWidget );
    connect( m_plot, SIGNAL( PlotsUsed( const QStringList& ) ), this, SLOT( OnSettingPlotsUsed( const QStringList& ) ) );
    connect( m_plot, SIGNAL( AllPropertiesUsed( const QMap< int, QString >& ) ), this, SLOT( OnSettingAllPropertiesUsed( const QMap< int, QString >& ) ) );
    connect( m_plot, SIGNAL( AllCovariatesUsed( const QMap< int, QString >& ) ), this, SLOT( OnSettingAllCovariatesUsed( const QMap< int, QString >& ) ) );
    connect( m_plot, SIGNAL( CovariatesAvailableForPlotting( const QMap< int, QString >& ) ), this, SLOT( OnUpdatingCovariatesAvailable( const QMap< int, QString >& ) ) );
    connect( m_plot, SIGNAL( LinesSelected( const QStringList& ) ), this, SLOT( OnSettingLinesSelected( const QStringList& ) ) );

    /*** Load/Set Data Tab ***/
    connect( this->plottingTab_loadSetDataTab_browsePlotDirectory_pushButton, SIGNAL( clicked() ), this, SLOT( OnBrowsingPlotDir() ) );
    connect( this->para_plottingTab_loadSetDataTab_browsePlotDirectory_lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( OnSettingPlotDir( const QString& ) ) );
    connect( this->para_plottingTab_loadSetDataTab_fibername_lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( OnSettingPlotFibername( const QString& ) ) );


    m_plotComboBox = new QComboBox();
    m_plotComboBox = this->plottingTab_loadSetDataTab_plotSelection_comboBox;
    connect( m_plotComboBox, SIGNAL( currentIndexChanged( const QString& ) ), this, SLOT( OnPlotSelection( const QString& ) ) );

    m_propertyComboBox = new QComboBox();
    m_propertyComboBox = this->plottingTab_loadSetDataTab_propertySelection_comboBox;
    connect( m_propertyComboBox, SIGNAL( currentIndexChanged( const QString& ) ), this, SLOT( OnPropertySelection( const QString& ) ) );

    m_covariateComboBox = new QComboBox();
    m_covariateComboBox = this->plottingTab_loadSetDataTab_covariateSelection_comboBox;
    connect( m_covariateComboBox, SIGNAL( currentIndexChanged( const QString& ) ), this, SLOT( OnCovariateSelection( const QString& ) ) );

    m_lineDisplayedListWidget = new QListWidget();
    m_lineDisplayedListWidget = this->plottingTab_loadSetDataTab_linesToDisplay_listWidget;
    connect( m_lineDisplayedListWidget, SIGNAL( itemClicked( QListWidgetItem * ) ), this, SLOT( OnLineForDisplayClicked( QListWidgetItem * ) ) );
    connect( this->plottingTab_loadSetDataTab_checkAllLinesToDisplay_pushButton, SIGNAL( clicked() ), this, SLOT( OnCheckAllLinesToDisplay() ) );
    connect( this->plottingTab_loadSetDataTab_uncheckAllLinesToDisplay_pushButton, SIGNAL( clicked() ), this, SLOT( OnUncheckAllToDisplay() ) );

    m_lineSelectedListWidget = new QListWidget();
    m_lineSelectedListWidget = this->plottingTab_loadSetDataTab_linesSelected_listWidget;


    /*** Title/Axis/Legend Tab ***/
    connect( this->plottingTab_titleAxisLegendTab_useCustomizedTitle_checkBox, SIGNAL( toggled( bool ) ), this, SLOT( OnUseCustomizedTitle( bool ) ) );
    connect( this->plottingTab_titleAxisLegendTab_useCustomizedTitle_checkBox, SIGNAL( toggled( bool ) ), this, SLOT( OnUpdatingPlotTitle() ) );
    this->plottingTab_titleAxisLegendTab_useCustomizedTitle_checkBox->setChecked( false );

    connect( this->plottingTab_titleAxisLegendTab_titleName_lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( OnUpdatingPlotTitle() ) );
    connect( this->plottingTab_titleAxisLegendTab_titleBold_checkBox, SIGNAL( toggled( bool ) ), this, SLOT( OnUpdatingPlotTitle() ) );
    connect( this->plottingTab_titleAxisLegendTab_titleItalic_checkBox, SIGNAL( toggled( bool ) ), this, SLOT( OnUpdatingPlotTitle() ) );
    connect( this->plottingTab_titleAxisLegendTab_titleSize_doubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnUpdatingPlotTitle() ) );

    connect( this->plottingTab_titleAxisLegendTab_gridOn_checkBox, SIGNAL( toggled( bool ) ), this, SLOT( OnUpdatingPlotGrid( bool ) ) );

    connect( this->plottingTab_titleAxisLegendTab_useCustomizedAxis_checkBox, SIGNAL( toggled( bool ) ), this, SLOT( OnUseCustomizedAxis( bool ) ) );
    connect( this->plottingTab_titleAxisLegendTab_useCustomizedAxis_checkBox, SIGNAL( toggled( bool ) ), this, SLOT( OnUpdatingPlotAxis() ) );
    this->plottingTab_titleAxisLegendTab_useCustomizedAxis_checkBox->setChecked( false );

    connect( this->plottingTab_titleAxisLegendTab_xName_lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( OnUpdatingPlotAxis() ) );
    connect( this->plottingTab_titleAxisLegendTab_yName_lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( OnUpdatingPlotAxis() ) );
    connect( this->plottingTab_titleAxisLegendTab_axisBold_checkBox, SIGNAL( toggled( bool ) ), this, SLOT( OnUpdatingPlotAxis() ) );
    connect( this->plottingTab_titleAxisLegendTab_axisItalic_checkBox, SIGNAL( toggled( bool ) ), this, SLOT( OnUpdatingPlotAxis() ) );

    connect( this->plottingTab_titleAxisLegendTab_yMax_checkBox, SIGNAL( toggled( bool ) ), this, SLOT( OnYMaxToggled( bool ) ) );
    connect( this->plottingTab_titleAxisLegendTab_yMax_checkBox, SIGNAL( toggled( bool ) ), this, SLOT( OnUpdatingPlotAxis() ) );
    this->plottingTab_titleAxisLegendTab_yMax_checkBox->setChecked( false );

    connect( this->plottingTab_titleAxisLegendTab_yMin_doubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnYMinValueChanged( double ) ) );
    connect( this->plottingTab_titleAxisLegendTab_yMin_doubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnUpdatingPlotAxis() ) );

    connect( this->plottingTab_titleAxisLegendTab_yMin_checkBox, SIGNAL( toggled( bool ) ), this, SLOT( OnYMinToggled( bool ) ) );
    connect( this->plottingTab_titleAxisLegendTab_yMin_checkBox, SIGNAL( toggled( bool ) ), this, SLOT( OnUpdatingPlotAxis() ) );
    this->plottingTab_titleAxisLegendTab_yMin_checkBox->setChecked( false );

    connect( this->plottingTab_titleAxisLegendTab_yMax_doubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnYMaxValueChanged( double ) ) );
    connect( this->plottingTab_titleAxisLegendTab_yMax_doubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnUpdatingPlotAxis() ) );

    connect( this->plottingTab_titleAxisLegendTab_legendPosition_comboBox, SIGNAL( currentIndexChanged( const QString& ) ), this, SLOT( OnUpdatingLegend( const QString& ) ) );


    /*** Edition Tab ***/
    connect( this->plottingTab_editionTab_pvalueThreshold_doubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnUpdatingPvalueThreshold( double ) ) );

    connect( this->plottingTab_editionTab_lineWidth_doubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnUpdatingLineWidth( double ) ) );
    connect( this->plottingTab_editionTab_selectedLineColor_comboBox, SIGNAL( currentIndexChanged( const QString& ) ), this, SLOT( OnUpdatingSelectedColorLine( const QString& ) ) );
    SetColorsComboBox( this->plottingTab_editionTab_selectedLineColor_comboBox );
    this->plottingTab_editionTab_selectedLineColor_comboBox->setCurrentText( "Red" );

    connect( this->plottingTab_editionTab_markerType_comboBox, SIGNAL( currentIndexChanged( const QString& ) ), this, SLOT( OnUpdatingMarkerType( const QString& ) ) );
    connect( this->plottingTab_editionTab_markerSize_doubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnUpdatingMarkerSize( double ) ) );



    connect( this->plottingTab_displayPlot_pushButton, SIGNAL( clicked() ), this, SLOT( OnDisplayPlot() ) );
    connect( this->plottingTab_resetPlot_pushButton, SIGNAL( clicked() ), this, SLOT( OnResetPlot() ) );
    connect( this->plottingTab_savePlot_pushButton, SIGNAL( clicked() ), m_plot, SLOT( OnSavePlot() ) );
    connect( this->plottingTab_loadPlotSettings_pushButton, SIGNAL( clicked() ), this, SLOT( OnLoadPlotSettings() ) );
    connect( this->plottingTab_savePlotSettings_pushButton, SIGNAL( clicked() ), this, SLOT( OnSavePlotSettings() ) );

    SetPlotTab();
}

void FADTTSWindow::InitFADTTSWindow()
{
    m_data.InitData();

    /** Initialization of the menu bar and all FADTTSter tabs **/
    InitMenuBar();
    InitInputTab();
    InitSubjectCovariateTab();
    InitExecutionTab();
    InitPlottingTab();

    InitAvailableDiffusionProperties();
}


QDir FADTTSWindow::UpdateCurrentDir( QString newfilePath, QString& currentDir )
{
    QDir dir = QFileInfo( QFile( newfilePath ) ).absolutePath();
    if( dir.exists() )
    {
        currentDir = dir.absolutePath();
    }

    return dir;
}

QDir FADTTSWindow::SetDir( QString filePath, QString currentDir )
{
    return ( !filePath.isEmpty() && QDir( QFileInfo( QFile( filePath ) ).absolutePath() ).exists() ) ?
                QFileInfo( QFile( filePath ) ).absolutePath() :
                currentDir;
}


void FADTTSWindow::WarningPopUp( QString warningMessage )
{
    QMessageBox::warning( this, tr( "WARNING" ), tr( qPrintable( warningMessage ) ), QMessageBox::Ok );
}

void FADTTSWindow::CriticalPopUp( QString criticalMessage )
{
    QMessageBox::critical( this, tr( "CRITICAL ERROR" ), tr( qPrintable( criticalMessage ) ), QMessageBox::Ok );
}

void FADTTSWindow::DisplayIcon( QLabel *label , const QPixmap& icon )
{
    if( !icon.isNull() )
    {
        label->setPixmap( icon.scaled( QSize( m_iconSize, m_iconSize ), Qt::IgnoreAspectRatio ) );
    }
}


/***************************************************************/
/************************** Input tab **************************/
/***************************************************************/

/***********************  Private slots  ***********************/
void FADTTSWindow::OnSettingInputFile( int diffusionPropertyIndex )
{
    QLineEdit *lineEdit = m_inputTabInputFileLineEditMap[ diffusionPropertyIndex ];
    QString filePath = lineEdit->text();
    QFile file( filePath );

    if( filePath.isEmpty() )
    {
        m_inputTabIconLabelMap[ diffusionPropertyIndex ]->clear();
        m_data.ClearData( diffusionPropertyIndex );
    }
    else
    {
        if( !file.open( QIODevice::ReadOnly ) )
        {
            DisplayInputLineEditIcon( diffusionPropertyIndex, m_koPixmap );
            m_data.ClearData( diffusionPropertyIndex );
        }
        else
        {
            file.close();
            QList< QStringList > fileData = m_processing.GetDataFromFile( filePath );
            if( m_processing.IsMatrixDimensionOK( fileData ) )
            {
                DisplayInputLineEditIcon( diffusionPropertyIndex, m_okPixmap );

                m_data.SetFilename( diffusionPropertyIndex ) = filePath;
                m_data.SetFileData( diffusionPropertyIndex ) = fileData;
            }
            else
            {
                DisplayInputLineEditIcon( diffusionPropertyIndex, m_koPixmap );

                m_data.ClearData( diffusionPropertyIndex );

                QString criticalMessage = m_data.GetDiffusionPropertyName( diffusionPropertyIndex ).toUpper()
                        + " data file corrupted:<br><i>" + m_inputTabInputFileLineEditMap[ diffusionPropertyIndex ]->text()
                        + "</i><br>For each row, the number of columns is not constant.<br>Check the data file provided.";
                CriticalPopUp( criticalMessage );
            }

            UpdateCurrentDir( filePath, m_currentInputFileDir );
        }
    }
    UpdateInputFileInformation( diffusionPropertyIndex );

    if( diffusionPropertyIndex == m_data.GetSubMatrixIndex() )
    {
        m_editInputDialog->ResetSubjectColumnID(); /** By default Subjects are on the 1st column. **/
        SetInfoSubjectColumnID();
    }

    OnInputToggled();
}

void FADTTSWindow::OnAddInputFiles()
{
    QString dir = m_currentInputFileDir;
    QStringList fileList;
    fileList = QFileDialog::getOpenFileNames( this, tr( "Choose Input Files" ), dir, tr( ".csv( *.csv ) ;; .*( * )" ) );
    if( !fileList.isEmpty() )
    {
        UpdateLineEditsAfterAddingMultipleFiles( fileList );
    }
}

void FADTTSWindow::OnAddInputFile( int diffusionPropertyIndex )
{
    QLineEdit *lineEdit = m_inputTabInputFileLineEditMap[ diffusionPropertyIndex ];
    QString filePath = lineEdit->text();
    QDir dir = SetDir( filePath, m_currentInputFileDir );
    QString file = QFileDialog::getOpenFileName( this, tr( qPrintable( "Choose " + m_data.GetDiffusionPropertyName( diffusionPropertyIndex ).toUpper() + " File" ) ), dir.absolutePath(), tr( ".csv( *.csv ) ;; .*( * )" ) );
    if( !file.isEmpty() )
    {
        lineEdit->setText( file );
    }
}

void FADTTSWindow::OnEditInputFile( int diffusionPropertyIndex )
{
    if( m_data.GetFilename( diffusionPropertyIndex ).isEmpty() )
    {
        QString warningMessage = "<b>File Edition Unable</b><br>";
        warningMessage.append( m_inputTabInputFileLineEditMap[ diffusionPropertyIndex ]->text().isEmpty() ?
                                   "No file specified" :
                                   "Could not open the file:<br><i>" + m_inputTabInputFileLineEditMap[ diffusionPropertyIndex ]->text() + "</i>" );
        WarningPopUp( warningMessage );
    }
    else
    {
        LaunchEditInputDialog( diffusionPropertyIndex );
    }
}


void FADTTSWindow::OnCovariateClicked( QListWidgetItem *item )
{
    if( item->flags() == Qt::ItemIsEnabled )
    {
        if( item->checkState() == Qt::Unchecked )
        {
            item->setCheckState( Qt::Checked );
        }
        else if( !item->text().contains( "Intercept" ) )
        {
            item->setCheckState( Qt::Unchecked );
        }
        else
        {
            QString warningMessage = "You are about to uncheck the Intercept. This action is not recommended.<br>Are you sure you want to do it?";
            int ignoreWarning = QMessageBox::warning( this, tr( "Uncheck Intercept" ), tr( qPrintable( warningMessage ) ),
                                                      QMessageBox::Yes | QMessageBox::No, QMessageBox::No );

            if( ignoreWarning == QMessageBox::Yes )
            {
                item->setCheckState( Qt::Unchecked );
            }
        }
    }
}

void FADTTSWindow::OnCheckAllCovariates()
{
    SetCheckStateAllCovariates( Qt::Checked );
}

void FADTTSWindow::OnUnCheckAllCovariates()
{
    SetCheckStateAllCovariates( Qt::Unchecked );
}


void FADTTSWindow::OnUpdatingInputFile( const int &diffusionPropertyIndex, const QString& newFilePath )
{
    m_inputTabInputFileLineEditMap[ diffusionPropertyIndex ]->setText( newFilePath );
}

void FADTTSWindow::OnUpdatingSubjectColumnID( int newSubjectColumnID )
{
    /** Subjects are not on the 1st column anymore. **/
    m_data.SetSubjectColumnID() = newSubjectColumnID;
    UpdateInputFileInformation( m_data.GetSubMatrixIndex() );
    SetInfoSubjectColumnID();

    UpdateSubjectList();
}


/*********************** Private function ***********************/
void FADTTSWindow::SetCovariateListVisible( bool visible )
{
    m_covariateListWidget->setHidden( !visible );
    this->inputTab_covariatesCheckAll_pushButton->setHidden( !visible );
    this->inputTab_covariatesUncheckAll_pushButton->setHidden( !visible );
    this->inputTab_covariatesInformation_label->setHidden( !visible );
}

void FADTTSWindow::SetCovariateListWidget()
{
    m_covariateListWidget->clear();

    QMap< int, QString > covariates = m_data.GetCovariates();
    if( !covariates.isEmpty() )
    {
        QMap< int, QString >::ConstIterator iterCovariate = covariates.cbegin();
        while( iterCovariate != covariates.cend() )
        {
            QListWidgetItem *covariateItem = new QListWidgetItem( iterCovariate.value(), m_covariateListWidget );
            covariateItem->setCheckState( Qt::Checked );
            covariateItem->setFlags( Qt::ItemIsEnabled );
            m_covariateListWidget->addItem( covariateItem );
            ++iterCovariate;
        }

        m_covariateListWidget->setMaximumHeight( m_covariateListWidget->sizeHintForRow( 0 ) * ( m_covariateListWidget->count() + 1 ) );
    }
}

QString FADTTSWindow::GetInputFileInformation( int diffusionPropertyIndex ) const
{
    QString fileInformation;
    const QString filename = m_data.GetFilename( diffusionPropertyIndex );
    QString fileName =  QFileInfo( QFile( filename ) ).fileName();
    if( !fileName.isEmpty() )
    {
        fileInformation.append( tr( qPrintable( "Filename: <i>" + fileName + "</i><br>" ) ) );
        fileInformation.append( tr( qPrintable( "Number of subjects: <i>" + QString::number( m_data.GetNbrSubjects( diffusionPropertyIndex ) ) + "</i><br>" ) ) );
        fileInformation.append( tr( qPrintable( "Data matrix: <i>" + QString::number(  m_data.GetNbrRows( diffusionPropertyIndex ) ) + "x" + QString::number( m_data.GetNbrColumns( diffusionPropertyIndex ) )  + "</i><br>" ) ) );
        if( diffusionPropertyIndex == m_data.GetSubMatrixIndex() )
        {
            fileInformation.append( tr( qPrintable( "Number of covariates: <i>" + QString::number( m_data.GetCovariates().size()-1 )  + " ( + Intercept )</i>" ) ) );
        }
    }
    else
    {
        fileInformation.append( tr ( "<i>No File Information.<br>Please select a correct data file</i>" ) );
    }

    return fileInformation;
}

void FADTTSWindow::DisplayFileInformation()
{
    foreach( int diffusionPropertyIndex, m_data.GetDiffusionPropertiesIndices() )
    {
        QString info = tr( qPrintable( GetInputFileInformation( diffusionPropertyIndex ) ) );
        m_inputFileInformationLabelMap.value( diffusionPropertyIndex )->setText( info );

        if( diffusionPropertyIndex == m_data.GetSubMatrixIndex() )
        {
            SetCovariateListVisible( info.contains( "No File Information", Qt::CaseInsensitive )  ? false : true );
        }
    }
}

void FADTTSWindow::DisplayInputLineEditIcon( int diffusionPropertyIndex, const QPixmap& icon )
{
    DisplayIcon( m_inputTabIconLabelMap[ diffusionPropertyIndex ], icon );
}

void FADTTSWindow::UpdateInputFileInformation( int diffusionPropertyIndex )
{
    QList< QStringList > fileData = m_data.GetFileData( diffusionPropertyIndex );

    if( !fileData.isEmpty() )
    {
        m_data.SetNbrRows( diffusionPropertyIndex ) = fileData.count();
        m_data.SetNbrColumns( diffusionPropertyIndex ) = fileData.first().count();

        m_data.ClearSubjects( diffusionPropertyIndex );
        QStringList subjects = m_processing.GetSubjectsFromData( fileData, m_data.GetSubjectColumnID() );
        m_data.SetSubjects( diffusionPropertyIndex ) = subjects;
        m_data.SetNbrSubjects( diffusionPropertyIndex ) = subjects.count();

        if( diffusionPropertyIndex == m_data.GetSubMatrixIndex() )
        {
            m_data.ClearCovariates();
            m_data.SetCovariates() = m_processing.GetCovariatesFromData( fileData, m_data.GetSubjectColumnID() );
            /** Intercept representes everything that has not been classified in one of the previous
             *  covariates. It is important to add it as 1st element of m_covariatesList **/
            m_data.AddInterceptToCovariates();

            SetCovariateListWidget();
        }
    }

    UpdateAvailableDiffusionProperties( diffusionPropertyIndex );
    DisplayFileInformation();
}

void FADTTSWindow::UpdateLineEditsAfterAddingMultipleFiles( const QStringList& fileList )
{
    /** This function only works with filename that start with ad_, rd_, md_, fa_ and subMatrix_.
     *  If a prefix is detected more than once, the reated files will be ignored. **/
    QMap< int, QStringList > fileMap;
    foreach ( int diffusionPropertyIndex, m_data.GetDiffusionPropertiesIndices() )
    {
        foreach( QString file, fileList )
        {
            QString filename = QFileInfo( QFile( file ) ).fileName();
            if( filename.contains( "_" + m_data.GetDiffusionPropertyName( diffusionPropertyIndex ) + "_", Qt::CaseInsensitive ) ||
                    filename.startsWith( m_data.GetDiffusionPropertyName( diffusionPropertyIndex ) + "_", Qt::CaseInsensitive ) ||
                    filename.endsWith( "_" + m_data.GetDiffusionPropertyName( diffusionPropertyIndex ) + ".csv", Qt::CaseInsensitive ) )
            {
                ( fileMap[ diffusionPropertyIndex ] ).append( file );
            }
        }
    }

    foreach ( int diffusionPropertyIndex, m_data.GetDiffusionPropertiesIndices() )
    {
        m_inputTabInputFileLineEditMap[ diffusionPropertyIndex ]->clear();
    }

    QMap< int, QStringList >::ConstIterator iter = fileMap.cbegin();
    while( iter != fileMap.constEnd() )
    {
        int diffusionPropertyIndex = iter.key();
        m_inputTabInputFileLineEditMap[ diffusionPropertyIndex ]->setText( iter.value().size() == 1 ? iter.value().first() : "" );
        ++iter;
    }
}


void FADTTSWindow::SetInfoSubjectColumnID()
{
    this->inputTab_subjectColumnID_label->setText( !m_data.GetFileData( m_data.GetSubMatrixIndex() ).isEmpty() ?
                                                       tr( qPrintable( "<b><i><span style=""font-size:7pt;"">" + QString::number( m_data.GetSubjectColumnID() + 1 ) + "</span></i></b>" ) ) :
                                                       "" );
}


void  FADTTSWindow::LaunchEditInputDialog( int diffusionPropertyIndex )
{
    m_editInputDialog->setModal( true );
    m_editInputDialog->setWindowTitle( tr( qPrintable( "Edit " + m_data.GetDiffusionPropertyName( diffusionPropertyIndex).toUpper() + " File" ) ) );
    m_editInputDialog->DisplayDataEdition( diffusionPropertyIndex );
    m_editInputDialog->exec();
}


void FADTTSWindow::SetCheckStateAllCovariates( Qt::CheckState checkState )
{
    for( int i = 0; i < m_covariateListWidget->count(); i++ )
    {
        QListWidgetItem *currentItem = m_covariateListWidget->item( i );
        currentItem->setCheckState( ( checkState == Qt::Unchecked ) && ( currentItem->text() == "Intercept" )
                                    ? Qt::Checked : checkState );
    }
}


/****************************************************************/
/******************** Subjects/Covariates tab *******************/
/****************************************************************/

/***********************  Private slots  ************************/
void FADTTSWindow::OnApplyQCThreshold()
{
    QStringList matchedSubjects = GetCheckedMatchedSubjects();
    QMap< QString, QList< QStringList > > rawData = GetPropertyRawData();

    if( !matchedSubjects.isEmpty() )
    {
        bool initDiaolog = m_qcThresholdDialog->InitPlot( rawData, matchedSubjects, this->para_subjectTab_qcThreshold_doubleSpinBox->value() );

        if( initDiaolog )
        {
            m_qcThresholdDialog->setModal( true );
            m_qcThresholdDialog->setWindowTitle( tr( "Apply QC Threshold" ) );
            m_qcThresholdDialog->exec();
        }
    }
}

void FADTTSWindow::OnApplyingQCThreshold( const QStringList& subjectsCorrelated, const QStringList& subjectsNotCorrelated, double qcThreshold )
{
    m_failedQCThresholdSubjects.clear();
    m_failedQCThresholdSubjects = subjectsNotCorrelated;

    para_subjectTab_qcThreshold_doubleSpinBox->setValue( qcThreshold );
    m_qcThreshold = qcThreshold;

    for( int i = 0; i < m_matchedSubjectListWidget->count(); i++ )
    {
        QListWidgetItem *currentItem = m_matchedSubjectListWidget->item( i );
        if( m_failedQCThresholdSubjects.contains( currentItem->text() ) )
        {
            currentItem->setCheckState( Qt::Unchecked );
            currentItem->setBackgroundColor( m_grey );
        }

        if( subjectsCorrelated.contains( currentItem->text() ) )
        {
            currentItem->setCheckState( Qt::Checked );
            currentItem->setBackgroundColor( m_green );
        }
    }

    DisplayNbrSubjectSelected();
}


void FADTTSWindow::OnSearch()
{
    int nbrSubjectFind = SearchSubjects( m_matchedSubjectListWidget ) + SearchSubjects( m_unmatchedSubjectListWidget );
    this->subjectTab_nbrFound_label->
            setText( !this->subjectTab_search_lineEdit->text().isEmpty() ? "found " + QString::number( nbrSubjectFind ) : "" );
}

void FADTTSWindow::OnInputToggled()
{
    SetSelectedInputFiles();
    UpdateSubjectList();
}

void FADTTSWindow::OnSetCaseSensitivityToggled( bool checked )
{
    m_caseSensitivity = checked ? Qt::CaseSensitive : Qt::CaseInsensitive;
    OnSearch();
}


void FADTTSWindow::OnSubjectClicked( QListWidgetItem *item )
{
    if( item->flags() == Qt::ItemIsEnabled )
    {
        item->setCheckState( item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked );
        item->setBackgroundColor( item->checkState() == Qt::Checked ? m_green : m_grey );

        if( ( item->checkState() == Qt::Checked ) && m_failedQCThresholdSubjects.contains( item->text() ) )
        {
            m_failedQCThresholdSubjects.removeAll( item->text() );
        }
    }

    DisplayNbrSubjectSelected();
}

void FADTTSWindow::OnCheckAllVisibleSubjects()
{
    SetCheckStateAllVisibleSubjects( Qt::Checked );
}

void FADTTSWindow::OnUnCheckAllVisibleSubjects()
{
    SetCheckStateAllVisibleSubjects( Qt::Unchecked );
}

void FADTTSWindow::OnSaveCheckedSubjects()
{
    QString filePath = QFileDialog::getSaveFileName( this, tr( "Save subject list as ..." ), m_currentSubjectFileDir + "/newSubjectList.txt", tr( ".txt ( *.txt ) ;; .*( * )" ) );
    if( !filePath.isEmpty() )
    {
        SaveCheckedSubjects( filePath );
    }
}


void FADTTSWindow::OnLoadSubjectList()
{
    QLineEdit *lineEdit = m_subjectFileLineEdit;
    QString filePath = lineEdit->text();
    QDir dir = SetDir( filePath, m_currentSubjectFileDir );
    QString file = QFileDialog::getOpenFileName( this, tr( "Choose SubjectList File" ), dir.absolutePath(), tr( ".txt ( *.txt ) ;; .*( * )" ) );
    if( !file.isEmpty() )
    {
        lineEdit->setText( file );
    }
}

void FADTTSWindow::OnResetSubjectList()
{
    m_subjectFileLineEdit->clear();
}

void FADTTSWindow::OnSettingSubjectList( const QString& filePath )
{
    QLabel *label = this->subjectTab_iconLoadSubjectFile_label;
    m_loadedSubjects.clear();

    if( filePath.isEmpty() )
    {
        label->clear();
    }
    else
    {
        m_loadedSubjects = m_processing.GetSubjectsFromFileList( filePath );
        DisplayIcon( label, m_loadedSubjects.isEmpty() ? m_koPixmap : m_okPixmap );
        UpdateCurrentDir( filePath, m_currentSubjectFileDir );
    }
    m_areSubjectsLoaded = m_loadedSubjects.isEmpty() ? false : true;

    UpdateSubjectList();
}


/*********************** Private function ***********************/
void FADTTSWindow::SetSelectedInputFiles()
{
    m_propertySelected.clear();
    m_selectedFiles.clear();
    foreach ( int diffusionPropertyIndex, m_data.GetDiffusionPropertiesIndices() )
    {
        if( m_paramTabFileCheckBoxMap[ diffusionPropertyIndex ]->isChecked() && !m_data.GetFilename( diffusionPropertyIndex ).isEmpty() )
        {
            m_propertySelected.insert( diffusionPropertyIndex, m_data.GetDiffusionPropertyName( diffusionPropertyIndex ) );
            m_selectedFiles.insert( diffusionPropertyIndex, m_data.GetFilename( diffusionPropertyIndex ) );
        }
    }
}

void FADTTSWindow::UpdateAvailableDiffusionProperties( int diffusionPropertyIndex )
{
    //    QLabel *currentLabel = m_paramTabFileDataSizeLabelMap.value( diffusionPropertyIndex );
    QCheckBox *currentCheckBox = m_paramTabFileCheckBoxMap.value( diffusionPropertyIndex );
    int nbrRows = m_data.GetNbrSubjects( diffusionPropertyIndex );
    int nbrColumns = m_data.GetNbrColumns( diffusionPropertyIndex );
    int nbrSubjects = m_data.GetNbrSubjects( diffusionPropertyIndex );

    bool isDefine = !( ( nbrRows == 0 ) | ( nbrColumns == 0 ) | ( nbrSubjects == 0 ) );
    //    QString text = isDefine ? tr( qPrintable( QString::number( m_data.GetNbrSubjects( diffusionPropertyIndex ) ) + " subjects" ) ) :
    //                              tr( "N/A" );
    //    currentLabel->setEnabled( isDefine );
    //    currentLabel->setText( text );
    currentCheckBox->setEnabled( isDefine );
    currentCheckBox->setChecked( isDefine );
}

void FADTTSWindow::InitAvailableDiffusionProperties()
{
    foreach( int index, m_data.GetDiffusionPropertiesIndices() )
    {
        UpdateAvailableDiffusionProperties( index );
    }
}

QMap< int, bool > FADTTSWindow::GetDiffusionPropertiesCheckState()
{
    QMap< int, bool > diffusionPropertyCheckStatus;

    QMap< int, QCheckBox* >::ConstIterator iterCheckBoxMap = m_paramTabFileCheckBoxMap.cbegin();
    while( iterCheckBoxMap != m_paramTabFileCheckBoxMap.cend() )
    {
        diffusionPropertyCheckStatus.insert( iterCheckBoxMap.key(), iterCheckBoxMap.value()->isChecked() );
        ++iterCheckBoxMap;
    }

    return diffusionPropertyCheckStatus;
}


void FADTTSWindow::DisplaySubjectInformation()
{
    this->subjectTab_matchedSubjectsInformation_label->clear();
    this->subjectTab_unmatchedSubjectsInformation_label->clear();

    QString textMatchedSubjectsInformation;
    QString textUnmatchedSubjectsInformation;
    int nbrMatchedSubjects = m_matchedSubjectListWidget->count();
    int nbrUnmatchedSubjects = m_unmatchedSubjectListWidget->count();
    int nbrTotalSubjects = nbrMatchedSubjects + nbrUnmatchedSubjects;
    if( ( nbrMatchedSubjects != 0 ) || ( nbrUnmatchedSubjects != 0 ) )
    {
        if( ( nbrMatchedSubjects == 0 ) && ( nbrUnmatchedSubjects != 0 ) )
        {
            textMatchedSubjectsInformation = tr( qPrintable( "WARNING No matched! " + QString::number( nbrMatchedSubjects ) + "/" + QString::number( nbrTotalSubjects ) ) );
            textUnmatchedSubjectsInformation = tr( qPrintable( QString::number( nbrUnmatchedSubjects ) + "/" + QString::number( nbrTotalSubjects ) + " unmatched" ) );
        }
        else
        {
            if( ( nbrMatchedSubjects != 0 ) && ( nbrUnmatchedSubjects == 0 ) )
            {
                textMatchedSubjectsInformation = tr( qPrintable( "All subjects matched " + QString::number( nbrMatchedSubjects ) + "/" + QString::number( nbrTotalSubjects ) ) );
                textUnmatchedSubjectsInformation.clear();
            }
            else
            {
                textMatchedSubjectsInformation = tr( qPrintable( QString::number( nbrMatchedSubjects ) + "/" + QString::number( nbrTotalSubjects ) + " matched" ) );
                textUnmatchedSubjectsInformation = tr( qPrintable( QString::number( nbrUnmatchedSubjects ) + "/" + QString::number( nbrTotalSubjects ) + " unmatched" ) );
            }
        }
        this->subjectTab_matchedSubjectsInformation_label->setText( textMatchedSubjectsInformation );
        this->subjectTab_unmatchedSubjectsInformation_label->setText( textUnmatchedSubjectsInformation );
    }
    else
    {
        this->subjectTab_matchedSubjectsInformation_label->clear();
        this->subjectTab_unmatchedSubjectsInformation_label->clear();
    }
}

void FADTTSWindow::DisplayNbrSubjectSelected()
{
    this->subjectTab_checkedSubjectsInformation_label->clear();
    int nbrSubjectsSelected = 0;

    if( m_matchedSubjectListWidget->count() != 0 )
    {
        for( int i = 0; i < m_matchedSubjectListWidget->count(); i++ )
        {
            QListWidgetItem *item = m_matchedSubjectListWidget->item( i );

            if( item->checkState() == Qt::Checked )
            {
                nbrSubjectsSelected++;
            }
        }

        this->subjectTab_checkedSubjectsInformation_label->setText( tr( qPrintable( QString::number( nbrSubjectsSelected ) + " subjects selected") ) );
    }
}

void FADTTSWindow::DisplaySortedSubjects( const QStringList& matchedSubjects, const QMap< QString, QList< int > >& unMatchedSubjectMap )
{
    m_failedQCThresholdSubjects.clear();

    m_matchedSubjectListWidget->clear();
    m_matchedSubjectListWidget->setUpdatesEnabled( false );
    m_matchedSubjectListWidget->setSelectionMode( QAbstractItemView::NoSelection );
    foreach( QString subject, matchedSubjects )
    {
        QListWidgetItem *item = new QListWidgetItem( subject, m_matchedSubjectListWidget );
        item->setCheckState( Qt::Checked );
        item->setBackgroundColor( m_green );
        item->setFlags( Qt::ItemIsEnabled );
        m_matchedSubjectListWidget->addItem( item );
    }
    m_matchedSubjectListWidget->setUpdatesEnabled( true );

    m_unmatchedSubjectListWidget->clear();
    m_unmatchedSubjectListWidget->setUpdatesEnabled( false );
    m_unmatchedSubjectListWidget->setSelectionMode( QAbstractItemView::NoSelection );
    QMap< QString, QList< int > >::ConstIterator iterUnmatchedSubjects = unMatchedSubjectMap.cbegin();
    while( iterUnmatchedSubjects != unMatchedSubjectMap.constEnd() )
    {
        QStringList sortedText;
        foreach( int index, iterUnmatchedSubjects.value() )
        {
            if( index == -1 )
            {
                sortedText.append( "loaded subjects" );
            }
            else
            {
                sortedText.append( m_data.GetDiffusionPropertyName( index ) );
            }
        }

        QString text = tr( qPrintable( iterUnmatchedSubjects.key() + " --> " + sortedText.join( ", " ) ) );
        QListWidgetItem *item = new QListWidgetItem( text, m_unmatchedSubjectListWidget );
        item->setBackgroundColor( m_red );
        item->setTextColor( m_lightBlack );
        m_unmatchedSubjectListWidget->addItem( item );
        ++iterUnmatchedSubjects;
    }
    m_unmatchedSubjectListWidget->setUpdatesEnabled( true );

    DisplaySubjectInformation();
    DisplayNbrSubjectSelected();
}

void FADTTSWindow::UpdateSubjectList()
{
    QMap< int, bool > diffusionPropertiesCheckState = GetDiffusionPropertiesCheckState();
    QMap< int, QStringList > allSubjects = m_processing.GetSubjectsFromSelectedFiles( diffusionPropertiesCheckState, m_data.GetSubjects() );
    if( m_areSubjectsLoaded )
    {
        allSubjects.insert( -1, m_loadedSubjects );
    }

    QStringList allSubjectsList = m_processing.GetAllSubjects( allSubjects );

    QMap< QString, QMap< int, bool > > sortedSubjects = m_processing.SortSubjects( allSubjectsList, allSubjects );

    QStringList matchedSubjects;
    QMap< QString, QList< int > > unMatchedSubjects;
    m_processing.AssignSortedSubject( sortedSubjects, matchedSubjects, unMatchedSubjects );

    DisplaySortedSubjects( matchedSubjects, unMatchedSubjects );

    OnSearch();
}

void FADTTSWindow::SetCheckStateAllVisibleSubjects( Qt::CheckState checkState )
{
    for( int i = 0; i < m_matchedSubjectListWidget->count(); i++ )
    {
        QListWidgetItem* current = m_matchedSubjectListWidget->item( i );
        if( !current->isHidden() && current->flags() == Qt::ItemIsEnabled )
        {
            current->setCheckState( checkState );
            current->setBackgroundColor( checkState ? m_green : m_grey );

            if( ( current->checkState() == Qt::Checked ) && m_failedQCThresholdSubjects.contains( current->text() ) )
            {
                m_failedQCThresholdSubjects.removeAll( current->text() );
            }
        }
    }

    DisplayNbrSubjectSelected();
}

int FADTTSWindow::SearchSubjects( QListWidget *list )
{
    int nbrFound = 0;
    QLineEdit *lineEdit = this->subjectTab_search_lineEdit;
    QString search = lineEdit->text();
    QPalette palette;
    if( !search.isEmpty() )
    {
        if(  list->count() != 0  )
        {
            search.prepend( "*" );
            search.append( "*" );
            QRegExp regExp( search );
            regExp.setPatternSyntax( QRegExp::Wildcard );
            regExp.setCaseSensitivity( m_caseSensitivity );
            for( int i = 0; i < list->count(); i++ )
            {
                QListWidgetItem* current = list->item( i );
                if( regExp.exactMatch( current->text() ) )
                {
                    current->setHidden( false );
                    nbrFound++;
                }
                else
                {
                    current->setHidden( true );
                }
            }
        }
        palette.setColor( QPalette::Base, m_yellow );
        lineEdit->setPalette( palette );
    }
    else
    {
        if( list->count() != 0 )
        {
            for( int i = 0; i < list->count(); i++ )
            {
                list->item( i )->setHidden( false );
            }
        }
        palette.setColor( QPalette::Base, Qt::white );
        lineEdit->setPalette( palette );
    }
    return nbrFound;
}

void FADTTSWindow::SaveCheckedSubjects( QString filePath )
{
    QFile exportedTXT( filePath );
    exportedTXT.open( QIODevice::WriteOnly );
    QTextStream ts( &exportedTXT );

    for( int i = 0; i < m_matchedSubjectListWidget->count(); i++ )
    {
        if( m_matchedSubjectListWidget->item( i )->checkState() == Qt::Checked )
        {
            ts << m_matchedSubjectListWidget->item( i )->text() <<  endl;
        }
    }
    exportedTXT.flush();
    exportedTXT.close();

    QString previousFilePath = m_subjectFileLineEdit->text();
    m_subjectFileLineEdit->setText( filePath );
    if( previousFilePath == filePath )
    {
        /** If filePath does not change, an update is needed to display the right subject list **/
        UpdateSubjectList();
    }
}


QStringList FADTTSWindow::GetCheckedMatchedSubjects()
{
    QStringList matchedSubjects;
    QList< QListWidgetItem * > items = m_matchedSubjectListWidget->findItems( "*", Qt::MatchWrap | Qt::MatchWildcard );
    foreach( QListWidgetItem *currentItem, items )
    {
        if( currentItem->checkState() == Qt::Checked )
        {
            matchedSubjects.append( currentItem->text() );
        }
    }

    return matchedSubjects;
}

QMap< QString, QList< QStringList > > FADTTSWindow::GetPropertyRawData()
{
    QMap< QString, QList< QStringList > > fileData;
    foreach( int diffusionPropertyIndex, m_propertySelected.keys() )
    {
        if( m_data.GetDiffusionPropertyName( diffusionPropertyIndex ) != "submatrix" )
        {
            fileData.insert( m_data.GetDiffusionPropertyName( diffusionPropertyIndex ).toUpper(), m_data.GetFileData( diffusionPropertyIndex ) );
        }
    }

    return fileData;
}


/****************************************************************/
/************************ Execution  tab ************************/
/****************************************************************/

/***********************  Private  slots  ***********************/
void FADTTSWindow::OnBrowsingOutputDir()
{
    QLineEdit *lineEdit = this->para_executionTab_outputDir_lineEdit;
    QString filePath = lineEdit->text();
    QDir dir = filePath;

    QString dirPath;
    dirPath = ( dir.exists() && !filePath.isEmpty() ) ?
                QFileDialog::getExistingDirectory( this, tr( "Choose Output Directory" ), dir.absolutePath(), QFileDialog::ShowDirsOnly ) :
                QFileDialog::getExistingDirectory( this, tr( "Choose Output Directory" ), m_data.GetOutputDir(), QFileDialog::ShowDirsOnly );
    if( !dirPath.isEmpty() )
    {
        lineEdit->setText( dirPath );
    }
}

void FADTTSWindow::OnSettingOutputDir( const QString& path )
{
    QLabel *label = this->executionTab_iconOutputDir_label;
    if( !path.isEmpty() )
    {
        DisplayIcon( label, QDir( path ).exists() ? m_okPixmap : m_koPixmap );
        if( QDir( path ).exists() )
        {
            m_data.SetOutputDir() = path;
        }
    }
    else
    {
        label->clear();
    }
}

void FADTTSWindow::OnBrowsingMVCMPath()
{
    QLineEdit *lineEdit = this->para_executionTab_mvcm_lineEdit;
    QString filePath = lineEdit->text();
    QDir dir = filePath;

    QString dirPath = ( dir.exists() && !filePath.isEmpty() ) ?
                QFileDialog::getExistingDirectory( this, tr( "Choose MVCM Directory" ), dir.absolutePath(), QFileDialog::ShowDirsOnly ) :
                QFileDialog::getExistingDirectory( this, tr( "Choose MVCM Directory" ), m_mvcmPath, QFileDialog::ShowDirsOnly );
    if( !dirPath.isEmpty() )
    {
        lineEdit->setText( dirPath );
    }
}

void FADTTSWindow::OnSettingMVCMPath( const QString& path )
{
    QLabel *label = this->executionTab_iconMVCM_label;
    if( !path.isEmpty() )
    {
        QStringList nameFilterMVCM( "*MVCM*" );
        bool mvcmFilesExist = !QStringList( QStringList() << QDir( path ).entryList( nameFilterMVCM ) ).isEmpty();
        DisplayIcon( label, mvcmFilesExist ? m_okPixmap : m_koPixmap );
        if( mvcmFilesExist )
        {
            m_mvcmPath = path;
        }
    }
    else
    {
        label->clear();
    }
}

void FADTTSWindow::OnBrowsingMatlabExe()
{
    QLineEdit *lineEdit = this->soft_executionTab_matlabExe_lineEdit;
    QString filePath = lineEdit->text();
    QString file;
    QDir dir;

    dir = ( !filePath.isEmpty() && QDir( QFileInfo( QFile( filePath ) ).absolutePath() ).exists() ) ?
                QFileInfo( QFile( filePath ) ).absolutePath() : m_currentMatlabExeDir;

    file = QFileDialog::getOpenFileName( this, tr( "Choose Matlab Executable" ), dir.absolutePath() );
    if( !file.isEmpty() )
    {
        lineEdit->setText( file );
    }
}

void FADTTSWindow::OnSettingMatlabExe( const QString& executable )
{
    QFile matlabExe( executable );
    QLabel *label = this->executionTab_iconMatlabExe_label;
    if( executable.isEmpty() )
    {
        label->clear();
        m_isMatlabExeFound = false;
    }
    else
    {
        if( matlabExe.open( QIODevice::ReadOnly ) )
        {
            matlabExe.close();
            DisplayIcon( label, m_okPixmap );
            m_matlabThread->SetMatlabExe() = executable;
            m_isMatlabExeFound = true;
        }
        else
        {
            DisplayIcon( label, m_koPixmap );
            m_isMatlabExeFound = false;
        }

        UpdateCurrentDir( executable, m_currentMatlabExeDir );
    }
}


void FADTTSWindow::OnRunMatlabToggled( bool choice )
{
    this->executionTab_matlabExe_pushButton->setEnabled( choice );
    this->soft_executionTab_matlabExe_lineEdit->setEnabled( choice );
    this->executionTab_iconMatlabExe_label->setEnabled( choice );
    this->executionTab_nbrCompThreads_label->setEnabled( choice );
    this->soft_executionTab_nbrCompThreads_spinBox->setEnabled( choice );

    m_matlabThread->SetRunMatlab() = choice;
}


void FADTTSWindow::OnRun()
{ 
    SetSelectedCovariates();
    SetFibername();

    if( canFADTTSterBeRun() )
    {
        this->executionTab_run_pushButton->setEnabled( false );
        this->executionTab_stop_pushButton->setEnabled( true );
        m_progressBar->show();

        SetMatlabScript();

        m_matlabThread->start();

        m_log->AddText( "\nFile generation completed...\n" );
    }
}

void FADTTSWindow::OnStop()
{
    if( m_matlabThread->isRunning() )
    {
        QMessageBox::StandardButton closeBox =
                QMessageBox::question( this, tr( "FADTTSter" ), tr( "Data are still being processed.<br>Are you sure you want to stop the execution?" ),
                                       QMessageBox::No | QMessageBox::Yes, QMessageBox::No );
        switch( closeBox )
        {
        case QMessageBox::No:
            break;
        case QMessageBox::Yes:
        {
            m_log->AddText( "\nWarning! Thread terminated by user before completed matlab script.\n" );
            m_matlabThread->terminate();
            m_logWindow->insertPlainText( "Warning! Thread terminated by user before completed matlab script.\n");
            break;
        }
        default:
            break;
        }
    }
}

void FADTTSWindow::OnUpdatingLogActivity( const QString& LogActivity )
{
    QScrollBar *scrollBar = m_logWindow->verticalScrollBar();
    m_logWindow->insertPlainText( LogActivity );
    scrollBar->setValue( scrollBar->maximum() );
    //    if( scrollBar->value() == scrollBar->maximum() )
    //    {
    //        m_logWindow->insertPlainText( line );
    //        scrollBar->setValue( scrollBar->maximum() );
    //    }
    //    else
    //    {
    //        m_logWindow->insertPlainText( line );
    //    }
}

void FADTTSWindow::OnClearLog()
{
    m_logWindow->clear();
}


void FADTTSWindow::OnMatlabThreadFinished()
{
    this->executionTab_run_pushButton->setEnabled( true );
    this->executionTab_stop_pushButton->setEnabled( false );
    m_log->CloseLogFile();
    m_progressBar->hide();
}


/*********************** Private function ***********************/
void FADTTSWindow::GenerateFailedQCThresholdSubjectFile( QString outputDir )
{
    QFile failedQCThresholdSubjectFile( outputDir + "/" + this->para_executionTab_fiberName_lineEdit->text() + "_failed_QCThreshold_SubjectList.txt" );

    if( failedQCThresholdSubjectFile.open( QIODevice::WriteOnly ) )
    {
        QTextStream tsFailedQCThresholdSubjectFile( &failedQCThresholdSubjectFile );

        tsFailedQCThresholdSubjectFile << "After failing the QC Threshold (" << QString::number( m_qcThreshold ) << ") the following subjects were removed from the study:" << endl;

        for( int i = 0; i < m_failedQCThresholdSubjects.size(); i++ )
        {
            tsFailedQCThresholdSubjectFile << "- " << m_failedQCThresholdSubjects.at( i ) << endl;
        }
        failedQCThresholdSubjectFile.flush();
        failedQCThresholdSubjectFile.close();
    }
}

QStringList FADTTSWindow::GenerateSelectedSubjectFile( QString outputDir )
{
    QStringList selectedSubjects;
    QFile selectedSubjectFile( outputDir + "/" + this->para_executionTab_fiberName_lineEdit->text() + "_subjectList.txt" );
    m_nbrSelectedSubjects = 0;

    if( selectedSubjectFile.open( QIODevice::WriteOnly ) )
    {
        QTextStream tsSelectedSubjectFile( &selectedSubjectFile );
        for( int i = 0; i < m_matchedSubjectListWidget->count(); i++ )
        {
            if( m_matchedSubjectListWidget->item( i )->checkState() == Qt::Checked )
            {
                m_nbrSelectedSubjects++;
                tsSelectedSubjectFile << m_matchedSubjectListWidget->item( i )->text() << endl;
                selectedSubjects.append( m_matchedSubjectListWidget->item( i )->text() );
            }
        }
        selectedSubjectFile.flush();
        selectedSubjectFile.close();
    }

    if( !m_failedQCThresholdSubjects.isEmpty() )
    {
        GenerateFailedQCThresholdSubjectFile( outputDir );
    }

    return selectedSubjects;
}


void FADTTSWindow::SetSelectedCovariates()
{
    m_selectedCovariates.clear();

    if( m_paramTabFileCheckBoxMap.value( m_data.GetSubMatrixIndex() )->isChecked() )
    {
        for( int i = 0; i < m_covariateListWidget->count(); i++ )
        {
            QListWidgetItem *currentItem = m_covariateListWidget->item( i );
            if( currentItem->checkState() == Qt::Checked )
            {
                if( currentItem->text() == "Intercept" )
                {
                    m_selectedCovariates.insert( - 1, currentItem->text() );
                }
                else
                {
                    m_selectedCovariates.insert( m_data.GetCovariates().key( currentItem->text() ), currentItem->text() );
                }
            }
        }
    }
}

void FADTTSWindow::SetFibername()
{
    m_fibername = this->para_executionTab_fiberName_lineEdit->text();
}

bool FADTTSWindow::canFADTTSterBeRun()
{
    bool atLeastOneDiffusionPropertyEnabled = ( m_paramTabFileCheckBoxMap.value( m_data.GetAxialDiffusivityIndex() )->isEnabled() ||
                                                m_paramTabFileCheckBoxMap.value( m_data.GetRadialDiffusivityIndex() )->isEnabled() ||
                                                m_paramTabFileCheckBoxMap.value( m_data.GetMeanDiffusivityIndex() )->isEnabled() ||
                                                m_paramTabFileCheckBoxMap.value( m_data.GetFractionalAnisotropyIndex() )->isEnabled() );
    bool subMatrixEnabled = m_paramTabFileCheckBoxMap.value( m_data.GetSubMatrixIndex() )->isEnabled();

    bool atLeastOneDiffusionPropertyChecked = ( m_paramTabFileCheckBoxMap.value( m_data.GetAxialDiffusivityIndex() )->isChecked() ||
                                                m_paramTabFileCheckBoxMap.value( m_data.GetMeanDiffusivityIndex() )->isChecked() ||
                                                m_paramTabFileCheckBoxMap.value( m_data.GetRadialDiffusivityIndex() )->isChecked() ||
                                                m_paramTabFileCheckBoxMap.value( m_data.GetFractionalAnisotropyIndex() )->isChecked() );
    bool subMatrixChecked = m_paramTabFileCheckBoxMap.value( m_data.GetSubMatrixIndex() )->isChecked();

    bool atLeastOneCovariateChecked = m_selectedCovariates.count() != 0;

    bool fiberNameProvided = !m_fibername.isEmpty();

    bool mvcmPathSpecified = !this->para_executionTab_mvcm_lineEdit->text().isEmpty();
    bool matlabExeSpecified = !this->soft_executionTab_runMatlab_checkBox->isChecked() ? true : m_isMatlabExeFound;

    if( !atLeastOneDiffusionPropertyEnabled || !subMatrixEnabled ||
            !atLeastOneDiffusionPropertyChecked || !subMatrixChecked ||
            !atLeastOneCovariateChecked ||
            !fiberNameProvided ||
            !mvcmPathSpecified || !matlabExeSpecified )
    {
        QString warningText = "<b>FADTTSter will not be executed for the following reason(s):</b><br>";
        if( !atLeastOneDiffusionPropertyEnabled || !subMatrixEnabled )
        {
            warningText.append( "Inputs Tab<br>" );
            if( !atLeastOneDiffusionPropertyEnabled )
            {
                warningText.append( "- Provide at least 1 data file (AD, RD, MR or FA)<br>" );
            }
            if( !subMatrixEnabled )
            {
                warningText.append( "- No covariate file provided<br>" );
            }
        }
        if( atLeastOneDiffusionPropertyEnabled || subMatrixEnabled )
        {
            if( ( !atLeastOneDiffusionPropertyChecked && atLeastOneDiffusionPropertyEnabled ) || ( !subMatrixChecked && subMatrixEnabled ) || !atLeastOneCovariateChecked )
            {
                warningText.append( "Subjects / Covariates Tab<br>" );
                if( !atLeastOneDiffusionPropertyChecked && atLeastOneDiffusionPropertyEnabled )
                {
                    warningText.append( "- Select at least 1 data file (AD, RD, MR or FA)<br>" );
                }
                if( !subMatrixChecked && subMatrixEnabled )
                {
                    warningText.append( "- Covariate file not selected<br>" );
                }
                if( subMatrixChecked )
                {
                    if( !atLeastOneCovariateChecked )
                    {
                        warningText.append( "- Select at least 1 covariate<br>" );
                    }
                }
            }
        }
        if( !fiberNameProvided || !matlabExeSpecified || !mvcmPathSpecified )
        {
            warningText.append( "Execution Tab<br>" );
            if( !fiberNameProvided )
            {
                warningText.append( "- No fiber name provided<br>" );
            }
            if( !matlabExeSpecified )
            {
                warningText.append( "- Specify the matlab executable<br>" );
            }
            if( !mvcmPathSpecified )
            {
                warningText.append( "- Specify the path to FADTTS matlab function (MVCM)<br>" );
            }
        }
        WarningPopUp( warningText );
        return false;
    }
    else
    {
        return true;
    }
}

void FADTTSWindow::SetMatlabScript()
{
    QString outputDir = m_data.GetOutputDir() + "/FADTTSter_" + m_fibername;
    QDir().mkpath( outputDir );

    QMap< int, QString > matlabInputFiles = m_processing.GenerateMatlabInputs( outputDir,m_fibername, m_selectedFiles, m_propertySelected, m_selectedCovariates,
                                                                               m_data.GetSubjectColumnID(), GenerateSelectedSubjectFile( outputDir ) );

    m_matlabThread->InitMatlabScript( outputDir, "FADTTSterAnalysis_" + m_fibername + "_" + QString::number( this->para_executionTab_nbrPermutations_spinBox->value() ) + "perm.m" );
    m_matlabThread->SetHeader();
    m_matlabThread->SetNbrCompThreads( this->soft_executionTab_runMatlab_checkBox->isChecked(), this->soft_executionTab_nbrCompThreads_spinBox->value() );
    m_matlabThread->SetMVCMPath( this->para_executionTab_mvcm_lineEdit->text() );
    m_matlabThread->SetFiberName( m_fibername );
    m_matlabThread->SetDiffusionProperties( m_propertySelected.values() );
    m_matlabThread->SetNbrPermutation( this->para_executionTab_nbrPermutations_spinBox->value() );
    m_matlabThread->SetCovariates( m_selectedCovariates );
    m_matlabThread->SetInputFiles( matlabInputFiles );
    m_matlabThread->SetOmnibus( this->para_executionTab_omnibus_checkBox->isChecked() );
    m_matlabThread->SetPostHoc( this->para_executionTab_postHoc_checkBox->isChecked() );
    m_matlabThread->SetConfidenceBandsThreshold( this->para_executionTab_confidenceBandsThreshold_doubleSpinBox->value() );
    m_matlabThread->SetPvalueThreshold( this->para_executionTab_pvalueThreshold_doubleSpinBox->value() );

    m_log->SetLogFile( outputDir, m_fibername );
    m_log->SetFileWatcher();
    m_log->InitLog( outputDir, m_fibername, matlabInputFiles, m_selectedCovariates, m_loadedSubjects, m_subjectFileLineEdit->text(), m_nbrSelectedSubjects,
                    m_failedQCThresholdSubjects, m_qcThreshold, this->para_executionTab_nbrPermutations_spinBox->value(), this->para_executionTab_confidenceBandsThreshold_doubleSpinBox->value(),
                    this->para_executionTab_pvalueThreshold_doubleSpinBox->value(), this->para_executionTab_omnibus_checkBox->isChecked(), this->para_executionTab_postHoc_checkBox->isChecked(),
                    this->para_executionTab_mvcm_lineEdit->text(), this->soft_executionTab_runMatlab_checkBox->isChecked(), this->soft_executionTab_matlabExe_lineEdit->text(),
                    soft_executionTab_nbrCompThreads_spinBox->value() );
}


/****************************************************************/
/************************ Plotting  tab *************************/
/****************************************************************/

/***********************  Private  slots  ***********************/
void FADTTSWindow::OnBrowsingPlotDir()
{
    QLineEdit *lineEdit = this->para_plottingTab_loadSetDataTab_browsePlotDirectory_lineEdit;
    QString filePath = lineEdit->text();
    QDir dir = filePath;

    QString dirPath;
    dirPath = QFileDialog::getExistingDirectory( this, tr( "Choose Plot Directory" ), ( dir.exists() && !filePath.isEmpty() ) ? dir.absolutePath() : m_data.GetOutputDir(), QFileDialog::ShowDirsOnly );
    if( !dirPath.isEmpty() )
    {
        lineEdit->setText( dirPath );
    }
}

void FADTTSWindow::OnSettingPlotDir( const QString& path )
{
    QLabel *browsePlotDirIcon = this->plottingTab_loadSetDataTab_browsePlotDirectoryIcon_label;
    QLabel *plotDirInfo = this->plottingTab_loadSetDataTab_plotDirectoryInfo_label;
    QLabel *plotDirIcon = this->plottingTab_loadSetDataTab_plotDirectoryIcon_label;
    QLabel *fibernameIcon = this->plottingTab_loadSetDataTab_fibernameIcon_label;

    if( !path.isEmpty() )
    {
        bool pathExists = QDir( path ).exists();

        DisplayIcon( browsePlotDirIcon, pathExists ? m_okPixmap : m_koPixmap );

        if( pathExists )
        {
            QString containingFolder = QDir( path ).dirName();
            if( containingFolder.contains( "FADTTSter_" ) )
            {
                this->para_plottingTab_loadSetDataTab_fibername_lineEdit->setText( containingFolder.remove( "FADTTSter_" ) );
            }
            else
            {
                this->para_plottingTab_loadSetDataTab_fibername_lineEdit->clear();
            }
        }
        else
        {
            plotDirInfo->hide();
            plotDirIcon->clear();
            fibernameIcon->clear();
        }
    }
    else
    {
        browsePlotDirIcon->clear();
        plotDirInfo->hide();
        plotDirIcon->clear();
        fibernameIcon->clear();
    }

    SetPlotTab();
}

void FADTTSWindow::OnSettingPlotFibername( const QString& fibername )
{
    QLabel *plotDirInfo = this->plottingTab_loadSetDataTab_plotDirectoryInfo_label;
    QLabel *fibernameIcon = this->plottingTab_loadSetDataTab_fibernameIcon_label;

    if( plotDirInfo->isHidden() )
    {
        fibernameIcon->clear();
    }
    else
    {
        if( m_isPlotReady )
        {
            QString fibernameNoBlank = QString( fibername ).remove( " ", Qt::CaseInsensitive );
            DisplayIcon( fibernameIcon, !fibernameNoBlank.isEmpty() ? m_okPixmap : m_warningPixmap );
            m_plot->SetFibername() = fibernameNoBlank;
        }
        else
        {
            fibernameIcon->clear();
        }
    }
}


void FADTTSWindow::OnSettingPlotsUsed( const QStringList& plotsAvailable )
{
    m_plotComboBox->clear();
    if( !plotsAvailable.isEmpty() )
    {
        m_plotComboBox->addItem( "No Plot" );
        m_plotComboBox->addItems( plotsAvailable );
    }
}

void FADTTSWindow::OnSettingAllPropertiesUsed( const QMap< int, QString >& allPropertiesUsed )
{
    if( allPropertiesUsed.isEmpty() || allPropertiesUsed != m_previousPropertiesUsed )
    {
        SetPropertyEdition( allPropertiesUsed.values() );

        QString currentProperty = m_propertyComboBox->currentText();
        m_propertyComboBox->clear();
        m_propertyComboBox->addItem( "" );
        m_propertyComboBox->addItems( allPropertiesUsed.values() );
        m_propertyComboBox->setCurrentText( currentProperty );

        SetPropertiesForDisplay( allPropertiesUsed.values() );

        m_previousPropertiesUsed = allPropertiesUsed;
    }

}

void FADTTSWindow::OnSettingAllCovariatesUsed( const QMap< int, QString >& allCovariatesUsed )
{
    if( allCovariatesUsed.isEmpty() || allCovariatesUsed != m_previousCovariatesUsed )
    {
        SetCovariateEdition( allCovariatesUsed );
        SetCovariatesForDisplay( allCovariatesUsed );

        m_previousCovariatesUsed = allCovariatesUsed;
    }

    if( allCovariatesUsed == m_previousCovariatesUsed )
    {
        EditCovariatesNames();
    }
}


void FADTTSWindow::OnUpdatingCovariatesAvailable( const QMap< int, QString >& covariateAvailable )
{
    QString currentCovariate = m_covariateComboBox->currentText();
    m_covariateComboBox->clear();
    m_covariateComboBox->addItem( "" );
    QMap< int, QString >::ConstIterator iterCovariateAvailable = covariateAvailable.cbegin();
    while( iterCovariateAvailable != covariateAvailable.cend() )
    {
        m_covariateComboBox->addItem( iterCovariateAvailable.value() );
        ++iterCovariateAvailable;
    }
    m_covariateComboBox->setCurrentText( currentCovariate );
}

void FADTTSWindow::OnUpdatingPropertyPlotColor( const QString& property )
{
    displayMapType::Iterator currentIterator = m_propertiesForDisplay.begin();
    bool iterFound = false;
    while( !iterFound && currentIterator != m_propertiesForDisplay.end() )
    {
        if( currentIterator.value().first == property )
        {
            iterFound = true;
            currentIterator.value().second.second = m_propertiesColorsComboBoxMap.value( currentIterator.key() ).second->currentText();
        }
        else
        {
            ++currentIterator;
        }
    }
    m_currentLinesForDisplay = m_areLinesForDisplayProperties ? m_propertiesForDisplay : m_covariatesForDisplay;
}

void FADTTSWindow::OnUpdatingCovariatePlotColor( const QString& covariate )
{
    displayMapType::Iterator currentIterator = m_covariatesForDisplay.begin();
    bool iterFound = false;
    while( !iterFound && currentIterator != m_covariatesForDisplay.end() )
    {
        if( currentIterator.value().first == covariate )
        {
            iterFound = true;
            currentIterator.value().second.second = m_covariatesColorsComboBoxMap.value( currentIterator.key() ).second->currentText();
        }
        else
        {
            ++currentIterator;
        }
    }
    m_currentLinesForDisplay = m_areLinesForDisplayProperties ? m_propertiesForDisplay : m_covariatesForDisplay;
}


void FADTTSWindow::OnPlotSelection( const QString& plotSelected )
{
    m_plot->SetSelectedPlot( plotSelected );

    if( plotSelected == "No Plot" )
    {
        SetPlotOptions( false, false, false );
    }
    if( plotSelected == "Raw Data" || plotSelected == "Raw Stats" ||
            plotSelected == "Betas with Omnibus Confidence Bands" )
    {
        SetPlotOptions( true, true, true );
    }
    if( plotSelected == "Raw Betas by Properties" || plotSelected == "Omnibus FDR Significant Betas by Properties" ||
            plotSelected == "Post-Hoc FDR Significant Betas by Properties" )
    {
        SetPlotOptions( true, true, false );
    }
    if( plotSelected == "Raw Betas by Covariates" || plotSelected == "Omnibus FDR Significant Betas by Covariates" ||
            plotSelected == "Post-Hoc FDR Local pvalues by Covariates" || plotSelected == "Post-Hoc FDR Significant Betas by Covariates" )
    {
        SetPlotOptions( true, false, true );
    }
    if( plotSelected == "Omnibus Local pvalues" || plotSelected == "Omnibus FDR Local pvalues" )
    {
        SetPlotOptions( true, false, false );
    }

    m_plotSelected = plotSelected; // to remove later
}

void FADTTSWindow::OnPropertySelection( const QString& propertySelected )
{
    m_plot->SetSelectedProperty() = propertySelected;
}

void FADTTSWindow::OnCovariateSelection( const QString& covariateSelected )
{
    m_plot->SetSelectedCovariate() = covariateSelected;
}


void FADTTSWindow::OnLineForDisplayClicked( QListWidgetItem *item )
{
    if( item->flags() == Qt::ItemIsEnabled )
    {
        item->setCheckState( item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked );

        displayMapType::Iterator currentIterator = m_currentLinesForDisplay.begin();
        bool iterFound = false;
        while( !iterFound && ( currentIterator != m_currentLinesForDisplay.end() ) )
        {
            if( currentIterator.value().first == item->text() )
            {
                iterFound = true;
                currentIterator.value().second.first = item->checkState();
            }
            else
            {
                ++currentIterator;
            }
        }
    }

    if( m_areLinesForDisplayProperties )
    {
        m_propertiesForDisplay = m_currentLinesForDisplay;
    }
    else
    {
        m_covariatesForDisplay = m_currentLinesForDisplay;
    }

    m_plot->UpdateLineToDisplay( m_currentLinesForDisplay );
}

void FADTTSWindow::OnCheckAllLinesToDisplay()
{
    SetCheckStateLinesToDisplay( Qt::Checked );

    m_plot->UpdateLineToDisplay( m_currentLinesForDisplay );
}

void FADTTSWindow::OnUncheckAllToDisplay()
{
    SetCheckStateLinesToDisplay( Qt::Unchecked );

    m_plot->UpdateLineToDisplay( m_currentLinesForDisplay );
}


void FADTTSWindow::OnSettingLinesSelected( const QStringList& linesSelected )
{
    m_lineSelectedListWidget->clear();

    if( linesSelected.isEmpty() )
    {
        m_lineSelectedListWidget->hide();
    }
    else
    {
        QListWidgetItem *item = new QListWidgetItem( QString::number( linesSelected.size() ) + " line(s) selected:", m_lineSelectedListWidget );
        m_lineSelectedListWidget->addItem( item );

        for( int i = 1; i < linesSelected.size() + 1; i++ )
        {
            QListWidgetItem *currentLineSelectedItem = new QListWidgetItem( QString( "- " + linesSelected.at( i - 1 ) ), m_lineSelectedListWidget );
            m_lineSelectedListWidget->addItem( currentLineSelectedItem );
        }

        m_lineSelectedListWidget->setMaximumHeight( m_lineSelectedListWidget->sizeHintForRow( 0 ) * ( m_lineSelectedListWidget->count() + 1 ) );

        m_lineSelectedListWidget->show();
    }
}


void FADTTSWindow::OnUseCustomizedTitle( bool checkState )
{
    this->plottingTab_titleAxisLegendTab_titleName_lineEdit->setEnabled( checkState );
    this->plottingTab_titleAxisLegendTab_titleBold_checkBox->setEnabled( checkState );
    this->plottingTab_titleAxisLegendTab_titleItalic_checkBox->setEnabled( checkState );
    this->plottingTab_titleAxisLegendTab_titleSize_label->setEnabled( checkState );
    this->plottingTab_titleAxisLegendTab_titleSize_doubleSpinBox->setEnabled( checkState );
}

void FADTTSWindow::OnUpdatingPlotTitle()
{
    UpdatePlotTitle();
}


void FADTTSWindow::OnUpdatingPlotGrid( bool checkState )
{
    m_plot->UpdateGrid( checkState );
}

void FADTTSWindow::OnUseCustomizedAxis( bool checkState )
{
    this->plottingTab_titleAxisLegendTab_xName_label->setEnabled( checkState );
    this->plottingTab_titleAxisLegendTab_xName_lineEdit->setEnabled( checkState );
    this->plottingTab_titleAxisLegendTab_yName_label->setEnabled( checkState );
    this->plottingTab_titleAxisLegendTab_yName_lineEdit->setEnabled( checkState );
    this->plottingTab_titleAxisLegendTab_axisBold_checkBox->setEnabled( checkState );
    this->plottingTab_titleAxisLegendTab_axisItalic_checkBox->setEnabled( checkState );
    this->plottingTab_titleAxisLegendTab_yMin_checkBox->setEnabled( checkState );
    this->plottingTab_titleAxisLegendTab_yMin_doubleSpinBox->setEnabled( checkState && this->plottingTab_titleAxisLegendTab_yMin_checkBox->isChecked() );
    this->plottingTab_titleAxisLegendTab_yMax_checkBox->setEnabled( checkState );
    this->plottingTab_titleAxisLegendTab_yMax_doubleSpinBox->setEnabled( checkState && this->plottingTab_titleAxisLegendTab_yMax_checkBox->isChecked() );
}

void FADTTSWindow::OnYMinToggled( bool checkState )
{
    QDoubleSpinBox* yMinDoubleSpinBox = this->plottingTab_titleAxisLegendTab_yMin_doubleSpinBox;
    QDoubleSpinBox* yMaxDoubleSpinBox = this->plottingTab_titleAxisLegendTab_yMax_doubleSpinBox;

    yMinDoubleSpinBox->setEnabled( checkState );

    if( checkState )
    {
        yMaxDoubleSpinBox->setMinimum( yMinDoubleSpinBox->value() );
    }
    else
    {
        yMaxDoubleSpinBox->setMinimum( - 100 );
    }
}

void FADTTSWindow::OnYMaxToggled( bool checkState )
{
    QDoubleSpinBox* yMinDoubleSpinBox = this->plottingTab_titleAxisLegendTab_yMin_doubleSpinBox;
    QDoubleSpinBox* yMaxDoubleSpinBox = this->plottingTab_titleAxisLegendTab_yMax_doubleSpinBox;

    yMaxDoubleSpinBox->setEnabled( checkState );

    if( checkState )
    {
        yMinDoubleSpinBox->setMaximum( yMaxDoubleSpinBox->value() );
    }
    else
    {
        yMinDoubleSpinBox->setMaximum( 100 );
    }
}

void FADTTSWindow::OnYMinValueChanged( double yMinValue )
{
    this->plottingTab_titleAxisLegendTab_yMax_doubleSpinBox->setMinimum( yMinValue );
}

void FADTTSWindow::OnYMaxValueChanged( double yMaxValue )
{
    this->plottingTab_titleAxisLegendTab_yMin_doubleSpinBox->setMaximum( yMaxValue );
}

void FADTTSWindow::OnUpdatingPlotAxis()
{
    UpdatePlotAxis();
}


void FADTTSWindow::OnUpdatingLegend( const QString& legendPosition )
{
    m_plot->SetLegend( legendPosition );
}


void FADTTSWindow::OnUpdatingPvalueThreshold( double pvalueThreshold )
{
    m_plot->SetPvalueThreshold() = pvalueThreshold;
    m_plot->UpdatePvalueThresold( this->plottingTab_titleAxisLegendTab_useCustomizedTitle_checkBox->isChecked() );
}

void FADTTSWindow::OnUpdatingLineWidth( double lineWidth )
{
    m_plot->SetLineWidth() = lineWidth;
    m_plot->UpdateLineWidth();
}

void FADTTSWindow::OnUpdatingSelectedColorLine( const QString& color )
{
    m_plot->UpdateSelectedLineColor( color );
}

void FADTTSWindow::OnUpdatingMarkerType( const QString& markerType )
{
    m_plot->SetMarkerType( markerType );
    m_plot->UpdateMarker();
}

void FADTTSWindow::OnUpdatingMarkerSize( double markerSize )
{
    m_plot->SetMarkerSize() = markerSize;
    m_plot->UpdateMarker();
}


void FADTTSWindow::OnDisplayPlot()
{
    m_plot->ClearPlot();
    m_lineSelectedListWidget->hide();

    if( m_plotSelected.contains( "by Covariates" ) )
    {
        AddLinesForDisplay( true );
    }
    else
    {
        if(  m_plotSelected.contains( "by Properties" ) ||  m_plotSelected.contains( "Local pvalues" ) )
        {
            AddLinesForDisplay( false );
        }
    }

    EditCovariatesNames();
    UpdatePlot();

    bool isPlotDisplayed = m_plot->DisplayPlot();
    this->plottingTab_savePlot_pushButton->setEnabled( isPlotDisplayed );
    this->plottingTab_resetPlot_pushButton->setEnabled( isPlotDisplayed );

    bool isVisible = isPlotDisplayed && ( m_plotSelected.contains( "Betas by" ) || m_plotSelected.contains( "Local pvalues" ) );
    SetSelectionLinesDisplayedVisible( isVisible );
}

void FADTTSWindow::OnResetPlot()
{
    m_plot->ClearPlot();
    this->plottingTab_savePlot_pushButton->setEnabled( false );
    this->plottingTab_resetPlot_pushButton->setEnabled( false );
}

void FADTTSWindow::OnLoadPlotSettings()
{
    QString filename = QFileDialog::getOpenFileName( this , tr( "Load Plot Configuration" ) , "" , tr( ".json( *.json ) ;; .*( * )" ) );
    if( !filename.isEmpty() )
    {
        LoadPlotSettings( filename );
        EditCovariatesNames();
    }
}

void FADTTSWindow::OnSavePlotSettings()
{
    QString filePath = QFileDialog::getSaveFileName( this, tr( qPrintable( "Save Plot Settings as ..." ) ), "newPlotConfiguration.json", tr( ".json( *.json ) ;; .*( * )" ) );
    if( !filePath.isEmpty() )
    {
        SavePlotSettings( filePath );
    }
}



/***********************  Private  functions  ***********************/
void FADTTSWindow::SetColorsComboBox( QComboBox* &comboBox )
{
    QStringList colors;
    colors.append( QStringList() << "Red" << "Lime" << "Blue" << "Carolina Blue"
                   << "Yellow" << "Cyan" << "Magenta"
                   << "Olive" << "Teal" << "Purple"
                   << "Rosy Brown" << "Dark Sea Green" << "Corn Flower Blue"
                   << "Maroon" << "Green" << "Navy"
                   << "Orange" << "Mint" << "Pink"
                   << "Brown" << "Black" );
    comboBox->addItems( colors );
}

void FADTTSWindow::ResetPropertyEdition()
{
    nameLabelMapType::ConstIterator iterPropertiesColorsLabel = m_propertiesNameLabelMap.cbegin();
    while( iterPropertiesColorsLabel != m_propertiesNameLabelMap.cend() )
    {
        delete iterPropertiesColorsLabel.value().second;
        ++iterPropertiesColorsLabel;
    }
    m_propertiesNameLabelMap.clear();

    comboBoxMapType::ConstIterator iterPropertiesColorsComboBoxMap = m_propertiesColorsComboBoxMap.cbegin();
    while( iterPropertiesColorsComboBoxMap != m_propertiesColorsComboBoxMap.cend() )
    {
        delete iterPropertiesColorsComboBoxMap.value().second;
        ++iterPropertiesColorsComboBoxMap;
    }
    m_propertiesColorsComboBoxMap.clear();
}

void FADTTSWindow::SetPropertyEdition( const QStringList& propertiesAvailable )
{
    ResetPropertyEdition();

    int i = 0;
    foreach( QString property, propertiesAvailable )
    {
        QLabel *newLabel = new QLabel( property );
        newLabel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );

        QComboBox *newComboBox = new QComboBox();
        SetColorsComboBox( newComboBox );
        newComboBox->setCurrentIndex( i );

        gridLayout_PropertiesColors->addWidget( newLabel, i, 0 );
        gridLayout_PropertiesColors->addWidget( newComboBox, i, 1 );
        gridLayout_PropertiesColors->addItem( new QSpacerItem( 27, 27, QSizePolicy::Expanding, QSizePolicy::Fixed ), i, 2 );

        QPair< QString, QLabel* > currentLabelPair;
        currentLabelPair.first = property;
        currentLabelPair.second = newLabel;
        m_propertiesNameLabelMap.insert( i, currentLabelPair );

        QPair< QString, QComboBox* > currentComboBoxPair;
        currentComboBoxPair.first = property;
        currentComboBoxPair.second = newComboBox;
        m_propertiesColorsComboBoxMap.insert( i, currentComboBoxPair );
        i++;
    }

    QSignalMapper *signalMapperUpdatePropertyPlotColor = new QSignalMapper( this );
    connect( signalMapperUpdatePropertyPlotColor, SIGNAL( mapped( const QString& ) ), this, SLOT( OnUpdatingPropertyPlotColor( const QString& ) ) );
    comboBoxMapType::ConstIterator iterPropertyColor = m_propertiesColorsComboBoxMap.cbegin();
    while( iterPropertyColor != m_propertiesColorsComboBoxMap.cend() )
    {
        connect( iterPropertyColor.value().second, SIGNAL( currentIndexChanged( const QString& ) ), signalMapperUpdatePropertyPlotColor,SLOT(map() ) );
        signalMapperUpdatePropertyPlotColor->setMapping( iterPropertyColor.value().second, iterPropertyColor.value().first );
        ++iterPropertyColor;
    }
}

void FADTTSWindow::SetPropertiesForDisplay( const QStringList& propertiesForDisplay )
{
    m_propertiesForDisplay.clear();
    int i = 0;
    foreach( QString property, propertiesForDisplay )
    {
        QPair< bool, QString > currentPairSelectedColor;
        currentPairSelectedColor.first = true;
        currentPairSelectedColor.second = m_propertiesColorsComboBoxMap.value( i ).second->currentText();

        QPair< QString, QPair< bool, QString > > currentPair;
        currentPair.first = property;
        currentPair.second = currentPairSelectedColor;

        m_propertiesForDisplay.insert( i, currentPair );
        i++;
    }
}

void FADTTSWindow::ResetCovariateEdition()
{
    nameLabelMapType::ConstIterator iterCovariatesColorsLabel = m_covariatesNameLabelMap.cbegin();
    while( iterCovariatesColorsLabel != m_covariatesNameLabelMap.cend() )
    {
        delete iterCovariatesColorsLabel.value().second;
        ++iterCovariatesColorsLabel;
    }
    m_covariatesNameLabelMap.clear();

    comboBoxMapType::ConstIterator iterCoveriatesColorsComboBoxMap = m_covariatesColorsComboBoxMap.cbegin();
    while( iterCoveriatesColorsComboBoxMap != m_covariatesColorsComboBoxMap.cend() )
    {
        delete iterCoveriatesColorsComboBoxMap.value().second;
        ++iterCoveriatesColorsComboBoxMap;
    }
    m_covariatesColorsComboBoxMap.clear();

    covariateNameLineEditMapType::ConstIterator iterCoveriatesColorsLineEdit = m_covariatesNameLineEditMap.cbegin();
    while( iterCoveriatesColorsLineEdit != m_covariatesNameLineEditMap.cend() )
    {
        delete iterCoveriatesColorsLineEdit.value().second;
        ++iterCoveriatesColorsLineEdit;
    }
    m_covariatesNameLineEditMap.clear();
}

void FADTTSWindow::SetCovariateEdition( const QMap< int, QString >& allCovariatesUsed )
{
    ResetCovariateEdition();

    QMap< int, QString >::ConstIterator iterCovariateForDisplay = allCovariatesUsed.cbegin();
    while( iterCovariateForDisplay != allCovariatesUsed.cend() )
    {
        QLabel *newLabel = new QLabel( iterCovariateForDisplay.value() );
        newLabel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );

        QComboBox *newComboBox = new QComboBox();
        SetColorsComboBox( newComboBox );
        newComboBox->setCurrentIndex( iterCovariateForDisplay.key() );

        QLineEdit *newLineEdit = new QLineEdit();

        gridLayout_CovariatesEdition->addWidget( newLabel, iterCovariateForDisplay.key(), 0 );
        gridLayout_CovariatesEdition->addWidget( newComboBox, iterCovariateForDisplay.key(), 1 );
        gridLayout_CovariatesEdition->addWidget( newLineEdit, iterCovariateForDisplay.key(), 2 );

        QPair< QString, QLabel* > currentLabelPair;
        currentLabelPair.first = iterCovariateForDisplay.value();
        currentLabelPair.second = newLabel;
        m_covariatesNameLabelMap.insert( iterCovariateForDisplay.key(), currentLabelPair );

        QPair< QString, QComboBox* > currentComboBoxPair;
        currentComboBoxPair.first = iterCovariateForDisplay.value();
        currentComboBoxPair.second = newComboBox;
        m_covariatesColorsComboBoxMap.insert( iterCovariateForDisplay.key(), currentComboBoxPair );

        QPair< QString, QLineEdit* > currentLineEditPair;
        currentLineEditPair.first = iterCovariateForDisplay.value();
        currentLineEditPair.second = newLineEdit;
        m_covariatesNameLineEditMap.insert( iterCovariateForDisplay.key(), currentLineEditPair );

        ++iterCovariateForDisplay;
    }

    QSignalMapper *signalMapperUpdateCovariatePlotColor = new QSignalMapper( this );
    connect( signalMapperUpdateCovariatePlotColor, SIGNAL( mapped( const QString& ) ), this, SLOT( OnUpdatingCovariatePlotColor( const QString& ) ) );
    comboBoxMapType::ConstIterator iterCovariateColor = m_covariatesColorsComboBoxMap.cbegin();
    while( iterCovariateColor != m_covariatesColorsComboBoxMap.cend() )
    {
        connect( iterCovariateColor.value().second, SIGNAL( currentIndexChanged( const QString& ) ), signalMapperUpdateCovariatePlotColor,SLOT(map() ) );
        signalMapperUpdateCovariatePlotColor->setMapping( iterCovariateColor.value().second, iterCovariateColor.value().first );
        ++iterCovariateColor;
    }
}

void FADTTSWindow::SetCovariatesForDisplay( const QMap< int, QString >& covariatesForDisplay )
{
    m_covariatesForDisplay.clear();
    QMap< int, QString >::ConstIterator iterCovariateForDisplay = covariatesForDisplay.cbegin();
    while( iterCovariateForDisplay != covariatesForDisplay.cend() )
    {
        QPair< bool, QString > currentPairSelectedColor;
        currentPairSelectedColor.first = true;
        currentPairSelectedColor.second = m_covariatesColorsComboBoxMap.value( iterCovariateForDisplay.key() ).second->currentText();

        QPair< QString, QPair< bool, QString > > currentPair;
        currentPair.first = iterCovariateForDisplay.value();
        currentPair.second = currentPairSelectedColor;

        m_covariatesForDisplay.insert( iterCovariateForDisplay.key(), currentPair );
        ++iterCovariateForDisplay;
    }
}


void FADTTSWindow::SetPlotOptions( bool isPlotSelected, bool propertySelectionAvailable, bool covariateSelectionAvailable )
{
    this->plottingTab_displayPlot_pushButton->setEnabled( isPlotSelected );

    m_propertyComboBox->setEnabled( propertySelectionAvailable );
    this->plottingTab_loadSetDataTab_propertySelection_label->setEnabled( propertySelectionAvailable );

    m_covariateComboBox->setEnabled( covariateSelectionAvailable );
    this->plottingTab_loadSetDataTab_covariateSelection_label->setEnabled( covariateSelectionAvailable );
}

void FADTTSWindow::AddLinesForDisplay( bool isSelectionProperties )
{
    m_lineDisplayedListWidget->clear();
    m_areLinesForDisplayProperties = isSelectionProperties;
    m_currentLinesForDisplay = m_areLinesForDisplayProperties ? m_propertiesForDisplay : m_covariatesForDisplay;
    displayMapType::ConstIterator iterLinesForDisplay = m_currentLinesForDisplay.cbegin();
    while( iterLinesForDisplay != m_currentLinesForDisplay.cend() )
    {
        QListWidgetItem *covariateItem = new QListWidgetItem( iterLinesForDisplay.value().first, m_lineDisplayedListWidget );
        covariateItem->setCheckState( iterLinesForDisplay.value().second.first ? Qt::Checked : Qt::Unchecked );
        covariateItem->setFlags( Qt::ItemIsEnabled );
        m_lineDisplayedListWidget->addItem( covariateItem );
        ++iterLinesForDisplay;
    }
    if( m_plotComboBox->currentText() == "Omnibus Local pvalues" ||
            m_plotComboBox->currentText() == "Omnibus FDR Local pvalues" )
    {
        m_lineDisplayedListWidget->item( 0 )->setHidden( true );
    }

    m_lineDisplayedListWidget->setMaximumHeight( m_lineDisplayedListWidget->sizeHintForRow( 0 ) * ( m_lineDisplayedListWidget->count() + 1 ) );
}

void FADTTSWindow::SetCheckStateLinesToDisplay( Qt::CheckState checkState )
{
    for( int i = 0; i < m_lineDisplayedListWidget->count(); i++ )
    {
        QListWidgetItem * currentItem = m_lineDisplayedListWidget->item( i );
        currentItem->setCheckState( checkState );
        displayMapType::Iterator currentIterator = m_currentLinesForDisplay.begin();
        while( currentIterator != m_currentLinesForDisplay.end() )
        {
            currentIterator.value().second.first = currentItem->checkState();
            ++currentIterator;
        }
    }
    if( m_areLinesForDisplayProperties )
    {
        m_propertiesForDisplay = m_currentLinesForDisplay;
    }
    else
    {
        m_covariatesForDisplay = m_currentLinesForDisplay;
    }
}

void FADTTSWindow::SetSelectionLinesDisplayedVisible( bool visible )
{
    this->plottingTab_loadSetDataTab_linesToDisplay_label->setHidden( !visible );
    m_lineDisplayedListWidget->setHidden( !visible );
    this->plottingTab_loadSetDataTab_checkAllLinesToDisplay_pushButton->setHidden( !visible );
    this->plottingTab_loadSetDataTab_uncheckAllLinesToDisplay_pushButton->setHidden( !visible );
}

void FADTTSWindow::EditCovariatesNames()
{
    covariateNameLineEditMapType::ConstIterator iterCovariatesName = m_covariatesNameLineEditMap.cbegin();
    displayMapType::ConstIterator iterCovariatesForDisplay = m_covariatesForDisplay.cbegin();
    QMap< int, QString > newCovariatesNames;
    int currentIndex = m_covariateComboBox->currentIndex();

    int i = 0;
    while( iterCovariatesName != m_covariatesNameLineEditMap.cend() )
    {
        QString newCovariateName = iterCovariatesName.value().second->text().isEmpty() ? iterCovariatesName.value().first : iterCovariatesName.value().second->text();
        m_covariatesForDisplay[ iterCovariatesName.key() ].first = newCovariateName;
        newCovariatesNames.insert( iterCovariatesName.key(), newCovariateName );
        ++iterCovariatesName;
        ++iterCovariatesForDisplay;
    }
    m_plot->UpdateCovariatesNames( newCovariatesNames );
    m_covariateComboBox->setCurrentIndex( currentIndex );

    AddLinesForDisplay( m_areLinesForDisplayProperties );
}


void FADTTSWindow::UpdatePlotAxis()
{
    if( this->plottingTab_titleAxisLegendTab_useCustomizedAxis_checkBox->isChecked() )
    {
        m_plot->SetCustomizedAxis( this->plottingTab_titleAxisLegendTab_xName_lineEdit->text(),
                                   this->plottingTab_titleAxisLegendTab_yName_lineEdit->text(),
                                   this->plottingTab_titleAxisLegendTab_axisBold_checkBox->isChecked(),
                                   this->plottingTab_titleAxisLegendTab_axisItalic_checkBox->isChecked(),
                                   this->plottingTab_titleAxisLegendTab_yMin_checkBox->isChecked(),
                                   this->plottingTab_titleAxisLegendTab_yMin_doubleSpinBox->value(),
                                   this->plottingTab_titleAxisLegendTab_yMax_checkBox->isChecked(),
                                   this->plottingTab_titleAxisLegendTab_yMax_doubleSpinBox->value() );
    }
    else
    {
        m_plot->SetDefaultAxis();
    }
}

void FADTTSWindow::UpdatePlotTitle()
{
    if( this->plottingTab_titleAxisLegendTab_useCustomizedTitle_checkBox->isChecked() )
    {
        m_plot->SetCustomizedTitle( this->plottingTab_titleAxisLegendTab_titleName_lineEdit->text(),
                                    this->plottingTab_titleAxisLegendTab_titleBold_checkBox->isChecked(),
                                    this->plottingTab_titleAxisLegendTab_titleItalic_checkBox->isChecked(),
                                    this->plottingTab_titleAxisLegendTab_titleSize_doubleSpinBox->value() );
    }
    else
    {
        m_plot->SetDefaultTitle();
    }
}

void FADTTSWindow::UpdatePlot()
{
    m_plot->SetSelectionToDisPlay() = m_currentLinesForDisplay;
    m_plot->SetCovariatesProperties( m_covariatesForDisplay );

    m_plot->SetGrid() = this->plottingTab_titleAxisLegendTab_gridOn_checkBox->isChecked();
    UpdatePlotAxis();

    m_plot->SetLegend( this->plottingTab_titleAxisLegendTab_legendPosition_comboBox->currentText() );

    m_plot->SetPvalueThreshold() = this->plottingTab_editionTab_pvalueThreshold_doubleSpinBox->value();

    m_plot->SetLineWidth() = this->plottingTab_editionTab_lineWidth_doubleSpinBox->value();
    m_plot->SetSelectedLineColor( this->plottingTab_editionTab_selectedLineColor_comboBox->currentText() );

    m_plot->SetMarkerType( this->plottingTab_editionTab_markerType_comboBox->currentText() );
    m_plot->SetMarkerSize() = this->plottingTab_editionTab_markerSize_doubleSpinBox->value();

    UpdatePlotTitle();
}


void FADTTSWindow::ResetPlotTab()
{
    m_plotComboBox->setCurrentText( "No Plot" );

    this->plottingTab_titleAxisLegendTab_titleName_lineEdit->clear();

    this->plottingTab_displayPlot_pushButton->setEnabled( false );
    this->plottingTab_resetPlot_pushButton->setEnabled( false );
    this->plottingTab_savePlot_pushButton->setEnabled( false );

    this->plottingTab_loadPlotSettings_pushButton->setEnabled( false );
    this->plottingTab_savePlotSettings_pushButton->setEnabled( false );
}

void FADTTSWindow::SetPlotTab()
{
    m_isPlotReady = false;
    m_plot->ClearPlot();
    m_plot->ResetPlotData();
    m_lineSelectedListWidget->hide();
    SetSelectionLinesDisplayedVisible( false );

    QLabel *fibernameIcon = this->plottingTab_loadSetDataTab_fibernameIcon_label;
    QLabel *plotDirInfo = this->plottingTab_loadSetDataTab_plotDirectoryInfo_label;
    QLabel *plotDirIcon = this->plottingTab_loadSetDataTab_plotDirectoryIcon_label;
    QString fibername = this->para_plottingTab_loadSetDataTab_fibername_lineEdit->text().remove( " ", Qt::CaseInsensitive );
    QString directory = this->para_plottingTab_loadSetDataTab_browsePlotDirectory_lineEdit->text();

    if( QDir( directory ).exists() && !directory.isEmpty() )
    {
        m_isPlotReady = m_plot->InitPlot( directory, fibername );
    }
    else
    {
        ResetPlotTab();
    }

    if( m_isPlotReady )
    {
        DisplayIcon( fibernameIcon, !fibername.isEmpty() ? m_okPixmap : m_warningPixmap );
    }
    else
    {
        fibernameIcon->clear();
    }
    if( !directory.isEmpty() )
    {
        plotDirInfo->show();
        DisplayIcon( plotDirIcon, m_isPlotReady ? m_okPixmap : m_koPixmap );
    }

    this->plottingTab_loadSetDataTab_set_widget->setEnabled( m_isPlotReady );
    this->plottingTab_titleAxisLegendTab->setEnabled( m_isPlotReady );
    this->plottingTab_editionTab->setEnabled( m_isPlotReady );
    this->plottingTab_loadPlotSettings_pushButton->setEnabled( m_isPlotReady );
    this->plottingTab_savePlotSettings_pushButton->setEnabled( m_isPlotReady );

    SetPlotOptions( false, false, false );
}

void FADTTSWindow::LoadPlotSettings( QString filePath )
{
    QString text;
    QFile file( filePath );
    file.open( QIODevice::ReadOnly | QIODevice::Text );
    text = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson( text.toUtf8() );
    QJsonObject jsonObject = jsonDoc.object();

    /*** Title/Axis/Legend Data Tab ***/
    QJsonObject title = jsonObject.value( "title" ).toObject();
    this->plottingTab_titleAxisLegendTab_useCustomizedTitle_checkBox->setChecked( title.value( "customizedTitle" ).toBool() );
    this->plottingTab_titleAxisLegendTab_titleBold_checkBox->setChecked( title.value( "bold" ).toBool() );
    this->plottingTab_titleAxisLegendTab_titleItalic_checkBox->setChecked( title.value( "italic" ).toBool() );
    this->plottingTab_titleAxisLegendTab_titleSize_doubleSpinBox->setValue( title.value( "size" ).toDouble() );

    QJsonObject axis = jsonObject.value( "axis" ).toObject();
    this->plottingTab_titleAxisLegendTab_gridOn_checkBox->setChecked( axis.value( "showGrid" ).toBool() );
    this->plottingTab_titleAxisLegendTab_useCustomizedAxis_checkBox->setChecked( axis.value( "customizedAxis" ).toBool() );
    this->plottingTab_titleAxisLegendTab_xName_lineEdit->setText( axis.value( "xName" ).toString() );
    this->plottingTab_titleAxisLegendTab_yName_lineEdit->setText( axis.value( "yName" ).toString() );
    this->plottingTab_titleAxisLegendTab_axisBold_checkBox->setChecked( axis.value( "bold" ).toBool() );
    this->plottingTab_titleAxisLegendTab_axisItalic_checkBox->setChecked( axis.value( "italic" ).toBool() );
    this->plottingTab_titleAxisLegendTab_yMin_checkBox->setChecked( axis.value( "yMin" ).toObject().value( "set" ).toBool() );
    this->plottingTab_titleAxisLegendTab_yMin_doubleSpinBox->setValue( axis.value( "yMin" ).toObject().value( "value" ).toDouble() );
    this->plottingTab_titleAxisLegendTab_yMax_checkBox->setChecked( axis.value( "yMax" ).toObject().value( "set" ).toBool() );
    this->plottingTab_titleAxisLegendTab_yMax_doubleSpinBox->setValue( axis.value( "yMax" ).toObject().value( "value" ).toDouble() );

    QJsonObject legend = jsonObject.value( "legend" ).toObject();
    this->plottingTab_titleAxisLegendTab_legendPosition_comboBox->setCurrentText( legend.value( "position" ).toString() );

    /*** Edition Tab ***/
    QJsonObject pvalueThreshold = jsonObject.value( "pvalueThreshold" ).toObject();
    this->plottingTab_editionTab_pvalueThreshold_doubleSpinBox->setValue( pvalueThreshold.value( "value" ).toDouble() );

    QJsonObject line = jsonObject.value( "line" ).toObject();
    this->plottingTab_editionTab_lineWidth_doubleSpinBox->setValue( line.value( "lineWidth" ).toDouble() );
    this->plottingTab_editionTab_selectedLineColor_comboBox->setCurrentText( line.value( "colorSelection" ).toString() );

    QJsonObject marker = jsonObject.value( "marker" ).toObject();
    this->plottingTab_editionTab_markerType_comboBox->setCurrentText( marker.value( "type" ).toString() );
    this->plottingTab_editionTab_markerSize_doubleSpinBox->setValue( marker.value( "size" ).toDouble() );

    /*** properties Edition ***/
    QJsonArray propertiesEdition = jsonObject.value( "propertiesEdition" ).toArray();
    QMap< int, QString > properties;
    for( int i = 0; i < m_propertiesColorsComboBoxMap.size(); i++ )
    {
        QString currentProperty = m_propertiesColorsComboBoxMap.value( i ).first;
        properties.insert( i, currentProperty );
    }

    for( int i = 0; i < propertiesEdition.size(); i++ )
    {
        QJsonObject currentPropertyObject = propertiesEdition.at( i ).toObject();
        QString currentProperty = currentPropertyObject.value( "name" ).toString();
        if( properties.values().contains( currentProperty ) )
        {
            m_propertiesColorsComboBoxMap.value( properties.key( currentProperty ) ).second->setCurrentText( currentPropertyObject.value( "color" ).toString() );
        }
    }

    /*** covariates Edition ***/
    QJsonArray covariatesEdition = jsonObject.value( "covariatesEdition" ).toArray();
    QMap< int, QString > covariates;
    for( int i = 0; i < m_covariatesColorsComboBoxMap.size(); i++ )
    {
        QString currentCovariate = m_covariatesColorsComboBoxMap.value( i ).first;
        covariates.insert( i, currentCovariate );
    }

    for( int i = 0; i < covariatesEdition.size(); i++ )
    {
        QJsonObject currentCovariateObject = covariatesEdition.at( i ).toObject();
        QString currentCovariate = currentCovariateObject.value( "name" ).toString();
        if( covariates.values().contains( currentCovariate ) )
        {
            m_covariatesColorsComboBoxMap.value( covariates.key( currentCovariate ) ).second->setCurrentText( currentCovariateObject.value( "color" ).toString() );
            m_covariatesNameLineEditMap.value( covariates.key( currentCovariate ) ).second->setText( currentCovariateObject.value( "newName" ).toString() );
        }
    }
}

void FADTTSWindow::SavePlotSettings( QString filePath )
{
    QString text;
    QFile refJSON( QString( ":/ConfigurationFiles/Resources/ConfigurationFiles/plotConfiguration.json" ) );
    refJSON.open( QIODevice::ReadOnly | QIODevice::Text );
    text = refJSON.readAll();

    QJsonDocument jsonDoc = QJsonDocument::fromJson( text.toUtf8() );
    QJsonObject jsonObject = jsonDoc.object();


    /*** Title/Axis/Legend Data Tab ***/
    QJsonObject title = jsonObject[ "title" ].toObject();
    title[ "customizedTitle" ] = this->plottingTab_titleAxisLegendTab_useCustomizedTitle_checkBox->isChecked();
    title[ "bold" ] = this->plottingTab_titleAxisLegendTab_titleBold_checkBox->isChecked();
    title[ "italic" ] = this->plottingTab_titleAxisLegendTab_titleItalic_checkBox->isChecked();
    title[ "size" ] = this->plottingTab_titleAxisLegendTab_titleSize_doubleSpinBox->value();
    jsonObject[ "title" ] = title;

    QJsonObject axis = jsonObject[ "axis" ].toObject();
    axis[ "showGrid" ] = this->plottingTab_titleAxisLegendTab_gridOn_checkBox->isChecked();
    axis[ "customizedAxis" ] = this->plottingTab_titleAxisLegendTab_useCustomizedAxis_checkBox->isChecked();
    axis[ "xName" ] = this->plottingTab_titleAxisLegendTab_xName_lineEdit->text();
    axis[ "yName" ] = this->plottingTab_titleAxisLegendTab_yName_lineEdit->text();
    axis[ "bold" ] = this->plottingTab_titleAxisLegendTab_axisBold_checkBox->isChecked();
    axis[ "italic" ] = this->plottingTab_titleAxisLegendTab_axisItalic_checkBox->isChecked();
    axis[ "yMin" ].toObject()[ "set" ] = this->plottingTab_titleAxisLegendTab_yMin_checkBox->isChecked();
    axis[ "yMin" ].toObject()[ "value" ] = this->plottingTab_titleAxisLegendTab_yMin_doubleSpinBox->value();
    axis[ "yMax" ].toObject()[ "set" ] = this->plottingTab_titleAxisLegendTab_yMax_checkBox->isChecked();
    axis[ "yMax" ].toObject()[ "value" ] = this->plottingTab_titleAxisLegendTab_yMax_doubleSpinBox->value();
    jsonObject[ "axis" ] = axis;

    QJsonObject legend = jsonObject[ "legend" ].toObject();
    legend[ "position" ] = this->plottingTab_titleAxisLegendTab_legendPosition_comboBox->currentText();
    jsonObject[ "legend" ] = legend;

    /*** Edition Tab ***/
    QJsonObject pvalueThreshold = jsonObject[ "pvalueThreshold" ].toObject();
    pvalueThreshold[ "value" ] = this->plottingTab_editionTab_pvalueThreshold_doubleSpinBox->value();
    jsonObject[ "pvalueThreshold" ] = pvalueThreshold;

    QJsonObject line = jsonObject[ "line" ].toObject();
    line[ "lineWidth" ] = this->plottingTab_editionTab_lineWidth_doubleSpinBox->value();
    line[ "colorSelection" ] = this->plottingTab_editionTab_selectedLineColor_comboBox->currentText();
    jsonObject[ "line" ] = line;

    QJsonObject marker = jsonObject[ "marker" ].toObject();
    marker[ "type" ] = this->plottingTab_editionTab_markerType_comboBox->currentText();
    marker[ "size" ] = this->plottingTab_editionTab_markerSize_doubleSpinBox->value();
    jsonObject[ "marker" ] = marker;

    /*** properties Edition ***/
    QJsonArray propertiesEdition = jsonObject[ "propertiesEdition" ].toArray();

    comboBoxMapType::ConstIterator iterPropertyColor = m_propertiesColorsComboBoxMap.cbegin();
    while( iterPropertyColor != m_propertiesColorsComboBoxMap.cend() )
    {
        QJsonObject currentProperty;
        currentProperty.insert( "name", iterPropertyColor.value().first );
        currentProperty.insert( "color", iterPropertyColor.value().second->currentText() );

        propertiesEdition.append( currentProperty );

        ++iterPropertyColor;
    }
    jsonObject[ "propertiesEdition" ] = propertiesEdition;

    /*** covariates Edition ***/
    QJsonArray covariatesEdition = jsonObject[ "covariatesEdition" ].toArray();

    comboBoxMapType::ConstIterator iterCovariateColor = m_covariatesColorsComboBoxMap.cbegin();
    covariateNameLineEditMapType::ConstIterator iterCovariateName = m_covariatesNameLineEditMap.cbegin();
    while( iterCovariateColor != m_covariatesColorsComboBoxMap.cend() )
    {
        QJsonObject currentCovariate;
        currentCovariate.insert( "name", iterCovariateColor.value().first );
        currentCovariate.insert( "color", iterCovariateColor.value().second->currentText() );
        currentCovariate.insert( "newName", iterCovariateName.value().second->text() );

        covariatesEdition.append( currentCovariate );

        ++iterCovariateColor;
        ++iterCovariateName;
    }
    jsonObject[ "covariatesEdition" ] = covariatesEdition;


    jsonDoc.setObject( jsonObject );
    QFile exportedJSON( filePath );
    exportedJSON.open( QIODevice::WriteOnly | QIODevice::Text );
    exportedJSON.write( jsonDoc.toJson( QJsonDocument::Indented ) );
    exportedJSON.flush();
    exportedJSON.close();
}
