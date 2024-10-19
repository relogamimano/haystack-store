*** Settings ***
Library     Process
Library     OperatingSystem
Library     String
Library     Collections
Library     ./lib/Errors.py    ${SRC_DIR}/error.h    error_codes    ${SRC_DIR}/error.c    ERR_MESSAGES    prefix=u6fs exited with error:
Library     ./lib/Imgfs.py    ${EXE}    ${SERVER_EXE}    ${SRC_DIR}/error.h    error_codes    ${SRC_DIR}/error.c    ERR_MESSAGES    ${DATA_DIR}    prefix=u6fs exited with error:
Library     ./lib/Utils.py    ${EXE}

*** Variables ***
${SRC_DIR}              ../../../done
${EXE}                  ${SRC_DIR}/imgfscmd
${SERVER_EXE}           ${SRC_DIR}/imgfs_server
${MOUNTPOINT}           /tmp/cs212
${DATA_DIR}             ../data
@{DATA_FILE_NAMES}
...                     empty
...                     test02
...                     test03
...                     test04
...                     test05
...                     test06
...                     test07
...                     test08
...                     test09
...                     test10
...                     test11
...                     test12
...                     test13


*** Keywords ***
Should Match Help
    [Arguments]    ${output}    @{commands}    ${error}=None
    IF    "${error}" == "None"
        Should Match Regexp    ${output}    ^\\s*imgfscmd\\s+\\[COMMAND\\]\\s+\\[ARGUMENTS\\]
    ELSE
        ${regexp}    Regexp Escape    ${error}
        Should Match Regexp    ${output}    ^ERROR:\\s+${regexp}\\s*\n\\s*imgfscmd\\s+\\[COMMAND\\]\\s+\\[ARGUMENTS\\]
    END

    FOR    ${cmd}    IN    @{commands}
        ${regexp}    Regexp Escape    ${cmd}
        ${regexp}    Replace String Using Regexp    ${regexp}    ( |\t)    \s+
        ${regexp}    Replace String Using Regexp    ${regexp}    (\\n)    n\\\\s*
        Log    ${regexp}
        Should Match Regexp    ${output}    ${regexp}    flags=MULTILINE
    END
