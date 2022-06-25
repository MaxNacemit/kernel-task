#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#define MY_MAJOR 42
#define BLOCKING 0
#define NONBLOCKING 1

MODULE_LICENSE("GPLv3");

static int bufsize = 1024;
module_param(bufsize, int, 0660);

static char *buffer_address = NULL;
int mode = BLOCKING;
struct mutex counter_mutex;
int bytes_unread = 0;

struct operation_info {
    uint64_t timestamp;
    kuid_t owner;
    long pid;
};

struct operation_info lastread;
struct operation_info lastwrite;

struct my_device_data {
    struct cdev cdev;
};

struct my_device_data device;

static int my_open(struct inode *inode, struct file *file) {
    struct my_device_data *my_data =
             container_of(inode->i_cdev, struct my_device_data, cdev);
    if (file->private_data) {
        return 0;
    }
    file->private_data = my_data;
    return 0;
}

static int my_release(struct inode *inode, struct file *file) {
    mutex_lock(&counter_mutex);
    bytes_unread = -1;
    mutex_unlock(&counter_mutex);
    file->private_data = NULL;
    return 0;
}

static ssize_t my_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset) {
    ssize_t size_to_copy = bufsize - *offset - 1;
    if (size < size_to_copy) {
        size_to_copy = size;
    }
    mutex_lock(&counter_mutex);
    lastread.timestamp = ktime_get_real_ns();
    lastread.pid = current->pid;
    lastread.owner = current_uid();
    mutex_unlock(&counter_mutex);
    if (bytes_unread < size_to_copy) {
        size_to_copy = bytes_unread; 
    }
    if (size_to_copy <= 0) {
        return 0;
    }
    if (copy_to_user(user_buffer, buffer_address + *offset, size_to_copy)) {
        return -EFAULT;
    }
    mutex_lock(&counter_mutex);
    if (bytes_unread != -1) {
        bytes_unread -= size_to_copy;
    }
    mutex_unlock(&counter_mutex);
    *offset += size_to_copy;
    if (*offset == bufsize - 1) {
        *offset = 0;
    }
    if (mode == NONBLOCKING) {
        return size_to_copy;
    } else {
        ssize_t bytes_read = size_to_copy;
        while (bytes_read < size && bytes_unread != -1) {
            if (bytes_unread > 0) {
                size_to_copy = bufsize - *offset - 1;
                if (size - bytes_read < size_to_copy) {
                    size_to_copy = size - bytes_read;
                }
                if (bytes_unread < size_to_copy) {
                    size_to_copy = bytes_unread;
                }
                if (copy_to_user(user_buffer, buffer_address + *offset, size_to_copy)) {
                    return -EFAULT;
                }
                mutex_lock(&counter_mutex);
                if (bytes_unread != -1) {
                    bytes_unread -= size_to_copy;
                }
                mutex_unlock(&counter_mutex);
                *offset += size_to_copy;
                bytes_read += size_to_copy;
                if (*offset == bufsize - 1) {
                    *offset = 0;
                }
            }
        }
        return bytes_read;
    }
}

static ssize_t my_write(struct file *file, const char __user *user_buffer, size_t size, loff_t *offset) {
    ssize_t size_to_copy = bufsize - *offset - 1;
    if (size < size_to_copy) {
        size_to_copy = size;
    }
    mutex_lock(&counter_mutex);
    lastwrite.timestamp = ktime_get_real_ns();
    lastwrite.pid = current->pid;
    lastwrite.owner = current_uid();
    mutex_unlock(&counter_mutex);
    if (size_to_copy <= 0) {
        return 0;
    }
    if (copy_from_user(user_buffer, buffer_address + *offset, size_to_copy)) {
        return -EFAULT;
    }
    mutex_lock(&counter_mutex);
    if (bytes_unread != -1) {
        bytes_unread += size_to_copy;
    }
    mutex_unlock(&counter_mutex);
    *offset += size_to_copy;
    if (*offset == bufsize - 1) {
        *offset = 0;
    }
    if (mode == NONBLOCKING) {
        return size_to_copy;
    } else {
        ssize_t bytes_written = size_to_copy;
        while (bytes_written < size && bytes_unread != -1) {
            size_to_copy = bufsize - *offset - 1;
            if (size - bytes_written < size_to_copy) {
                size_to_copy = size - bytes_written;
            }
            if (copy_from_user(user_buffer, buffer_address + *offset, size_to_copy)) {
                return -EFAULT;
            }
            mutex_lock(&counter_mutex);
            if (bytes_unread != -1) {
                bytes_unread += size_to_copy;
            }
            mutex_unlock(&counter_mutex);
            *offset += size_to_copy;
            bytes_written += size_to_copy;
            if (*offset == bufsize - 1) {
                if (bytes_unread == 0) {
                    *offset = 0;
                }
            }
        }
        return bytes_written;
    }
}

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    switch(cmd) {
    case _IO(MY_MAJOR, 0): // switch between blocking and non-blocking calls
        mutex_lock(&counter_mutex);
        mode = 1 - mode;
        mutex_unlock(&counter_mutex);
        return 0;
    case _IOR(42, 1, int64_t*): // return timestamp of the last read operation to the provided 8-byte buffer
        if (copy_to_user(&(lastread.timestamp), (int64_t*) arg, 8)) {
            return -EFAULT;
        }
        return 0;
    case _IOR(42, 2, int64_t*): // return timestamp of the last write operation to the provided 8-byte buffer
        if (copy_to_user(&(lastwrite.timestamp), (int64_t*) arg, 8)) {
            return -EFAULT;
        }
        return 0;
    case _IOR(42, 3, int64_t*): // return pid of the last read operation to the provided 8-byte buffer
        if (copy_to_user(&(lastread.pid), (int64_t*) arg, 8)) {
            return -EFAULT;
        }
        return 0;
    case _IOR(42, 4, int64_t*): // return pid of the last write operation to the provided 8-byte buffer
        if (copy_to_user(&(lastwrite.pid), (int64_t*) arg, 8)) {
            return -EFAULT;
        }
        return 0;
    case _IOR(42, 5, int64_t*): // return process owner of the last read operation to the provided 8-byte buffer
        if (copy_to_user(&(lastread.owner), (int64_t*) arg, 8)) {
            return -EFAULT;
        }
        return 0;
    case _IOR(42, 6, int64_t*): // return process owner of the last write operation to the provided 8-byte buffer
        if (copy_to_user(&(lastwrite.owner), (int64_t*) arg, 8)) {
            return -EFAULT;
        }
        return 0;
    default:
        return -ENOTTY;
    }
}

const struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .read = my_read,
    .write = my_write,
    .release = my_release,
    .unlocked_ioctl = my_ioctl
};

static int my_init(void) {
    int err;
    err = register_chrdev_region(MKDEV(MY_MAJOR, 0), 1, "my_device_driver");
    if (err != 0) {
        return err;
    }
    cdev_init(&device.cdev, &my_fops);
    cdev_add(&device.cdev, MKDEV(MY_MAJOR, 0), 1);
    buffer_address = vmalloc(bufsize);
    if (!buffer_address) {
        return 1;
    }
    mutex_init(&counter_mutex);
    return 0;
}

static void my_cleanup(void) {
    vfree(buffer_address);
    cdev_del(&device.cdev);
    unregister_chrdev_region(MKDEV(MY_MAJOR, 0), 1);
}

module_init(my_init);
module_exit(my_cleanup);