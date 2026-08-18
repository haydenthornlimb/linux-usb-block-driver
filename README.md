# Linux USB Block Device Driver

A Linux kernel module providing block-level read and write access to a USB storage device through a character-device IOCTL interface.

> **Source Code Notice:** The source code for this project is intentionally not included because it was completed as part of university coursework. This repository documents the project's architecture, functionality, and technical concepts without distributing an assignment solution that could be reused by current students.

## Overview

This project was developed as part of my Operating Systems coursework at Arizona State University.

The goal of the project was to develop a kernel-level interface capable of performing read and write operations on a USB block device. The implementation connected a user-facing character-device interface with Linux's underlying block I/O system.

The project provided hands-on experience with Linux kernel programming, character devices, IOCTL system calls, user/kernel memory transfers, block devices, and low-level storage I/O.

## Core Functionality

The kernel module supported:

- A Linux character device for communication with user-space applications
- IOCTL-based read and write commands
- Sequential block reads
- Sequential block writes
- Offset-based reads
- Offset-based writes
- Safe transfer of data between user space and kernel space
- Dynamic kernel-buffer allocation for I/O operations
- Access to USB storage through the Linux block-device system
- BIO-based block I/O requests
- Configurable USB block-device paths

## Architecture

The project separated the user-facing character-device interface from the underlying USB block-device operations.

```text
User-Space Application
        |
        | IOCTL Request
        v
Character Device Interface
        |
        | Read / Write Commands
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

This design provided a structured path for user-space applications to request storage operations while the kernel module handled communication with the underlying block device.

## Character Device & IOCTL Interface

The module registered a Linux character device that exposed an IOCTL interface to user-space applications.

The interface supported four primary operations:

- `BREAD` — sequential block read
- `BWRITE` — sequential block write
- `BREADOFFSET` — block read from a specified offset
- `BWRITEOFFSET` — block write from a specified offset

The IOCTL handler interpreted incoming requests, allocated the necessary kernel buffers, and coordinated the requested storage operation.

## User/Kernel Memory Management

Because user-space memory cannot be directly accessed like ordinary kernel memory, the project used Linux kernel memory-access mechanisms to safely move data across the user/kernel boundary.

The implementation used:

- `copy_from_user()` when transferring write data from a user-space application into kernel memory
- `copy_to_user()` when returning read data from kernel memory to a user-space application

Kernel buffers were dynamically allocated for I/O operations and released after requests were completed.

## Block-Level I/O

USB storage operations were performed through the Linux block I/O subsystem.

The implementation created BIO requests representing operations on the underlying storage device. These requests were submitted through the kernel's block-device infrastructure to perform the requested reads and writes.

The module supported both sequential access and explicit offsets, allowing callers to specify where operations should occur on the USB block device.

## Request Flow

A typical request passed through several layers of the system:

```text
User-Space Application
        |
        v
IOCTL Request
        |
        v
Character Device
        |
        v
IOCTL Handler
        |
        v
Kernel Buffer
        |
        v
Block I/O Request
        |
        v
Linux BIO Subsystem
        |
        v
USB Storage Device
```

This project therefore connected several operating-system concepts into a single working interface, from user-space communication down to block-level storage operations.

## Technologies & Concepts

- C
- Linux
- Linux Kernel Programming
- Linux Kernel Modules
- Character Devices
- IOCTL
- Block Devices
- Linux BIO API
- User/Kernel Memory Management
- Dynamic Kernel Memory Allocation
- Device I/O
- Low-Level Storage I/O

## What I Learned

This project strengthened my understanding of:

- The separation between user space and kernel space
- Developing Linux kernel modules
- Character-device registration and communication
- IOCTL-based user/kernel interfaces
- Safely transferring memory across the user/kernel boundary
- Working with Linux block devices
- Creating and submitting BIO requests
- Kernel memory allocation and cleanup
- Sequential and offset-based storage operations
- Low-level device I/O
- Connecting higher-level interfaces to lower-level kernel subsystems

The project provided practical experience working with operating-system mechanisms that are normally abstracted away from application-level software development.

## Academic Project Notice

This project was completed as part of Operating Systems coursework at Arizona State University.

The source code is intentionally excluded to protect the academic integrity of the assignment and prevent the implementation from being reused as a solution by current or future students. This repository is provided solely as a portfolio demonstration of the project's architecture, functionality, and technical concepts.
