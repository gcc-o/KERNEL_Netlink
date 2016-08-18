#ifndef _NETLINK_KERN_MODULE_H
#define _NETLINK_KERN_MODULE_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <net/netlink.h>

#define NETLINK_MYPROTO 17
#define MAX_PAYLOAD     1024  /* maximum payload size*/

extern struct net 					init_net;
static struct sock 					*ntl_sock;
static struct netlink_kernel_cfg 	ntl_cfg;

int init_module(void);
void cleanup_module(void);

static void netlink_input(struct sk_buff *skb);
static int netlink_repack_skb(struct sk_buff *, struct nlmsghdr *);

#endif	/* _NETLINK_KERN_MODULE_H */