*** Settings ***
Resource    keyword.resource
Library     Process
Library     OperatingSystem
Library     ./lib/Errors.py    ${SRC_DIR}/error.h    error_codes    ${SRC_DIR}/error.c    ERR_MESSAGES    prefix=Imgfs exited with error:
Library     ./lib/Imgfs.py    ${EXE}    ${SERVER_EXE}    ${SRC_DIR}/error.h    error_codes    ${SRC_DIR}/error.c    ERR_MESSAGES    ${DATA_DIR}    prefix=Imgfs exited with error:
Library     ./lib/Utils.py    ${EXE}

*** Test Cases ***
Help
    ${res}    Imgfs Run    help
    Should Match Help    
    ...    ${res.stdout}
    ...    help: displays this help.
    ...    list <imgFS_filename>: list imgFS content.
    ...    create <imgFS_filename> [options]: create a new imgFS.\noptions are:\n-max_files <MAX_FILES>: maximum number of files.\ndefault value is 128\nmaximum value is 4294967295\n-thumb_res <X_RES> <Y_RES>: resolution for thumbnail images.\ndefault value is 64x64\nmaximum value is 128x128\n-small_res <X_RES> <Y_RES>: resolution for small images.\ndefault value is 256x256\nmaximum value is 512x512
    ...    delete <imgFS_filename> <imgID>: delete image imgID from imgFS.
    ...    insert <imgFS_filename> <imgID> <filename>: insert a new image in the imgFS.
    ...    read   <imgFS_filename> <imgID> [original|orig|thumbnail|thumb|small]:\nread an image from the imgFS and save it to a file.\ndefault resolution is "original".

Insert invalid file
    Imgfs Run    insert    ${DATA_DIR}/none.imfgs    pic1    ${DATA_DIR}/none.jpg    expected_ret=ERR_IO

Insert duplicate id
    ${dump}    Copy Dump File    test02
    Imgfs Run    insert    ${dump}    pic1    ${DATA_DIR}/papillon.jpg    expected_ret=ERR_DUPLICATE_ID

Insert full
    ${dump}    Copy Dump File    full
    Imgfs Run    insert    ${dump}    pic4    ${DATA_DIR}/papillon.jpg    expected_ret=ERR_IMGFS_FULL

Insert duplicate image
    ${dump}    Copy Dump File    test02
    Imgfs Run    insert    ${dump}    pic3    ${DATA_DIR}/papillon.jpg    expected_ret=ERR_NONE
    Imgfs Run    list    ${dump}    expected_file=${DATA_DIR}/insert_duplicate.txt

    ${dump_size}    Get File Size    ${dump}
    Should Be Equal As Integers    ${dump_size}    192659

Insert new image
    ${dump}    Copy Dump File    test02
    Imgfs Run    insert    ${dump}    pic3    ${DATA_DIR}/foret.jpg    expected_ret=ERR_NONE
    Imgfs Run    list    ${dump}    expected_file=${DATA_DIR}/insert_new.txt

    ${dump_size}    Get File Size    ${dump}
    Should Be Equal As Integers    ${dump_size}    562570

Read invalid file
    Imgfs Run    read    ${DATA_DIR}/none.imfgs    pic1    orig    expected_ret=ERR_IO

Read no such image
    ${dump}    Copy Dump File    empty
    Imgfs Run    read    ${dump}    pic1    orig    expected_ret=ERR_IMAGE_NOT_FOUND

Read valid
    ${dump}    Copy Dump File    test02
    Imgfs Run    read    ${dump}    pic1    orig    expected_ret=ERR_NONE
    Jpeg Files Should Be Equal    ${DATA_DIR}/papillon.jpg    pic1_orig.jpg

Read valid default args
    ${dump}    Copy Dump File    test02
    Imgfs Run    read    ${dump}    pic1    expected_ret=ERR_NONE
    Jpeg Files Should Be Equal    ${DATA_DIR}/papillon.jpg    pic1_orig.jpg

# ======================== /!\ WARNING /!\ WARNING /!\ WARNING /!\ WARNING /!\ WARNING /!\ ========================
# The below tests may fail with some VIPS versions.
# The above ones should always work. If the latters passe and formers fail,
# use `make feedback` or run it on the VMs to verify whether this is due to your environment.
# ======================== /!\ WARNING /!\ WARNING /!\ WARNING /!\ WARNING /!\ WARNING /!\ ========================

Read resize Fallible
    ${dump}    Copy Dump File    test02
    Imgfs Run    read    ${dump}    pic1    small    expected_ret=ERR_NONE
    Resize Jpeg    ${DATA_DIR}/papillon.jpg    ${DATA_DIR}/papillon_small.jpg    256    256
    Jpeg Files Should Be Equal    ${DATA_DIR}/papillon_small.jpg    pic1_small.jpg


Read valid Fallible
    ${dump}    Copy Dump File    test02
    Imgfs Run    read    ${dump}    pic1    orig    expected_ret=ERR_NONE
    Binary Files Should Be Equal    ${DATA_DIR}/papillon.jpg    pic1_orig.jpg

Read valid default args Fallible
    ${dump}    Copy Dump File    test02
    Imgfs Run    read    ${dump}    pic1    expected_ret=ERR_NONE
    Binary Files Should Be Equal    ${DATA_DIR}/papillon.jpg    pic1_orig.jpg

Read resize
    ${dump}    Copy Dump File    test02
    Imgfs Run    read    ${dump}    pic1    small    expected_ret=ERR_NONE
    Binary Files Should Be Equal    ${DATA_DIR}/papillon256_256-VIPS.jpg    pic1_small.jpg

    Imgfs Run    list    ${dump}    expected_ret=ERR_NONE    expected_file=${DATA_DIR}/read_resize-VIPS.txt
