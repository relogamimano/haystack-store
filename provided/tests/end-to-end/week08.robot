*** Settings ***
Resource    keyword.resource
Library     Process
Library     OperatingSystem
Library     ./lib/Errors.py    ${SRC_DIR}/error.h    error_codes    ${SRC_DIR}/error.c    ERR_MESSAGES    prefix=Imgfs exited with error:
Library     ./lib/Imgfs.py    ${EXE}    ${SERVER_EXE}    ${SRC_DIR}/error.h    error_codes    ${SRC_DIR}/error.c    ERR_MESSAGES    ${DATA_DIR}    prefix=Imgfs exited with error:
Library     ./lib/Utils.py    ${EXE}


*** Variables ***
@{HELP_ENTRIES}
...    help: displays this help.
...    list <imgFS_filename>: list imgFS content.
...    create <imgFS_filename> [options]: create a new imgFS.\noptions are:\n-max_files <MAX_FILES>: maximum number of files.\ndefault value is 128\nmaximum value is 4294967295\n-thumb_res <X_RES> <Y_RES>: resolution for thumbnail images.\ndefault value is 64x64\nmaximum value is 128x128\n-small_res <X_RES> <Y_RES>: resolution for small images.\ndefault value is 256x256\nmaximum value is 512x512
...    delete <imgFS_filename> <imgID>: delete image imgID from imgFS.


*** Test Cases ***
Help
    ${res}    Imgfs Run    help
    Should Match Help    ${res.stdout}    @{HELP_ENTRIES}

Shows help on invalid command
    ${res}    Imgfs Run    asdf
    Should Match Help    ${res.stdout}    @{HELP_ENTRIES}    error=Invalid command

Shows help on error
    ${res}    Imgfs Run    list    /does/not/exists.imgfs
    Should Match Help    ${res.stdout}    @{HELP_ENTRIES}    error=I/O Error

Create
    [Documentation]    valid create command
    [Template]    Create template
    ERR_NONE    list_default
    ERR_NONE    list_max_files    -max_files    10
    ERR_NONE    list_max_files    -max_files    1021    -max_files    10
    ERR_NONE    list_thumb_res    -thumb_res    96    96
    ERR_NONE    list_small_res    -small_res    216    216
    ERR_NONE    list_all_params    -max_files    10    -thumb_res    96    96    -small_res    216    216
    ERR_NONE    list_all_params    -thumb_res    128    128    -max_files    10    -thumb_res    96    96    -small_res    216    216

Create invalid argument
    [Documentation]    create command with invalid arguments
    [Template]    Create template
    ERR_INVALID_ARGUMENT    None    -max_fles    10
    ERR_INVALID_ARGUMENT    None    -max_fles    10b
    ERR_INVALID_ARGUMENT    None    -thub_res    96    96
    ERR_INVALID_ARGUMENT    None    -thub_res    96    9b6
    ERR_INVALID_ARGUMENT    None    -smll_res    216    216
    ERR_INVALID_ARGUMENT    None    -smll_res    a16    216

Delete no such image
    [Documentation]    delete on empty database
    ${dump}    Copy Dump File    empty
    Imgfs Run    delete    ${dump}    picture    expected_ret=ERR_IMAGE_NOT_FOUND

Delete one
    [Documentation]    delete one existing image
    ${dump}    Copy Dump File    test02
    Imgfs Run    delete    ${dump}    pic1    expected_ret=ERR_NONE
    Imgfs Run    list    ${dump}    expected_ret=ERR_NONE    expected_file=${DATA_DIR}/delete/delete_one.txt

Delete all
    [Documentation]    delete all existing images
    ${dump}    Copy Dump File    test10
    Imgfs Run    delete    ${dump}    pic1    expected_ret=ERR_NONE
    Imgfs Run    delete    ${dump}    pic3    expected_ret=ERR_NONE
    Imgfs Run    list    ${dump}    expected_ret=ERR_NONE    expected_file=${DATA_DIR}/delete/delete_all.txt

Delete twice
    [Documentation]    delete same image twice
    ${dump}    Copy Dump File    test02
    Imgfs Run    delete    ${dump}    pic1    expected_ret=ERR_NONE
    Imgfs Run    delete    ${dump}    pic1    expected_ret=ERR_IMAGE_NOT_FOUND


*** Keywords ***
Create template
    [Arguments]    ${expected_error}    ${expected_file}    @{options}
    ${failed}    Set Variable    false
    ${dump}    Imgfs Dump
    TRY
        Imgfs Run
        ...    create
        ...    ${DATA_DIR}/${dump}
        ...    @{options}
        ...    expected_ret=${expected_error}

        IF    "${expected_error}" == "ERR_NONE"
            Imgfs Run
            ...    list
            ...    ${DATA_DIR}/${dump}
            ...    expected_ret=ERR_NONE
            ...    expected_file=${DATA_DIR}/create/${expected_file}.txt
        END
    EXCEPT    AS    ${message}
        Register Failure    ${message}
        ${failed}    Set Variable    true
    FINALLY
        Imgfs End Test
    END

    IF    $failed == "true"    Fail    Failed for ${dump}.imgfs (see above)
