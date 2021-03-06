#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h>

 /* Declaring kernel module settings: */
MODULE_LICENSE("Dual BSD/GPL");

MODULE_AUTHOR("Room 2 Syrmia");

MODULE_DESCRIPTION("Tiny encryption module. The module takes up to 50 "
                    "characters from the user and applies an rotation " 
                    "function. The module returns to the user up to 50 "
                    "characters (encrypted). For the debug purposes the "
                    "encryption functionality can be disabled by setting"
		    "0 to ee flag");

#define BUF_LEN 50

/* Encryption switch */
static bool ee = 1; /* Enable encryption. */
module_param(ee, bool, 0);

//Broj rotacija
static int howmany = 1;
module_param(howmany, int, 0);

/* Declaration of encrypter.c functions */
void encrypter_exit(void);
int encrypter_init(void);
static int encrypter_open(struct inode *, struct file *);
static int encrypter_release(struct inode *, struct file *);
static ssize_t encrypter_read(struct file *, char *buf, size_t , loff_t *);
static ssize_t encrypter_write(struct file *, const char *buf, size_t , loff_t *);

/* Structure that declares the usual file access functions. */
struct file_operations encrypter_fops =
{
    read    :   encrypter_read,
    write   :   encrypter_write,
    open    :   encrypter_open,
    release :   encrypter_release,
};

/* Declaration of the init and exit functions. */
module_init(encrypter_init);
module_exit(encrypter_exit);


/* Global variables of the driver */

/* Major number. */
int encrypter_major = 60;

/* Buffer to store data. */
char *encrypter_buffer;

/*
 * Initialization:
 *  1. Register device driver
 *  2. Allocate buffer
 *  3. Initialize buffer
 */
int encrypter_init(void)
{
    int result;

    printk(KERN_INFO "Inserting encrypter module (major number %d)\n", encrypter_major);

    /* Registering device. */
    result = register_chrdev(encrypter_major, "encrypter", &encrypter_fops);
    if (result < 0)
    {
        printk(KERN_INFO "encrypter: cannot obtain major number %d\n", encrypter_major);
        return result;
    }



    /* Allocating memory for the buffer. */
    encrypter_buffer = kmalloc(BUF_LEN, GFP_KERNEL);
    if (!encrypter_buffer)
    {
        result = -ENOMEM;
        goto fail_no_mem;
    }

    memset(encrypter_buffer, 0, BUF_LEN);
    

    return 0;

fail_no_mem:
    unregister_chrdev(encrypter_major, "encrypter");
    return result;
}

/*
 * Cleanup:
 *  1. Free buffer
 *  2. Unregister device driver
 */
void encrypter_exit(void)
{
    printk(KERN_INFO "Removing encrypter module\n");

    /* Freeing buffer memory. */
    if (encrypter_buffer)
    {
        kfree(encrypter_buffer);
    }

    /* Freeing the major number. */
    unregister_chrdev(encrypter_major, "encrypter");
    printk(KERN_INFO "Encrypter module removed\n");	
}

/* File open function. */
static int encrypter_open(struct inode *inode, struct file *filp)
{
    /* Initialize driver variables here. */

    /* Reset the device here. */

    /* Success. */
    return 0;
}

/* File close function. */
static int encrypter_release(struct inode *inode, struct file *filp)
{
    /* Free memory allocated in encrypter_open here. */

    /* Success. */
    return 0;
}

/*
 * File read function
 *  Parameters:
 *   filp  - a type file structure;
 *   buf   - a buffer, from which the user space function (fread) will read;
 *   len - a counter with the number of bytes to transfer, which has the same
 *           value as the usual counter in the user space function (fread);
 *   f_pos - a position of where to start reading the file;
 *  Operation:
 *   The encrypter_read function transfers data from the driver buffer (encrypter_buffer)
 *   to user space with the function copy_to_user.
 */
static ssize_t encrypter_read(struct file *filp, char *buf, size_t len, loff_t *f_pos)
{
    /* Size of valid data in memory - data to send in user space. */
    int data_size = 0;

    if (*f_pos == 0)
    {
        /* Get size of valid data. */
        data_size = strlen(encrypter_buffer);

        /* Send data to user space. */
        if (copy_to_user(buf, encrypter_buffer, data_size) != 0)
        {
            return -EFAULT;
        }
        else
        {
            (*f_pos) += data_size;

            return data_size;
        }
    }
    else
    {
        return 0;
    }
}

void rotate_array_left(char array[], int size, int n) {
	if(n >= size) n = n % size;
	char *tmp_array = (char*)kmalloc(n * sizeof(char), GFP_KERNEL);
	memcpy(tmp_array, array, n * sizeof(char)); // temporarily save leading n elements
	memmove(array, array + n, (size - n) * sizeof(char)); // shift array to the left
	memmove(array + size - n, tmp_array, n * sizeof(char)); // append saved
	kfree(tmp_array);
}

/*
 * File write function
 *  Parameters:
 *   filp  - a type file structure;
 *   buf   - a buffer in which the user space function (fwrite) will write;
 *   len - a counter with the number of bytes to transfer, which has the same
 *           values as the usual counter in the user space function (fwrite);
 *   f_pos - a position of where to start writing in the file;
 *  Operation:
 *   The function copy_from_user transfers the data from user space to kernel space.
 */
static ssize_t encrypter_write(struct file *filp, const char *buf, size_t len, loff_t *f_pos)
{
    //int i = 0;

    /* Reset memory. */
    memset(encrypter_buffer, 0, BUF_LEN);

    /* Get data from user space.*/
    if (copy_from_user(encrypter_buffer, buf, len) != 0)
    {
        return -EFAULT;
    }
    else
    {
        if(ee){
		rotate_array_left(encrypter_buffer, strlen(encrypter_buffer), howmany);
        }
        return len;
    }
}

