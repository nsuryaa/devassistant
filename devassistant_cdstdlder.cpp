#include "devassistant_cdstdlder.h"

QString DevAssistant_CdStdLder::loadOfficeCodingStandardsFromFile()
{
    QFile qfCodingStandards(":/standards/office_coding_standards.txt");

    if(!qfCodingStandards.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to load office coding standards";
        return QString();
    }

    QTextStream qtsInput(&qfCodingStandards);
    qtsInput.setCodec("UTF-8");
    return qtsInput.readAll();
}
