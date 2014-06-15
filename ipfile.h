#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

struct ifaddrs * ifAddrStruct=NULL;
struct ifaddrs * ifa=NULL;

string ipaddress (void);

string ipaddress (void)
{
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;
    int flag = 0;
    getifaddrs(&ifAddrStruct);
    string ans;



    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);


            if (strcmp(ifa->ifa_name,"eth0")==0)
            {
            ans = addressBuffer;
            //printf("%s\n",addressBuffer);
                //cout<<addressBuffer;
            flag = 1;
            return ans;
            }

        }
    }

    if(flag==0)
    {
    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);


             if (strcmp(ifa->ifa_name,"eth1")==0)
            {
                 ans = addressBuffer;
            //printf("%s\n",addressBuffer);
            flag = 2;
            return ans;
            }

        }
    }

    if(flag == 0)
    {
    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);


             if (strcmp(ifa->ifa_name,"lo")==0)
            {
                 ans = addressBuffer;
            //printf("%s\n",addressBuffer);
            flag = 3;
            return ans;
            }

        }
    }
    }
}

}
