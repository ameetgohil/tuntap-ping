#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h> 
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>

#define BUFSIZE 2000
int tun_alloc(char *dev, int flags) {

  struct ifreq ifr;
  int fd, err;
  char *clonedev = "/dev/net/tun";

  if( (fd = open(clonedev , O_RDWR)) < 0 ) {
    perror("Opening /dev/net/tun");
    return fd;
  }

  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_flags = flags;

  if (*dev) {
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  }

  if( (err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
    perror("ioctl(TUNSETIFF)");
    close(fd);
    return err;
  }

  strcpy(dev, ifr.ifr_name);

  return fd;
}

// int tun_alloc(char *dev, int flags) {

//   struct ifreq ifr;
//   int fd, err;
//   char const *clonedev = "/dev/net/tun";

//   /* Arguments taken by the function:
//    * char *dev: the name of an interface (or '\0'. MUST have enough
//    * spcace to hold the interface name if '\0' is passed
//    * int flags: interface flags (eg, IFF_TUN etc.)
//    */

//   /* open the clone device */
//   if((fd = open(clonedev,O_RDWR)) < 0 ) {
//     return fd;
//   }

//   /* preparation of the struct ifr, of type "struct ifreq" */
//   memset(&ifr, 0, sizeof(ifr));

//   if(*dev) {
//     /* if a dev name was specified, put it in the structurel; otherwise,
//      * the kernel will try ot allocate the "next" device of the
//      * specified type */
//     strncpy(ifr.ifr_name, dev, IFNAMSIZ);
//   }

//   /* try to create the device */
//   if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
//     close(fd);
//     return err;
//   }

//   /* if the operation was successful, write back the name of the
//    * interface to the variable "dev", so the caller can know
//    * it. Note that the caller MUST reserve space in *dev (see calling
//    * code below) */
//   strcpy(dev, ifr.ifr_name);

//   /* this is the special file descriptor that the caller will use tot talk
//    * with the virutal interface */
//   return fd;
// }

void example_crate_taptun(){
  char tun_name[IFNAMSIZ];
  char tap_name[IFNAMSIZ];
  char *a_name;
  int tunfd, tapfd;
  
  strcpy(tun_name, "tun1");
  tunfd = tun_alloc(tun_name, IFF_TUN); /* tun interface */

  strcpy(tap_name, "tap44");
  tapfd = tun_alloc(tap_name, IFF_TAP); /* tap interface */

  /*  a_name = malloc(IFNAMSIZ);
      a_name[0]='\0';*/
  tapfd = tun_alloc(a_name, IFF_TAP); /* let the kernel pick a name */
  
}

short onesSum(unsigned short data, unsigned short result){
  unsigned int sum;
  sum = data + result;
  sum = sum + (sum>>16);
  printf("%4x %4x %4x %4x %4x, ", data, result, sum, sum>>16, sum+(sum>>16));
  return sum&0xFFFF;
}

short checksum(char *buf, int len){
  unsigned short result = 0;
  unsigned short data = 0;
  unsigned short buf1, buf2;
  for(int i=20;i<len;i=i+2){
    buf1 = buf[i]&0xFF;
    buf2 = buf[i+1]&0xFF;
    data = buf2<<8 | buf1;
    //printf("%4x %2x %2x,", data, buf[i], buf[i+1]);
    result = onesSum(data,result);
  }
  return result;
}


int main(){
  char tun_name[IFNAMSIZ];
  int tun_fd;
  char buffer[BUFSIZE];
  char srcip[4];
  char destip[4];
  uint16_t nread, nwrite, plength;
  unsigned short chksum;
  
  /* Connect to the device */
  strcpy(tun_name, "tun7");
  tun_fd = tun_alloc(tun_name, IFF_TUN | IFF_NO_PI);  /* tun interface */

  if(tun_fd < 0){
    perror("Allocating interface");
    exit(1);
  }
  for(int i=0; i<BUFSIZE;i++){
    buffer[i]=0;
  }
  /* Now read data coming from the kernel */
  while(1) {
    /* Note that "buffer" should be at least the MTU size of the interface, eg 1500 bytes */
    nread = read(tun_fd,buffer,sizeof(buffer));
    //extract src ip
    srcip[0]=buffer[12];
    srcip[1]=buffer[13];
    srcip[2]=buffer[14];
    srcip[3]=buffer[15];
    //extract dest ip
    destip[0]=buffer[16];
    destip[1]=buffer[17];
    destip[2]=buffer[18];
    destip[3]=buffer[19];
    //replace src with dest
    buffer[12]=destip[0];
    buffer[13]=destip[1];
    buffer[14]=destip[2];
    buffer[15]=destip[3];
    //replace dest with src
    buffer[16]=srcip[0];
    buffer[17]=srcip[1];
    buffer[18]=srcip[2];
    buffer[19]=srcip[3];
    buffer[20]=0;
    buffer[22]+=0x8;
    //buffer[23]=0x00;
    //chksum=checksum(buffer,nread);

    //buffer[22]=(char)((chksum>>8)&0xFF);
    //buffer[23]=(char)chksum&0xFF;
    printf("checksum: %x buf22: %x buf23: %x\n", chksum, buffer[22],buffer[23]);
    printf("src ip %2d:%2d:%2d:%2d", srcip[0],srcip[1],srcip[2],srcip[3]);
    //    destip={buffer[16],buffer[17],buffer[18],buffer[19]};
    //write(tun_fd,buffer,nread);
    if(nread < 0) {
      perror("Reading from interface");
      close(tun_fd);
      exit(1);
    }
    
    /* Do whatever with the data */
    printf("Read %d bytes fromthe device %s:\n", nread, tun_name);
    //printf("%x\n",*buffer);
    /*for(int i=0; i<nread;i++){
      printf(" %2d ",buffer[i]);
      }*/
    printf("\n");
    write(tun_fd,buffer,nread);
    //exit(0);
  }
}
