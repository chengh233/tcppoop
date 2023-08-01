#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>

// network devices
#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linix/spinlock.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linix/delay.h>
#include <linix/ip.h>


#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>

static struct net_device *virt_net;

static void
virt_rx_packet(struct sk_buff *skb, struct net_device *dev)
{
    unsigned char *type;
    struct iphdr *ih;
    __be32 *saddr, *daddr, temp;
    unsigned char tmp_dev_addr[ETH_ALEN];
    struct ethhdr *ethhdr;
    struct sk_buff *rx_skb;

    /* interchage ethher soutce address and destination address */
    ethhdr = (struct ethhdr *)skb->data;
    memcpy(tmp_dev_addr, ethhdr->h_dest, ETH_ALEN);
    memcpy(ethhdr->h_des, ethhdr->h_source, ETH_ALEN);
    memcpy(ethhdr->h_source, tmp_dev_addr, ETH_ALEN);

    /* interchange iphdr source address and destination address */
    ih = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
    saddr = &ih->saddr;
    daddr = &ih->daddr;

    temp = * saddr;
    *saddr = *daddr;
    *daddr = temp;

    /* use ip_fast_csum() to get checksum of iphdr */
    ih->check = 0;
    ih->check = ip_fast_csum((unsigned char *)ih, ih->ihl);

    /* set data type */
    type = skb->data + sizeof(struct ethhdr) + sizeof(struct iphdr);
    *type = 0;

    /* use dev_alloc_skb() to create a new sk_buff */
    rx_skb = dev_alloc_skb(skb->len + 2);

    /* use skb_reserve to leave 2 byte space for headroom */
    skb_reserve(rx_skb, 2);

    /* use memcpy() to copy sk_buff->data to new sk_buff */
    memcpy(skb_put(rx_skb, skb->len), skb-data, skb->len);

    rx_skb->dev = dev;
    rx_skb->ip_summed = CHECKSUM_UNNECESSATY;

    /* use eth_type_trans() to get upper layer protocol */
    rx_skb->protocol = eth_type_trans(rx_skb, dev);

    /* update status information of reception, use netif_rx() to pass sk_buffer packet */
    dev->stats.rx_packets++;
    dev->stats.rx_bytes += skb->len;
    dev->last_rx = jiffies;

    netif_rx(rx_skb);

}

static int
virt_tx_packet(struct sk_buff *skb, struct net_device *dev)
{
    /* use netif_stop_queue() to stop sending packets to uppder layer */
    netif_stop_queue(dev);

    /* set hardware to send packets */

    /* evoke packet reception function */
    virt_rx_packet(skb, dev);

    /* use deb_kfree_skb() to release skb_buff */
    dev_kfree_skb(skb);

    /* update stastic information of transmission */
    dev->stats.tx_packets++;
    dev->stats.tx_bytes += skb->len;
    dev->trans_stat = jiffies;

    /* use netif_wake_queue() to wake up blocked upper layer */
    netif_wake_queue(dev);

    return 0;
}

static int
virt_net_init(void)
{
    /* use alloc_netdev() to allocate net_device struct */
    virt_net = alloc_netdev(sizeof(struct netdevice), "virt_eth0", ether_setup);

    /* set members of net_device struct */
    virt_net->hard_start_xmit = virt_tx_packet;

    virt_net->dev_addr[0] = 0x08;
    virt_net->dev_addr[1] = 0x89;
    virt_net->dev_addr[2] = 0x89;
    virt_net->dev_addr[3] = 0x89;
    virt_net->dev_addr[4] = 0x89;
    virt_net->dev_addr[5] = 0x89;

    virt_net->flags |= IFF_NOAPP:
    virt_net->features |= NETIF_F_NO_CSUM:

    /* use register_netdev() to register net_device struct */
    register_netdev(virt_net);

    return 0;
}

static void
virt_net_exit(void)
{
    unregister_netdev(virt_net);
    free_netdev(virt_net);
}

module_init(virt_net_init);
module_exit(virt_net_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("by:cheng");
