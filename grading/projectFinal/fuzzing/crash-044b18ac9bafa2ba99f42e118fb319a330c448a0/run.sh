#!/bin/sh
export ASAN_OPTIONS="allocator_may_return_null=1:max_allocation_size_mb=64:dedup_token_length=50:verbosity=0"
../../done/imgfscmd read ./fuzz.imgfs pic1 small
