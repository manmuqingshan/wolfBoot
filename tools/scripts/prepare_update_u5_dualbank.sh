#!/bin/bash

. .config
SIGN_TOOL="./tools/keytools/sign"

# SIZE is WOLFBOOT_PARTITION_SIZE - 5
SIZE=229371
VERSION=8
APP=test-app/image_v"$VERSION"_signed.bin
$SIGN_TOOL --ecc256 test-app/image.bin wolfboot_signing_private_key.der $VERSION
dd if=/dev/zero bs=$SIZE count=1 2>/dev/null | tr "\000" "\377" > update.bin
dd if=$APP of=update.bin bs=1 conv=notrunc
printf "pBOOT"  >> update.bin
