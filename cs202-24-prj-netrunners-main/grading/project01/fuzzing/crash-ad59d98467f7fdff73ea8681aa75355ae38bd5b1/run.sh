#!/bin/sh
export ASAN_OPTIONS="allocator_may_return_null=1:max_allocation_size_mb=64:dedup_token_length=50:verbosity=0"
../../done/imgfscmd create ./fuzz.imgfs -max_files 999010 -thumb_res 127 128 -small_res 512 512
