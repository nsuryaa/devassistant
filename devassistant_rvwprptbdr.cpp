#include "devassistant_rvwprptbdr.h"

QString DevAssistant_RvwPrptBdr::buildPromptReviewCodeGiveComments(
        const QString &qsSelectedCode,
        const QString &qsCodingStandards)
{
    return QString(
                "SYSTEM ROLE:\n"
                "You are a C/C++ code reviewer.\n"
                "You detect standard violations ONLY.\n\n"

                "DO NOT:\n"
                "- Rewrite code\n"
                "- Suggest fixes\n"
                "- Explain rules\n"
                "- Praise code\n"
                "- Summarize\n\n"

                "YOU MUST:\n"
                "- Enforce OFFICE CODING STANDARDS strictly\n"
                "- Report ALL violations\n"
                "- Treat missing validation as CRITICAL\n\n"

                "OFFICE CODING STANDARDS (MANDATORY):\n"
                "%1\n\n"

                "EXCEPTION RULES:\n"
                "- Entry-point function 'main' is exempt from naming rules\n"
                "- Return value of 'main' does not require checking\n"
                "- printf / puts used only for user messaging may be exempt\n\n"

                "OUTPUT FORMAT (EXACT — NO DEVIATION):\n"
                "[LINE:<number>]\n"
                "[SEVERITY:Critical|Major|Minor]\n"
                "[RULE:<exact rule text>]\n"
                "[ISSUE:<what is wrong>]\n"
                "[IMPACT:<technical risk>]\n\n"

                "OUTPUT RULES:\n"
                "- Line number is mandatory\n"
                "- One issue per block\n"
                "- No extra text\n"
                "- No markdown\n\n"

                "CODE:\n"
                "%2\n"
                ).arg(qsCodingStandards, qsSelectedCode);
}

QString DevAssistant_RvwPrptBdr::buildPromptReplaceCode(
        const QString &qsSelectedCode,
        const QString &qsCodingStandards)
{
    return QString(
                "You are a senior C/C++ software engineer and refactoring expert.\n"
                "Follow the OFFICE CODING STANDARDS strictly.\n\n"

                "OFFICE CODING STANDARDS:\n"
                "%1\n\n"

                "TASK:\n"
                "- Rewrite the given code to fully comply with the office coding standards\n"
                "- Fix coding standard violations\n"
                "- Improve safety and readability without changing functionality\n"
                "- Preserve original logic and behavior exactly\n\n"

                "OUTPUT RULES (MANDATORY):\n"
                "- OUTPUT ONLY THE REPLACED CODE\n"
                "- NO explanations\n"
                "- NO comments outside the code\n"
                "- NO markdown formatting\n"
                "- NO headings or extra text\n\n"

                "CODE TO REPLACE:\n"
                "%2\n"
                ).arg(qsCodingStandards, qsSelectedCode);
}

QString DevAssistant_RvwPrptBdr::buildPromptExplainCode(
        const QString &qsSelectedCode)
{
    return QString(
                "You are a senior C/C++ software engineer and technical mentor.\n\n"

                "TASK:\n"
                "- Explain the logic of the given code in simple and clear terms\n"
                "- Describe the overall purpose of the code\n"
                "- Explain key functions, control flow, and important conditions\n"
                "- Mention assumptions and edge cases if relevant\n"
                "- DO NOT rewrite or modify the code\n"
                "- DO NOT suggest improvements\n\n"

                "OUTPUT RULES:\n"
                "- Explanation only\n"
                "- No code generation\n"
                "- No markdown formatting\n\n"

                "CODE TO EXPLAIN:\n"
                "%1\n\n"
                ).arg(qsSelectedCode);
}

QString DevAssistant_RvwPrptBdr::buildPromptOptimizeCode(
        const QString &qsSelectedCode)
{
    return QString(
                "You are a senior C/C++ optimization expert.\n\n"

                "ASSUMPTION:\n"
                "- The given code already follows Office Coding Standards\n\n"

                "TASK:\n"
                "- Optimize the given code for performance and safety\n"
                "- Remove redundant operations\n"
                "- Improve efficiency and clarity\n\n"

                "STRICT RULES (MANDATORY):\n"
                "- DO NOT change behavior or logic\n"
                "- DO NOT change function signatures or interfaces\n"
                "- DO NOT change variable or function names\n"
                "- DO NOT change formatting style unless required for correctness\n"
                "- DO NOT add or remove comments\n"
                "- DO NOT add new features\n\n"

                "OUTPUT RULES:\n"
                "- OUTPUT ONLY THE OPTIMIZED CODE\n"
                "- NO explanations\n"
                "- NO markdown\n"
                "- NO extra text\n\n"

                "CODE TO OPTIMIZE:\n"
                "%1\n"
                ).arg(qsSelectedCode);
}
