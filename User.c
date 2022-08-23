#include "User.h"
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct nlmsghdr *nlh2 = NULL;
struct msghdr msg, resp;  
struct iovec iov, iov2;
int sock_fd;

int total_len=0,send_len;
struct ifreq ifreq_c,ifreq_i,ifreq_ip; 
int sock_raw;
unsigned char *sendbuff;
void get_eth_index()
{
	memset(&ifreq_i,0,sizeof(ifreq_i));
	strncpy(ifreq_i.ifr_name,"wlo1",IFNAMSIZ-1);

	if((ioctl(sock_raw,SIOCGIFINDEX,&ifreq_i))<0)
		printf("error in index ioctl reading");

	printf("index=%d\n",ifreq_i.ifr_ifindex);

}
void get_mac()
{
	memset(&ifreq_c,0,sizeof(ifreq_c));
	strncpy(ifreq_c.ifr_name,"wlo1",IFNAMSIZ-1);

	if((ioctl(sock_raw,SIOCGIFHWADDR,&ifreq_c))<0)
		printf("error in SIOCGIFHWADDR ioctl reading");

	printf("Mac= %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[0]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[1]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[2]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[3]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[4]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[5]));

	printf("ethernet packaging start ... \n");
	
	struct ethhdr *eth = (struct ethhdr *)(sendbuff);
  	eth->h_source[0] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[0]);
  	eth->h_source[1] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[1]);
   	eth->h_source[2] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[2]);
   	eth->h_source[3] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[3]);
   	eth->h_source[4] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[4]);
   	eth->h_source[5] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[5]);

   	eth->h_dest[0]    =  DESTMAC0;
   	eth->h_dest[1]    =  DESTMAC1;
   	eth->h_dest[2]    =  DESTMAC2;
  	eth->h_dest[3]    =  DESTMAC3;
   	eth->h_dest[4]    =  DESTMAC4;
   	eth->h_dest[5]    =  DESTMAC5;

   	eth->h_proto = htons(ETH_P_IP);   //protocol 0x800

   	printf("ethernet packaging done.\n");

	total_len+=sizeof(struct ethhdr);
}

void get_data(char* str3)
{
	printf("%sstr3\n",str3);
	sendbuff[total_len++]	=	*str3;
}

void get_udp(char* str2)
{
	printf("%sstr2\n",str2);
	struct udphdr *uh = (struct udphdr *)(sendbuff + sizeof(struct iphdr) + sizeof(struct ethhdr));

	uh->source	= htons(23451);
	uh->dest	= htons(23452);
	uh->check	= 0;

	total_len+= sizeof(struct udphdr);
	get_data(str2);
	uh->len		= htons((total_len - sizeof(struct iphdr) - sizeof(struct ethhdr)));

}

unsigned short checksum(unsigned short* buff, int _16bitword)
{
	unsigned long sum;
	for(sum=0;_16bitword>0;_16bitword--)
		sum+=htons(*(buff)++);
	do
	{
		sum = ((sum >> 16) + (sum & 0xFFFF));
	}
	while(sum & 0xFFFF0000);

	return (~sum);	
}
void get_ip(char* str1)
{
	printf("%sstr1\n",str1);
	memset(&ifreq_ip,0,sizeof(ifreq_ip));
	strncpy(ifreq_ip.ifr_name,"wlo1",IFNAMSIZ-1);
  	 if(ioctl(sock_raw,SIOCGIFADDR,&ifreq_ip)<0)
 	 {
		printf("error in SIOCGIFADDR \n");
	 }
	
	printf("%s\n",inet_ntoa((((struct sockaddr_in*)&(ifreq_ip.ifr_addr))->sin_addr)));

	struct iphdr *iph = (struct iphdr*)(sendbuff + sizeof(struct ethhdr));
	iph->ihl	= 5;
	iph->version	= 4;
	iph->tos	= 16;
	iph->id		= htons(10201);
	iph->ttl	= 64;
	iph->protocol	= 17;
	iph->saddr	= inet_addr(inet_ntoa((((struct sockaddr_in *)&(ifreq_ip.ifr_addr))->sin_addr)));
	iph->daddr	= inet_addr("destination_ip"); // put destination IP address
	total_len += sizeof(struct iphdr); 
	get_udp(str1);

	iph->tot_len	= htons(total_len - sizeof(struct ethhdr));
	iph->check	= htons(checksum((unsigned short*)(sendbuff + sizeof(struct ethhdr)), (sizeof(struct iphdr)/2)));
}
int SendPacket(char* data)
{
	printf("%sdata\n",data);
	sock_raw=socket(AF_PACKET,SOCK_RAW,IPPROTO_RAW);
	if(sock_raw == -1)
		printf("error in socket");

	sendbuff=(unsigned char*)malloc(64); // increase in case of large data.Here data is --> AA  BB  CC  DD  EE
	memset(sendbuff,0,64);

	get_eth_index();  // interface number
	get_mac();
	get_ip(data);

	struct sockaddr_ll sadr_ll;
	sadr_ll.sll_ifindex = ifreq_i.ifr_ifindex;
	sadr_ll.sll_halen   = ETH_ALEN;
	sadr_ll.sll_addr[0]  = DESTMAC0;
	sadr_ll.sll_addr[1]  = DESTMAC1;
	sadr_ll.sll_addr[2]  = DESTMAC2;
	sadr_ll.sll_addr[3]  = DESTMAC3;
	sadr_ll.sll_addr[4]  = DESTMAC4;
	sadr_ll.sll_addr[5]  = DESTMAC5;

	printf("sending...\n");
	send_len = sendto(sock_raw,sendbuff,64,0,(const struct sockaddr*)&sadr_ll,sizeof(struct sockaddr_ll));
		if(send_len<0)
		{
			printf("error in sending....sendlen=%d....errno=%d\n",send_len,errno);
			return -1;

		}
	total_len = 0;
}
int ConnectSocket(){
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER); 

    if(sock_fd < 0)
        return -1;

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */

    if(bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr))){
        perror("bind() error\n");
        close(sock_fd);
        return -1;
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;       /* For Linux Kernel */
    dest_addr.nl_groups = 0;    /* unicast */

    //nlh: contains "Hello" msg
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();  //self pid
    nlh->nlmsg_flags = 0; 

    //nlh2: contains received msg
    nlh2 = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh2, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh2->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh2->nlmsg_pid = getpid();  //self pid
    nlh2->nlmsg_flags = 0; 

    strcpy(NLMSG_DATA(nlh), "Hello this is a msg from userspace");   //put "Hello" msg into nlh

    iov.iov_base = (void *)nlh;         //iov -> nlh
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;  //msg_name is Socket name: dest
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;                 //msg -> iov
    msg.msg_iovlen = 1;

    iov2.iov_base = (void *)nlh2;         //iov -> nlh2
    iov2.iov_len = nlh2->nlmsg_len;
    resp.msg_name = (void *)&dest_addr;  //msg_name is Socket name: dest
    resp.msg_namelen = sizeof(dest_addr);
    resp.msg_iov = &iov2;                 //resp -> iov
    resp.msg_iovlen = 1;
}
int SendMessage(){
    int ret = sendmsg(sock_fd, &msg, 0);   
	if(ret < 1){
			printf("Error sending message\n");
			return -1;
		}
    printf("Waiting for an input\n");

    /* Read message from kernel */
    recvmsg(sock_fd, &resp, 0);  

    printf("Received %s\n", (char *) NLMSG_DATA(nlh2));  
	SendPacket((char *) NLMSG_DATA(nlh2));
    while (1) {
        ret = sendmsg(sock_fd, &msg, 0);   
		if(ret < 1){
			printf("Error sending message\n");
			return -1;
		}
        printf("Waiting for message from kernel\n");

        /* Read message from kernel */
        recvmsg(sock_fd, &resp, 0);  
        printf("Received message payload: %s\n", (char *)NLMSG_DATA(nlh2));   
		
		SendPacket((char *) NLMSG_DATA(nlh2));
        
    }
    close(sock_fd);

    return 0;
}


int main()
{
	system("make");
	system("insmod Module.ko");
    ConnectSocket();
    SendMessage();
    //SendPacket();
}
