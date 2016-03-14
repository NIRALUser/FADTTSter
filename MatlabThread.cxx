#include "MatlabThread.h"

const QString MatlabThread::m_csvSeparator = QLocale().groupSeparator();

MatlabThread::MatlabThread(QObject *parent) :
    QThread(parent)
{
}


/***************************************************************/
/*************************** Script ****************************/
/***************************************************************/

/************ Public Functions ************/
void MatlabThread::InitMatlabScript( QString outputDir, QString matlabScriptName )
{
    m_outputDir = outputDir;
    m_matlabScriptName = matlabScriptName;
    m_matlabScript.clear();
//    QResource resource( ":/MatlabFiles/Resources/MatlabFiles/MatlabScriptRef.m" );
    QResource resource( ":/MatlabFiles/Resources/MatlabFiles/MatlabScriptRefWithPlots.m" );
    QFile matlabScriptRef( resource.absoluteFilePath() );
    if ( !matlabScriptRef.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        QString criticalError = "Unable to open resource file: " + matlabScriptRef.fileName() +
                " because of error \"" + matlabScriptRef.errorString() + "\"";
        std::cerr << criticalError.toStdString() << std::endl;
    }
    else
    {
        QTextStream ts( &matlabScriptRef );
        m_matlabScript = ts.readAll();
        matlabScriptRef.close();
    }
}


void MatlabThread::SetHeader()
{
    m_matlabScript.replace( "$version$", QString( FADTTS_VERSION ).prepend( "V" ) );
    m_matlabScript.replace( "$date$", QDate::currentDate().toString( "MM/dd/yyyy" ) );
    m_matlabScript.replace( "$time$", QTime::currentTime().toString( "hh:mm ap" ) );
}


void MatlabThread::SetNbrCompThreads( bool isRunOnSystem, int nbrComp )
{
    m_matlabScript.replace( "$nbrCompThreads$", isRunOnSystem ? "maxNumCompThreads( " + QString::number( nbrComp ) + " );" : "% Option ignored");
}

void MatlabThread::SetMVCMPath( QString mvcmPath )
{
    m_matlabScript.replace( "$addMVCMPath$", mvcmPath );
}


void MatlabThread::SetFiberName( QString fiberName )
{
    m_matlabScript.replace( "$fiberName$", "fiberName = \'" + fiberName + "\';\n" );
}

void MatlabThread::SetDiffusionProperties( QStringList selectedPrefixes )
{
    QString diffusionProperties;
    QString listDiffusionProperties;
    listDiffusionProperties.append( "Dnames = cell( " + QString::number( selectedPrefixes.size() ) + ", 1 );\n" );

    int i = 1;
    foreach ( QString prefID, selectedPrefixes )
    {
        diffusionProperties.append( prefID.toUpper() + " = \'" + prefID.toUpper() + "\';\n" );

        listDiffusionProperties.append( "Dnames{ " + QString::number( i ) + " } = " + prefID.toUpper() + ";\n" );
        i++;
    }

    m_matlabScript.replace( "$diffusionProperties$", diffusionProperties );

    m_matlabScript.replace( "$listDiffusionProperties$", listDiffusionProperties );
}

void MatlabThread::SetInputFiles( const QMap< int, QString >& csvInputFiles )
{
    QString diffusionFiles;
    QString diffusionData;
    diffusionData.append("diffusionFiles = cell( " + QString::number( csvInputFiles.size() - 1 ) + ", 1 );\n");

    int i = 1;
    QMap< int, QString >::ConstIterator iterMatlabInputFile = csvInputFiles.cbegin();
    while( iterMatlabInputFile != csvInputFiles.cend() )
    {
        QString filename = QFileInfo( QFile( iterMatlabInputFile.value() ) ).fileName();
        if( !filename.contains( "_subMatrix_", Qt::CaseInsensitive ) )
        {
            diffusionFiles.append( filename.split( "." ).first() + " = strcat( loadingFolder, \'/" + filename + "\' );\n" );
            diffusionData.append( "dataFiber" + QString::number( i ) + "All = dlmread( " + filename.split( "." ).first() +
                                   ", \'" + m_csvSeparator + "\', 1, 0 );\n" );
            diffusionData.append( "diffusionFiles{ " + QString::number( i ) + " } = dataFiber" + QString::number( i ) + "All( :, 2:end );\n" );
            i++;
        }
        else
        {
            m_matlabScript.replace( "$subMatrixFile$", filename.split( "." ).first() + " = strcat( loadingFolder, \'/" + filename + "\' );" );
            m_matlabScript.replace( "$subMatrixData$", "data2 = dlmread( " + filename.split( "." ).first() + ", \'" + m_csvSeparator + "\', 1, 1);" );
        }
        ++iterMatlabInputFile;
    }
    m_matlabScript.replace( "$diffusionFiles$", diffusionFiles );
    m_matlabScript.replace( "$diffusionData$", diffusionData );
}

void MatlabThread::SetCovariates( const QMap< int, QString > &selectedCovariates )
{
    m_matlabScript.replace( "$nbrCovariates$", "nbrCovariates = " + QString::number( selectedCovariates.count() ) + ";" );
    QString covariates;
    QString listCovariates;
    int i = 1;
    QMap< int, QString >::ConstIterator iterCovariate = selectedCovariates.cbegin();
    while( iterCovariate != selectedCovariates.cend() )
    {
        covariates.append( iterCovariate.value() + " = \'" + iterCovariate.value() + "\';\n" );
        listCovariates.append( "Cnames{ " + QString::number( i ) + " } = " + iterCovariate.value() + ";\n" );
        ++iterCovariate;
        i++;
    }
    m_matlabScript.replace( "$covariates$", covariates );
    m_matlabScript.replace( "$listCovariates$", listCovariates );
}


void MatlabThread::SetNbrPermutation( int nbrPermutation )
{
    m_matlabScript.replace( "$nbrPermutations$", "nbrPermutations = " + QString::number( nbrPermutation ) + ";" );
}

void MatlabThread::SetOmnibus( bool omnibus )
{
    m_matlabScript.replace( "$omnibus$", "omnibus = " + QString::number( omnibus ) + ";" );
}

void MatlabThread::SetPostHoc( bool postHoc )
{
    m_matlabScript.replace( "$postHoc$", "postHoc = " + QString::number( postHoc ) + ";" );
}

void MatlabThread::SetConfidenceBandsThreshold( double confidenceBandsThreshold )
{
    m_matlabScript.replace( "$confidenceBandsThreshold$", "confidenceBandsThreshold = " + QString::number( confidenceBandsThreshold ) + ";" );
}

void MatlabThread::SetPvalueThreshold( double pvalueThreshold )
{
    m_matlabScript.replace( "$pvalueThreshold$", "pvalueThreshold = " + QString::number( pvalueThreshold ) + ";" );
}


/*********** Private  Functions ***********/
void MatlabThread::GenerateMyFDR()
{
    QResource resource( ":/MatlabFiles/Resources/MatlabFiles/myFDR.m" );
    QFile matlabFunctionResource( resource.absoluteFilePath() );
    QFile matlabFunction( m_outputDir + "/myFDR.m" );

    if ( !matlabFunctionResource.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        QString criticalError = "Unable to open resource file: " + matlabFunctionResource.fileName() +
                " because of error \"" + matlabFunctionResource.errorString() + "\"";
        std::cerr << criticalError.toStdString() << std::endl;
    }
    else
    {
        if( matlabFunction.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
            QTextStream tsMatlabFunctionResource( &matlabFunctionResource );
            QTextStream tsMatlabFunction( &matlabFunction );
            tsMatlabFunction << tsMatlabFunctionResource.readAll();
            matlabFunction.flush();
            matlabFunction.close();
            matlabFunctionResource.close();
        }
    }
}

QString MatlabThread::GenerateMatlabFiles()
{
    QDir().mkpath( m_outputDir + "/MatlabOutputs" );
    m_matlabScriptPath = m_outputDir + "/" + m_matlabScriptName;
    QFile matlabScript( m_matlabScriptPath );
    if( matlabScript.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QTextStream ts( &matlabScript );
        ts << m_matlabScript;
        matlabScript.flush();
        matlabScript.close();
    }

    GenerateMyFDR();

    return m_matlabScriptPath;
}


/***************************************************************/
/*************************** Thread ****************************/
/***************************************************************/

/************ Public Functions ************/
void MatlabThread::SetLogFile( QFile *logFile )
{
    m_logFile = logFile;
}


QString& MatlabThread::SetMatlabExe()
{
    return m_matlabExe;
}

bool& MatlabThread::SetRunMatlab()
{
    return m_runMatlab;
}


void MatlabThread::terminate()
{
    m_process->terminate();
    QThread::terminate();
}


/*********** Private  Functions ***********/
void MatlabThread::RedirectOutput()
{
    m_process = new QProcess();
    m_process->setProcessChannelMode( QProcess::MergedChannels );
    m_process->setStandardOutputFile( m_logFile->fileName(), QProcess::Append );
}

void MatlabThread::RunScript()
{
    QStringList arguments;
    QString mScript = "run('" + m_matlabScriptPath + "')";
    std::cout << mScript.toStdString() << std::endl;
    arguments << "-nosplash" << "-nodesktop" << QString( "-r \"try, " + mScript + "; catch, disp('failed'), end, quit\"" ) << "-logfile matlabLog.out";

    m_process->start( m_matlabExe, arguments );
    m_process->waitForFinished();
}


void MatlabThread::run()
{
    m_matlabScriptPath.clear();
    GenerateMatlabFiles();
    if( m_runMatlab )
    {
        RedirectOutput();
        RunScript();
    }
}
