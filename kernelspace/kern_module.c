#include "kern_module.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ivannikov Igor");
MODULE_DESCRIPTION("Testing char dev module");

int init_module(void)
{
	ntl_cfg.input = netlink_input;
	ntl_sock = netlink_kernel_create(&init_net, NETLINK_MYPROTO, &ntl_cfg);
	if (!ntl_sock)
		goto err_exit;
	pr_info("init_module: initialization successful\n");
	return 0;

err_exit:
	pr_info("init_module: Failed to create netlink socket\n");
	return -1;
}

void cleanup_module(void)
{
	netlink_kernel_release(ntl_sock);
	pr_info("cleanup_module: release successful\n");
}

static void netlink_input(struct sk_buff *skb)
{
	/*
	 * Можно самому извлечь заголовок netlink, но предпочтительнее
	 * использовать соответствующую функцию.
	 */
	netlink_rcv_skb(skb, netlink_repack_skb);
}

static int netlink_repack_skb(struct sk_buff *skb, struct nlmsghdr *nlhdr)
{
	struct nlmsghdr *nlhdr_reply;
	struct sk_buff	*skb_reply;
	char		msg[] = "Hello, user! I'm kernel!\0";
	int		nlhdr_len;

	pr_info("netlink_repack_skb get: %s\n", (char *)NLMSG_DATA(nlhdr));

	/*
	 * Формируем структуру sk_buff для ответа. После выделения памяти под
	 * нее обязателен вызов skb_put, чтобы изменить поля, хранящие размер
	 * данных.
	 */
	nlhdr_len = NLMSG_SPACE(MAX_PAYLOAD);
	skb_reply = alloc_skb(nlhdr_len, GFP_KERNEL);
	if (!skb_reply)
		goto err_exit;
	skb_put(skb_reply, nlhdr_len);

	nlhdr_reply = (struct nlmsghdr *)skb_reply->data;
	memset(nlhdr_reply, 0, sizeof(*nlhdr_reply));
	nlhdr_reply->nlmsg_len		= nlhdr_len;
	nlhdr_reply->nlmsg_flags	= NLM_F_REQUEST;
	nlhdr_reply->nlmsg_seq		= nlhdr->nlmsg_seq + 1;
	nlhdr_reply->nlmsg_type		= NLMSG_MIN_TYPE + 1;
	strcpy(NLMSG_DATA(nlhdr_reply), msg);

	netlink_unicast(ntl_sock, skb_reply, nlhdr->nlmsg_pid, MSG_DONTWAIT);
	/* kfree_skb(skb); в данном случае sk_buff система чистит сама */
	return 0;

err_exit:
	pr_info("netlink_repack_skb: Failed to create sk_buff\n");
	return -1;
}