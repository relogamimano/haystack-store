*** Settings ***
Resource    keyword.resource
Library     Process
Library     OperatingSystem
Library     ./lib/Errors.py    ${SRC_DIR}/error.h    error_codes    ${SRC_DIR}/error.c    ERR_MESSAGES    prefix=Imgfs exited with error:
Library     ./lib/Imgfs.py    ${EXE}    ${SERVER_EXE}    ${SRC_DIR}/error.h    error_codes    ${SRC_DIR}/error.c    ERR_MESSAGES    ${DATA_DIR}    prefix=Imgfs exited with error:
Library     ./lib/Utils.py    ${EXE}

Test Setup    Imgfs Start Server    test02    8000
Test Teardown    Imgfs Stop Server

*** Test Cases ***
List empty
    [Setup]    Imgfs Start Server    empty    8000
    Imgfs Curl    http://localhost:8000/imgfs/list    expected_file=${DATA_DIR}/http_empty_list.bin
    [Teardown]    Imgfs Stop Server

List
    Imgfs Curl    http://localhost:8000/imgfs/list    expected_file=${DATA_DIR}/http_test02_list.bin

Read not found
    Imgfs Curl    http://localhost:8000/imgfs/read?img_id\=pic3&res\=orig    expected_err=ERR_IMAGE_NOT_FOUND

Read missing parameter
    Imgfs Curl    http://localhost:8000/imgfs/read    expected_err=ERR_NOT_ENOUGH_ARGUMENTS
    Imgfs Curl    http://localhost:8000/imgfs/read?img_id\=pic1    expected_err=ERR_NOT_ENOUGH_ARGUMENTS
    Imgfs Curl    http://localhost:8000/imgfs/read?res\=orig    expected_err=ERR_NOT_ENOUGH_ARGUMENTS
    Imgfs Curl    http://localhost:8000/imgfs/read?img_id\=pic1&resolution\=orig    expected_err=ERR_NOT_ENOUGH_ARGUMENTS

Read invalid resolution
    Imgfs Curl    http://localhost:8000/imgfs/read?img_id\=pic1&res\=not_a_res    expected_err=ERR_RESOLUTIONS

Read successful
    Imgfs Curl    http://localhost:8000/imgfs/read?img_id\=pic1&res\=orig    expected_file=${DATA_DIR}/http_read.bin
    Imgfs Curl    http://localhost:8000/imgfs/read?img_id\=pic2&res\=thumb    expected_file=${DATA_DIR}/http_read_resize-VIPS.bin

Delete not found
    Imgfs Curl    http://localhost:8000/imgfs/delete?img_id\=pic3    expected_err=ERR_IMAGE_NOT_FOUND

Delete missing parameter
    Imgfs Curl    http://localhost:8000/imgfs/delete    expected_err=ERR_NOT_ENOUGH_ARGUMENTS

Delete successful
    Imgfs Curl    http://localhost:8000/imgfs/delete?img_id\=pic1    expected_file=${DATA_DIR}/http_found.bin

Delete twice fails
    Imgfs Curl    http://localhost:8000/imgfs/delete?img_id\=pic1    expected_file=${DATA_DIR}/http_found.bin
    Imgfs Curl    http://localhost:8000/imgfs/delete?img_id\=pic1    expected_err=ERR_IMAGE_NOT_FOUND
    
Insert Get
    Imgfs Curl    http://localhost:8000/imgfs/insert    expected_err=ERR_INVALID_COMMAND

Insert missing parameter
    Imgfs Curl    http://localhost:8000/imgfs/insert    -X    POST    --data-binary    @${DATA_DIR}/brouillard.jpg    expected_err=ERR_NOT_ENOUGH_ARGUMENTS

Insert duplicate id
    Imgfs Curl    http://localhost:8000/imgfs/insert?name\=pic1    -X    POST    --data-binary    @${DATA_DIR}/brouillard.jpg    expected_err=ERR_DUPLICATE_ID

Insert successful
    Imgfs Curl    http://localhost:8000/imgfs/insert?name\=pic3    -X    POST    --data-binary    @${DATA_DIR}/brouillard.jpg    expected_file=${DATA_DIR}/http_found.bin

Insert then read
    Imgfs Curl    http://localhost:8000/imgfs/insert?name\=pic3    -X    POST    --data-binary    @${DATA_DIR}/brouillard.jpg    expected_file=${DATA_DIR}/http_found.bin
    Imgfs Curl    http://localhost:8000/imgfs/read?img_id\=pic3&res\=orig    expected_file=${DATA_DIR}/http_insert_read.bin
