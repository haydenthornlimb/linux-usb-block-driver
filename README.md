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
- Transfers data safely between user space and kernel space using `copy_from_user()` and `copy_to_user()`
- Dynamically allocates kernel buffers for I/O requests
- Opens and accesses USB storage as a Linux block device
- Creates and submits BIO requests to perform block-level I/O
- Processes read/write requests in 512-byte sectors
- Supports configurable USB block-device paths

## Architecture

The project separates the user-facing device interface from the underlying block-device operations.

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
