# Linux USB Block Device Driver

A Linux kernel module that provides block-level read and write access to a USB storage device through a character-device IOCTL interface. The module supports sequential and offset-based operations while handling communication between user space and kernel space.

## Overview

This project was developed as part of my Operating Systems coursework at Arizona State University. The goal was to implement a kernel-level interface capable of performing read and write operations directly on a USB block device.

The project combines Linux character-device interfaces, IOCTL system calls, user/kernel memory transfers, and the Linux block I/O (BIO) subsystem.

## Features

- Registers a Linux character device for communication with user-space applications
- Implements IOCTL commands for:
  - Sequential block reads
  - Sequential block writes
  - Offset-based reads
  - Offset-based writes
- Transfers data between user space and kernel space using `copy_from_user()` and `copy_to_user()`
- Dynamically allocates kernel buffers for I/O requests
- Opens and accesses USB storage as a Linux block device
- Creates and submits BIO requests to perform block-level I/O
- Processes individual read/write operations in chunks of up to 512 bytes
- Supports configurable USB block-device paths

## Architecture

The project separates the user-facing character-device interface from the underlying block-device operations.

```text
User-Space Application
        |
        | IOCTL Request
        v
Character Device Interface
        |
        | BREAD / BWRITE
        | BREADOFFSET / BWRITEOFFSET
        v
Kernel IOCTL Handler
        |
        | User <-> Kernel Memory Transfer
        v
USB Read/Write Layer
        |
        | Linux BIO Requests
        v
USB Block Device
```

## Technical Implementation

### Character Device & IOCTL Interface

The [`kmod-ioctl.c`](./kmod-ioctl.c) component registers the character device and exposes an IOCTL interface to user-space applications.

The IOCTL handler supports four operations:

- `BREAD` — sequential block read
- `BWRITE` — sequential block write
- `BREADOFFSET` — block read from a specified offset
- `BWRITEOFFSET` — block write to a specified offset

For each request, the module allocates a kernel buffer and uses `copy_from_user()` and `copy_to_user()` to transfer data across the user/kernel boundary.

### USB Block-Level I/O

The [`kmod-main.c`](./kmod-main.c) component handles communication with the underlying USB block device.

The module opens a configurable block-device path and processes read and write requests using the Linux BIO subsystem. I/O requests are divided into operations of up to 512 bytes, mapped into BIO structures, and submitted to the block device using `submit_bio_wait()`.

This provides the lower-level storage interface used by the character-device IOCTL layer.

### Request Flow

Together, the components provide the following path from a user-space request to the block device:

```text
User-Space Application
        |
        | IOCTL
        v
kmod-ioctl.c
        |
        | BREAD / BWRITE
        | BREADOFFSET / BWRITEOFFSET
        v
User <-> Kernel Memory Transfer
        |
        v
kmod-main.c
        |
        | rw_usb()
        v
Linux BIO Subsystem
        |
        v
USB Block Device
```

### Build System

The included [`Makefile`](./Makefile) builds the kernel module and its associated components using the Linux kernel build system.

## Technologies & Concepts

- C
- Linux
- Linux Kernel Programming
- Kernel Modules
- Character Devices
- IOCTL
- Block Devices
- Linux BIO API
- User/Kernel Memory Management
- Low-Level I/O

## What I Learned

This project provided hands-on experience working with Linux kernel interfaces and strengthened my understanding of:

- The separation between user space and kernel space
- Character devices and kernel device registration
- IOCTL-based communication
- User/kernel memory transfers
- Block-device architecture
- Linux BIO request handling
- Kernel memory allocation and cleanup
- Low-level storage I/O
- Modular kernel-level software design

## Academic Project Notice

This project was completed as part of Operating Systems coursework at Arizona State University. The implementation included in this repository represents my work on the project and is presented as part of my technical portfolio.
