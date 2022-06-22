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
This call closes the device and signals the process still using the device that the device isn't active now by setting the counter of bytes unread to -1.

# Read
This call has a blocking mode and a non-blocking mode. In both modes, it tries to read the number of bytes equal to the smallest value of:
- number of bytes to read passed as an argument
- number of bytes until the end of the buffer
- number of bytes unread stored in a global variable
After that, it updates the counter of bytes unread and checks for mode. In non-blocking mode, the call returns the number of bytes unread immediately. In blocking mode, it checks whether the number of bytes passed as an argument had been read or the writer stopped working(indicated by the bytes_unread counter being set to -1). If this is the case, the call returns the number of bytes read. Otherwise, it attempts to read as much bytes as it can like a new non-blocking call would until the total number of bytes read is equal to the size argument of the call or the writer stops working.

# Write
This call also has a blocking mode and a non-blocking mode. In non-blocking mode and on each iteration of the blocking mode(which stops if the reader stops working or the number of bytes written is equal to size) it attempts to write the number of bytes equal to the minimum of:
- number of bytes to write passed as an argument
- number of bytes until the end of the buffer
Also, in blocking mode, the call doesn't set the buffer offset to 0 until all data in the buffer is read. The write call also updates the bytes_unread counter.

# IOCTL
This call has several commands implemented.
- \_IO(42, 0) changes the read and write mode from blocking to non-blocking and vice versa.
- \_IOR(42, 1, int64_t*) puts the real kernel time of the last read operation to the buffer provided as an argument.
- \_IOR(42, 2, int64_t*) puts the real kernel time of the last write operation to the buffer provided as an argument.
- \_IOR(42, 3, int64_t*) puts the caller PID of the last read operation to the buffer provided as an argument.
- \_IOR(42, 4, int64_t*) puts the caller PID of the last write operation to the buffer provided as an argument.
- \_IOR(42, 5, int64_t*) puts the UID of the owner of the calling process of the last read operation to the buffer provided as an argument.
- \_IOR(42, 6, int64_t*) puts the UID of the owner of the calling process of the last write operation to the buffer provided as an argument.
