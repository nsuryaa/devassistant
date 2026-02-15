#ifndef DEVASSISTANT_RVWPRPTBDR_H
#define DEVASSISTANT_RVWPRPTBDR_H

#include "devassistant_global.h"

class DevAssistant_RvwPrptBdr
{
public:
    static QString buildPromptReviewCodeGiveComments(
            const QString &qsSelectedCode,
            const QString &qsCodingStandards);

    static QString buildPromptReplaceCode(
            const QString &qsSelectedCode,
            const QString &qsCodingStandards);

    static QString buildPromptExplainCode(
            const QString &qsSelectedCode);

    static QString buildPromptOptimizeCode(
            const QString &qsSelectedCode);
};

#endif // DEVASSISTANT_RVWPRPTBDR_H
