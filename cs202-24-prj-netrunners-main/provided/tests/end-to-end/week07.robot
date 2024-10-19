*** Settings ***
Resource    keyword.resource
Library     Process
Library     OperatingSystem
Library     ./lib/Errors.py    ${SRC_DIR}/error.h    error_codes    ${SRC_DIR}/error.c    ERR_MESSAGES    prefix=Imgfs exited with error:
Library     ./lib/Imgfs.py    ${EXE}    ${SERVER_EXE}    ${SRC_DIR}/error.h    error_codes    ${SRC_DIR}/error.c    ERR_MESSAGES    ${DATA_DIR}    prefix=Imgfs exited with error:
Library     ./lib/Utils.py    ${EXE}


*** Test Cases ***
List correct    [Documentation]    list with valid imgfs has expected behaviour
    [Template]    List template
    FOR    ${item}    IN    @{DATA_FILE_NAMES}
        ${item}
    END


*** Keywords ***
List template
    [Documentation]    Template for the test of Imgfs list
    [Arguments]    ${name}
    ${failed}    Set Variable    false
    TRY
        Imgfs Run
        ...    list
        ...    ${DATA_DIR}/${name}.imgfs
        ...    expected_ret=ERR_NONE
        ...    expected_file=${DATA_DIR}/list_out/${name}.txt
    EXCEPT    AS    ${message}
        Register Failure    ${message}
        ${failed}    Set Variable    true
    FINALLY
        Imgfs End Test
    END

    IF    $failed == "true"    Fail    Failed for ${name}.imgfs (see above)
