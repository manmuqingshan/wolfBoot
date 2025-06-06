# Firmware update

This section documents the complete firmware update procedure, enabling secure boot
for an existing embedded application.


## Updating Microcontroller FLASH

The steps to complete a firmware update with wolfBoot are:
   - Compile the firmware with the correct entry point
   - Sign the firmware
   - Transfer the image using a secure connection, and store it to the secondary firmware slot
   - Trigger the image swap
   - Reboot to let the bootloader begin the image swap

At any given time, an application or OS running on a wolfBoot system can receive an updated version of itself,
and store the updated image in the second partition in the FLASH memory.

![Update and Rollback](png/wolfboot_update_rollback.png)

Applications or OS threads can be linked to the [libwolfboot library](API.md), which exports the API to trigger
the update at the next reboot, and some helper functions to access the flash partition for 
erase/write through the target specific [HAL](HAL.md).

## Update procedure description

Using the [API](API.md) provided to the application, wolfBoot offers the possibility to initiate, confirm or 
rollback an update.

After storing the new firmware image in the UPDATE partition, the application should initiate the update by calling
`wolfBoot_update_trigger()`. By doing so, the UPDATE partition is marked for update. Upon the next reboot, wolfBoot will:
  - Validate the new firmware image stored in the UPDATE partition
  - Verify the signature attached against a known public key stored in the bootloader image
  - Swap the content of the BOOT and the UPDATE partitions
  - Mark the new firmware in the BOOT partition as in state `STATE_TESTING`
  - Boot into the newly received firmware

If the system is interrupted during the swap operation and reboots,
wolfBoot will pick up where it left off and continue the update
procedure.

### Successful boot

Upon a successful boot, the application should inform the bootloader by calling `wolfBoot_success()`, after verifying that
the system is up and running again. This operation confirms the update to a new firmware.

Failing to set the BOOT partition to `STATE_SUCCESS` before the next reboot triggers a roll-back operation.
Roll-back is initiated by the bootloader by triggering a new update, this time starting from the backup copy of the original 
(pre-update) firmware, which is now stored in the UPDATE partition due to the swap occurring earlier.

### Building a new firmware image

Firmware images are position-dependent, and can only boot from the origin of the **BOOT** partition in FLASH.
This design constraint implies that the chosen firmware is always stored in the **BOOT** partition, and wolfBoot
is responsible for pre-validating an update image and copy it to the correct address.

All the firmware images must therefore have their entry point set to the address corresponding to the beginning 
of the **BOOT** partition, plus an offset of 256 Bytes to account for the image header.

Once the firmware is compiled and linked, it must be signed using the `sign` tool. The tool produces
a signed image that can be transferred to the target using a secure connection, using the same key corresponding
to the public key currently used for verification.

The tool also adds all the required Tags to the image header, containing the signatures and the SHA256 hash of 
the firmware.

### Self-update

wolfBoot can update itself if `RAM_CODE` is set. This procedure
operates almost the same as firmware update with a few key
differences. The header of the update is marked as a bootloader
update (use `--wolfboot-update` for the sign tools).

The new signed wolfBoot image is loaded into the UPDATE partition and
triggered the same as a firmware update. Instead of performing a swap,
after the image is validated and signature verified, the bootloader is
erased and the new image is written to flash. This operation is _not_
safe from interruption. Interruption will prevent the device from
rebooting.

wolfBoot can be used to deploy new bootloader versions as well as
update keys.

### Incremental updates (aka: 'delta' updates)

wolfBoot supports incremental updates, based on a specific older version. The sign tool
can create a small "patch" that only contains the binary difference between the version
currently running on the target and the update package. This reduces the size of the image
to be transferred to the target, while keeping the same level of security through public key
verification, and integrity due to the repeated check (on the patch and the resulting image).

The format of the patch is based on the mechanism suggested by Bentley/McIlroy, which is particularly effective
to generate small binary patches. This is useful to minimize time and resources needed to transfer,
authenticate and install updates.


#### How it works

As an alternative to transferring the entire firmware image, the key tools create
a binary diff between a base version previously uploaded and the new updated image.

The resulting bundle (delta update) contains the information to derive the content
of version '2' of the firmware, starting from the base version, that is currently
running on the target (version '1' in this example), and the reverse patch to downgrade
version '2' back to version '1' if something goes wrong running the new version.

![Delta update](png/delta_updates.png)

On the device side, wolfboot will recognize and verify the authenticity of the delta update before
applying the patch to the current firmware. The new firmware is rebuilt in place,
replacing the content of the BOOT partition according to the indication in the
(authenticated) 'delta update' bundle.


#### Two-steps verification

Binary patches are created by comparing signed firmware images. wolfBoot verifies
that the patch is applied correctly by checking for the integrity and the authenticity
of the resulting image after the patch.

The delta update bundle itself, containing the patches, is prefixed with a manifest
header describing the details for the patch, and signed like a normal full update bundle.

This means that wolfBoot will apply two levels of authentication: the first one 
when the delta bundle is processed (e.g. when an update is triggered), and the second
one every time a patch is applied, or reversed, to validate the firmware image
before booting.

These steps are performed automatically by the key tools when using the `--delta`
option, as described in the example.


#### Confirming the update

From the application perspective, nothing changes from the normal, 'full' update case.
Application must still call `wolfBoot_success()` on the first boot with the updated version
to ensure that the update is confirmed.

Failing to confirm the success of the update will cause wolfBoot to revert the patch
applied during the update. The 'delta update' bundle also contains a reverse patch,
which can revert the update and restore the base version of the firmware.

The diagram below shows the authentication steps and the diff/patch process in both
directions (update and roll-back for missed confirmation).

![Delta update: details](png/delta_updates_2.png)


#### Incremental update: example

Requirement: wolfBoot is compiled with `DELTA_UPDATES=1`

Version "1" is signed as usual, as a standalone image:

`tools/keytools/sign --ecc256 --sha256 test-app/image.bin wolfboot_signing_private_key.der 1`

When updating from version 1 to version 2, you can invoke the sign tool as:

`tools/keytools/sign --delta test-app/image_v1_signed.bin --ecc256 --sha256 test-app/image.bin wolfboot_signing_private_key.der 2`

Besides the usual output file `image_v2_signed.bin`, the sign tool creates an additional `image_v2_signed_diff.bin`
which should be noticeably smaller in size as long as the two binary files contain overlapping areas.

This is the delta update bundle, a signed package containing the patches for updating version 1 to version 2, and to roll back to version 1 if needed, after the first patch has been applied.

The delta bundle `image_v2_signed_diff.bin` can be now transferred to the update partition on the target like a full update image.

At next reboot, wolfBoot recognizes the incremental update, checks the integrity, the authenticity and the versions
of the patch. If all checks succeed, the new version is installed by applying the patch on the current firmware image.

If the update is not confirmed, at the next reboot wolfBoot will restore the original base `image_v1_signed.bin`, using
the reverse patch contained in the delta update bundle.

## ELF loading

wolfBoot supports loading ELF (Executable and Linkable Format) images via both the RAM [update_ram.c](../src/update_ram.c) and [flash update](../src/update_flash.c) mechanisms.

### RAM ELF loading

The wolfBoot RAM loader supports loading ELF images from flash into RAM before booting. When using the RAM loader [update_ram.c](../src/update_ram.c) with `WOLFBOOT_ELF` defined, wolfBoot will verify the ELF file signature and hash as stored in the boot or update partition, and then load the ELF file into RAM based on the LMA (Load Memory Address) of each section before jumping to the entry point.

### Flash ELF loading

The wolfBoot flash loader also supports loading ELF files containing scattered LMA (Load Memory Address) segments to their respective locations in flash. This feature allows firmware images to be distributed as ELF files with sections only containing loadable regions, rather than requiring a contiguous/flat binary image for the entire memory space or image size. The flash elf loading procedure only supports loading ELF file program segments into flash memory with the same access restrictions as the BOOT partition (e.g. will use the same hal_flash/ext_flash functions) and does not support loading sections into RAM.

#### How it works

When wolfBoot is compiled with `WOLFBOOT_ELF_FLASH_SCATTER` defined, it includes support for:

1. **Section Loading**: During boot or update, wolfBoot:
   - Parses the ELF headers to identify section locations
   - Loads each section into its designated memory address
   - Sets up the proper entry point for execution
   - Verifies the scattered hash after loading

2. **Dual-Layer Verification**: wolfBoot performs two distinct integrity checks:
   - Initial verification of the ELF file signature and integrity check (hash) as stored in the boot or update partition
   - A second "scattered hash" verification that re-computes the same image hash as in the manifest header, but computed over the actual loaded segments in their final memory locations (Load Memory Address - LMA) rather than their data in the ELF file

3. **Update Support**: The scattered ELF support is transparently integrated with wolfBoot's flash update mechanism:
   - ELF images can be used as update packages
   - The bootloader automatically handles loading and verifying scattered sections during the update process
   - If the scattered hash verification fails after an update, the system can fall back to the previous version

#### Using Scattered ELF Images

To use scattered ELF images:

1. Ensure wolfBoot is compiled with `WOLFBOOT_ELF` and `WOLFBOOT_ELF_FLASH_SCATTER`
2. Build your firmware as an ELF file with the desired memory layout
3. Preprocess the ELF file using [squashelf](../tools/squashelf/README.md) to strip the ELF file to just loadable sections and ensure it is compatible with wolfBoot (if building using wolfBoot's Makefile build system, this step is performed automatically)
4. Sign the ELF image using the wolfBoot signing tools
5. Transfer the signed ELF image to the target like any other update

The on boot or during update, the bootloader will automatically perform both layers of verification:

1. Verifying the signature and hash over the image in the BOOT or UPDATE partition
2. Computing and verifying the scattered hash after loading sections to their final locations

Note: When using scattered ELF images, ensure that:

- The ELF file adheres to the ELF file specification and was generated by a toolchain supporting the target architecture
- All section addresses are within valid executable memory regions and **do not overlap with the wolfBoot image, nor the BOOT, UPDATE and SWAP partitions**.

## Certificate Verification

wolfBoot supports authenticating images using certificate chains instead of raw public keys. in this mode of operation, a certificate chain is included in the image manifest header, and the image is signed with the private key corresponding to the leaf certificate identity (signer cert). On boot, wolfBoot verifies the trust of the certificate chain (and therefore the signer cert) against a trusted root CA stored in the wolfHSM server, and if the chain is trusted, verifies the authenticity of the firmware image using the public key from the image signer certificate.

To use this feature:

1. Enable the feature in your wolfBoot configuration by defining `WOLFBOOT_CERT_CHAIN_VERIFY`
2. When signing firmware, include the certificate chain using the `--cert-chain` option:

```sh
./tools/keytools/sign --rsa2048 --sha256 --cert-chain cert_chain.der test-app/image.bin private_key.der 1
```

When verifying firmware, wolfBoot will:

1. Extract the certificate chain from the firmware header
2. Verify the chain using the pre-provisioned root certificate
3. Use the public key from the leaf certificate to verify the firmware signature

This feature is particularly useful in scenarios where you want to rotate signing keys without updating the bootloader, as you can simply resign the image with a new key, create a new certificate chain, then update the certificate chain in the firmware header.

Note: Currently, support for certificate verification is limited to use in conjuction with wolfHSM. Fore more information see [wolfHSM.md](wolfHSM.md).
