#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>
#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<net/if.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<netinet/if_ether.h>
#include<netinet/udp.h>
#include<linux/if_packet.h>
#include<arpa/inet.h>
#define NETLINK_USER 30 // same customized protocol as in my kernel module
#define MAX_PAYLOAD 1024 // maximum payload size
#define DESTMAC0	0x00
#define DESTMAC1	0xe0
#define DESTMAC2	0x4c
#define DESTMAC3	0x82
#define DESTMAC4	0x48
#define DESTMAC5	0x26 
#define destination_ip 192.168.227.34
void get_eth_index();
/*
@brief : This function lets us find the index of the interface we want to use in order to send the packet by using 
an ioctl called SIOCGIFINDEX that puts the interface index into the ifindex attribute in the ifreq struct.
@param ifreq_i : the struct of the ifreq we use to configure the network interface.
@param IFNAMSIZ : the name of the interface in the ifreq struct.
@return : returns the index of the interface into the ifreq struct since it's a void function.
*/
void get_mac();
/*
@brief: This function is getting us the MAC address of the interface by using another ioctl called SIOCGIFHWADDR.
@param : ifreq_c : another ifreq struct by with as a different variable otherwise we will get errors when using the ioctl macros.
@param : eth : the ethernet struct, 
@param : sendbuff : the buffer we use to send the information through the raw socket.
@param : totallen: the length of the buffer.
@return: returns the mac address of the interface into the struct since it's a void function.
*/
void get_data(char* str3);
/*
@brief :  This function puts the data the the user got from the kernel into the sendbuffer.
@param : totallen : the length of the buffer.
@return: doesn't return anything.
*/
void get_udp(char* str2);
/*
@brief :  This function constructs the udp header and putting the fields into the structure like we did for the rest of the headers.
@param : totallen : the length of the buffer.
@return: doesn't return anything.
*/
void get_ip(char* str1);
/*
@brief : This function is getting the ip address of the interface by using the SIOCGIFADDR macro.
@param :iph : the ip header, we parse it afterwords with the sendbuffer.
@param : totallen: the length of the buffer
*/
int sendpacket(char* data);
/*
@brief : This function is used to send the packet to another device.
@param : data : the buffer we send through the raw socket.
@return : returns -1 incase of an error with allocating the buffer or if the socket connection has failed. returns 1 otherwise.
*/
int connectsocket();
/*
@brief: This function configures the netlink sockets settings and connecting the socket the user space.
@param : sockfd : if this is negative, it means there's an error in the socket.
@return : can return -1 incase of an error.
*/
int sendmessage();
/*
@brief : Sends back a message to the userspace using netlink raw socket.
@param : ret : the number of bytes sent.
@return : the function returns -1 incase there's an error when sending, 0 otherwise.
*/