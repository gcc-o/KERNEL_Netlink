#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define NETLINK_MYPROTO 17
#define ERRRET          -1
#define MAX_PAYLOAD     1024  /* maximum payload size*/

int main()
{
    int                 ntl_sock;
    unsigned int        sequence;   // Характирезует порядковый номер
                                    // посылаемого сообщения
    struct sockaddr_nl  addr_src;
    struct sockaddr_nl  addr_dest;
    struct nlmsghdr     *nlhdr;
    struct msghdr       msghdr;
    struct iovec        vec;        // Необходима, потому что netlink
                                    // подразумевает разряженную 
                                    // посылку/прием сообщений
    /*
     * Поскольку netlink основан на передаче дейтаграм можно выставить
     * SOCK_DGRAM или SOCK_RAW, но разницы не будет.
     */
    ntl_sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_MYPROTO);
    if (ntl_sock == -1){
        perror("Failed to create netlink socket");
        return ERRRET;  
    }

    /*
     * Аналогично сокетам семества AF_INET необходимо произвести именование
     * сокета, чтобы мы могли получать сообщения
     */
    memset(&addr_src, 0, sizeof(addr_src));
    addr_src.nl_family = AF_NETLINK;
    addr_src.nl_pid = getpid();
    if (bind(ntl_sock, (struct sockaddr *)&addr_src, sizeof(addr_src)) == -1){
        close(ntl_sock);
        perror("Failed to bind netlink socket");
        return ERRRET;
    }
    
    memset(&addr_dest, 0, sizeof(addr_dest));
    addr_dest.nl_family = AF_NETLINK;
    sequence = 0;

    /*
     * Структуры nlmsghdr, msghdr и iovec должны заполняться перед каждой
     * посылкой, а выделение памяти происходить с учетом смещения. 
     * см linux/netlink.h
     *
     * Для отправки необходимо заполнить область следующего вида
     * (msghdr( iovec( nlmsghdr ) ) )
     */
    nlhdr = malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if (!nlhdr){
        close(ntl_sock);
        perror("Failed to allocate memory for nlmsghdr");
        return ERRRET;
    }
    memset(nlhdr, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlhdr->nlmsg_len   = NLMSG_SPACE(MAX_PAYLOAD);
    nlhdr->nlmsg_pid   = getpid();
    nlhdr->nlmsg_flags = NLM_F_REQUEST;
    nlhdr->nlmsg_type  = NLMSG_MIN_TYPE + 1;
    /*
     * В данном случае использование поля последовательности излишне, но
     * оставлено для демострации. 
     */
    nlhdr->nlmsg_seq   = sequence++;
    strcpy(NLMSG_DATA(nlhdr), "Hello, kernel! I'm user!\0");

    memset(&vec, 0, sizeof(vec));
    vec.iov_base = (void *)nlhdr;
    vec.iov_len  = nlhdr->nlmsg_len;

    memset(&msghdr, 0, sizeof(struct msghdr));
    msghdr.msg_name    = (void *)&addr_dest;
    msghdr.msg_namelen = sizeof(addr_dest);
    msghdr.msg_iov     = &vec;
    msghdr.msg_iovlen  = 1;

    if (sendmsg(ntl_sock, &msghdr, 0) == -1)
        perror("Failed sendmsg to kernel");

    if (recvmsg(ntl_sock, &msghdr, 0) == -1)
        perror("Failed recvmsg to kernel");
    printf("Received message: %s\n", (char *)NLMSG_DATA(nlhdr));

    close(ntl_sock);
    return 0;
}
