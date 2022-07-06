# kernel-task
A kernel module implementing a pipe-like character device.


# Installation
Copy the repository files to a directory, then run make. Then load module.ko with insmod.


# Usage
The module creates a character device file after initialization and allocates a buffer(the default size is 1 KiB; it can be changed as a kernel module parameter). The calls available are:
- open
- read
- write
- ioctl
- close

# Open
This call initializes the device.

# Close
This call closes the device and, if it closes the reader, it also unlocks the semaphore responsible for waiting for data becoming available in the reader's blocking mode and resets the bytes_unread counter.

# Read
This call has a blocking mode and a non-blocking mode. In both modes, it tries to read the number of bytes equal to the smallest value of:
- number of bytes to read passed as an argument
- number of bytes until the end of the buffer
- number of bytes unread stored in a global variable

In non-blocking mode, if this value is zero, it returns zero immediately. In non-blocking mode, it waits until data appears in the buffer and then returns the number of bytes read.

# Write
This call also has a blocking mode and a non-blocking mode. In both modes, it attempts to write a number of bytes to the buffer equal to the minimum of:
- the number of bytes left in the buffer
- the number of bytes passed as an argument

In non-blocking mode, if this is zero, it returns zero immediately. In blocking mode, the call waits until all data in the buffer is read and writes the necessary number of bytes from the beginning of the buffer.

# IOCTL
This call has several commands implemented.
- \_IO(42, 0) changes the read and write mode from blocking to non-blocking and vice versa.
- \_IOR(42, 1, int64_t*) puts the real kernel time of the last read operation to the buffer provided as an argument.
- \_IOR(42, 2, int64_t*) puts the real kernel time of the last write operation to the buffer provided as an argument.
- \_IOR(42, 3, int64_t*) puts the caller PID of the last read operation to the buffer provided as an argument.
- \_IOR(42, 4, int64_t*) puts the caller PID of the last write operation to the buffer provided as an argument.
- \_IOR(42, 5, int64_t*) puts the UID of the owner of the calling process of the last read operation to the buffer provided as an argument.
- \_IOR(42, 6, int64_t*) puts the UID of the owner of the calling process of the last write operation to the buffer provided as an argument.
