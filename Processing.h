#ifndef PROCESSING_H
#define PROCESSING_H

#include "Data.h"

#include <QDate>
#include <QObject>
#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QCheckBox>
#include <QMap>


class Processing : public QObject
{
    Q_OBJECT

public:
    explicit Processing( QObject *parent = 0 );


    QList< QStringList > GetDataFromFile( QString filePath ); // Tested


    bool IsMatrixDimensionOK( const QList< QStringList >& data ); // Tested

    bool IsSubMatrix(const QStringList& dataSecondRow ); // Tested


    QStringList GetSubjectsFromFileList( QString filePath ); // Tested

    QStringList GetSubjectsFromData( const QList< QStringList >& data, int subjectColumnID ); // Tested

    QMap< int, QString > GetCovariatesFromData( QList< QStringList > data, int subjectColumnID ); //Tested


    QStringList GetAllSubjects( const QMap< int, QStringList >& subjectsMap ); // Tested

    QMap< int, QStringList > GetSubjectsFromSelectedFiles( const QMap< int, bool >& diffusionPropertiesCheckState, const QMap< int, QStringList >& subjectsMap ); // Tested

    QMap< QString, QMap< int, bool > > SortSubjects( const QStringList& allSubjectsList, const QMap< int, QStringList >& allSubjects ); // Tested

    void AssignSortedSubject( const QMap<QString, QMap< int, bool > >& checkedSubjects, QStringList& matchedSubjects, QMap<QString, QList<int> >& unMatchedSubjects ); // Tested


    QMap< int, QString > GenerateMatlabInputs( QString outputDir, QString fiberName,
                                               const QMap< int, QString >& inputs, const QMap< int, QString >& properties,
                                               const QMap< int, QString >& covariates, int subjectColumnID, const QStringList& subjects); // Tested


private:
    static const QString m_csvSeparator;
};

#endif // PROCESSING_H
