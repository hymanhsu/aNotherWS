//
// Created by xuhuahai on 2017/5/2.
//

#include <ws/common.h>
#include <simple_log.h>

#include <fcntl.h>

#include <net/if.h>
#include <arpa/inet.h>
#include <ifaddrs.h>


/**
 * 获取随机数，填充指定的缓冲区，可以设定长度，
 * 该方法可以比较方便的获取随机数,
 * 例如: int rand; get_random(&rand,sizeof(int));
 * @param buf  用于保存随机数的内存区域首指针
 * @param n  需要填充的随机数字节的个数
 * @return 返回0表示操作成功
 */
int get_random(void * buf, size_t n){
    int fd = open("/dev/urandom", O_RDONLY);
    if(-1 == fd){
        return -1;
    }
    read(fd, buf, n);
    close(fd);
    return 0;
}


inline int day_of_year(int y, int m, int d)
{
    int k, leap, s;
    int days[13] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
    leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    s = d;
    for (k = 1; k<m; k++)
    {
        s += days[k];
    }
    if (leap == 1 && m>2)
        s += 1;
    return s;
}


/**
 * 获取当天的时间秒数
 * @return
 */
uint get_local_time_secs()
{
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec % 86400);
}


/**
 * 从头比较字符串,返回相同的长度,不区分大小写
 * @param s1 字符串1
 * @param s2 字符串2
 * @return 如果<0，则表示字符串2大; 如果>0，则表示字符串1大，如果=0，则表示相等
 */
int strinstr(const char* s1, const char* s2)
{
    const char* cur = s1;
    while (s1 && *s1>0 && s2 && *s2>0)
    {
        if (*s1 == *s2 || (isalpha(*s1) && isalpha(*s2) && abs(*s1 - *s2) == 32))
            s1++, s2++;
        else
            break;
    }
    return s1 - cur;
}


//获取IP地址
static int get_ip_v4_and_v6_linux(int family, char *address)
{
    struct ifaddrs *ifap0, *ifap;
    char buf[101];
    struct sockaddr_in *addr4;
    struct sockaddr_in6 *addr6;
    if (NULL == address)
        return -1;

    if (getifaddrs(&ifap0))
        return -1;
    for (ifap = ifap0; ifap != NULL; ifap = ifap->ifa_next)
    {
        //if(strcmp(interface,ifap->ifa_name)!=0) continue;
        if (ifap->ifa_addr == NULL) continue;
        if ((ifap->ifa_flags & IFF_UP) == 0) continue;
        if (family != ifap->ifa_addr->sa_family) continue;

        if (AF_INET == ifap->ifa_addr->sa_family)
        {
            addr4 = (struct sockaddr_in *)ifap->ifa_addr;
            if (NULL != inet_ntop(ifap->ifa_addr->sa_family, (void *)&(addr4->sin_addr), buf, 100))
            {
                //printf("IP4 family=%d address=%s\n",ifap->ifa_addr->sa_family,address);
                if (strcmp(buf, "127.0.0.1") == 0)
                    continue;
                strcpy(address, buf);
                if (ifap0) { freeifaddrs(ifap0); ifap0 = NULL; }
                return 0;
            }
            //else
            //    break;
        }
        else if (AF_INET6 == ifap->ifa_addr->sa_family)
        {
            addr6 = (struct sockaddr_in6*) ifap->ifa_addr;
            if (IN6_IS_ADDR_MULTICAST(&addr6->sin6_addr)) {/*printf("IN6_IS_ADDR_MULTICAST\n");*/ continue; }
            //if(IN6_IS_ADDR_LINKLOCAL(&addr6->sin6_addr)){/*本地IPv6地址*/printf("IN6_IS_ADDR_LINKLOCAL\n"); /*continue;*/}
            if (IN6_IS_ADDR_LOOPBACK(&addr6->sin6_addr)) {/*::1*/continue; }
            if (IN6_IS_ADDR_UNSPECIFIED(&addr6->sin6_addr)) {/*printf("IN6_IS_ADDR_UNSPECIFIED\n");*/ continue; }
            if (IN6_IS_ADDR_SITELOCAL(&addr6->sin6_addr)) {/*printf("IN6_IS_ADDR_SITELOCAL\n");*/ continue; }

            if (NULL != inet_ntop(ifap->ifa_addr->sa_family, (void *)&(addr6->sin6_addr), buf, 100))
            {
                //printf("IP6 family=%d address=%s\n",ifap->ifa_addr->sa_family,protoBuf);
                //if(strcmp(protoBuf,"::1")==0) //上面已经过滤了
                //	continue;
                strcpy(address, buf);
                if (ifap0) { freeifaddrs(ifap0); ifap0 = NULL; }
                return 0;
            }
            //else
            //   break;
        }
    }
    if (ifap0) { freeifaddrs(ifap0); ifap0 = NULL; }
    return -1;
}


/**
 * 获取IPv4地址(第一个IPv4)
 * @return
 */
const char* get_ipv4_addr()
{
    static char CurIP[17] = { 0 };
    get_ip_v4_and_v6_linux(AF_INET, CurIP);
    return CurIP;
}


/**
 * 获取IPv6地址 (第一个IPv6)
 * @return
 */
const char* get_ipv6_addr()
{
    static char CurIP[50] = { 0 };
    get_ip_v4_and_v6_linux(AF_INET6, CurIP);
    return CurIP;
}


/**
 * 从一个文件描述字读一行('\n'作为行分隔符)
 * @param fd 文件描述字
 * @param buffer 保存行的字节缓冲区
 * @param n  字节缓冲区的长度
 * @return 读到的内容的长度, <=0表示发生了错误
 */
ssize_t read_line(int fd, void *buffer, size_t n)
{
    ssize_t numRead;    /* # of bytes fetched by last read()*/
    size_t  totRead;    /* Total bytes read so far*/
    char *buf;
    char ch;

    if (n == 0 || buffer == NULL){
        errno = EINVAL;
        return -1;
    }

    buf = static_cast<char *>(buffer);
    totRead = 0;
    for(;;){
        numRead = read(fd, &ch, 1);
        if (numRead == -1){
            if (errno == EINTR)     /*Interrupted ----> restart read()*/
                continue;
            else
                return -1;          /* some other error*/
        }else if (numRead == 0){    /* EOF*/
            if (totRead == 0)       /* No bytes read so far, return 0*/
                return 0;
            else                    /* Some bytes read so far, add '\0'*/
                break;
        }else{
            if (ch == '\n')
                break;

            if (totRead < n-1){     /* Discard > (n-1) bytes */
                totRead++;
                *buf++=ch;
            }
        }
    }

    *buf = '\0';
    return totRead;
}


/**
 * 把一个长度分为多个部分
 * @param size    需要分割的长度
 * @param count   指定分割的份数
 * @param result  保存分割结果
 */
void split_ont_size_to_multi_parts(size_t size, size_t count, split_result * result){
    if(size==0 || result == NULL){
        return;
    }
    if(count == 0){
        count = DEFAULT_SPLIT_COUNT;
    }
    if(count > MAX_SPLIT_COUNT){
        count = MAX_SPLIT_COUNT;
    }
    result->split_count = count;
    size_t average = size / count;
    size_t added = 0;
    srand((unsigned)time(NULL));
    for(int n= 0; n<count-1; ++n){
        size_t temp = 0;
        while(!temp){
            temp = (size_t)(rand() % average);
        }
        result->split_result[n] = temp;
        added += temp;
    }
    result->split_result[count-1] = size - added;
}

/**
 * 解析拿到目录和文件名
 * @param configFilePath
 * @return
 */
std::pair<std::string,std::string> parseFilePath(const std::string& configFilePath){
    std::size_t found = configFilePath.find_last_of("/");
    if(found == std::string::npos){
        return std::make_pair(std::string("."),configFilePath);
    }else{
        return std::make_pair(configFilePath.substr(0,found),configFilePath.substr(found+1));
    }
}

/**
 * 保存数据
 * @param saved_data_directory
 * @param buffer
 * @param buffer_len
 */
void save_proto_data(const char * saved_data_directory, void * buffer, size_t buffer_len){
    #define FILE_NAME_LEN   1024
    time_t endTime = time(NULL);
    char file_name[FILE_NAME_LEN];
    snprintf(file_name,FILE_NAME_LEN,"%s%ld",saved_data_directory,endTime);
    FILE *fp;
    fp = fopen(file_name, "wb");
    if(!fp){
        LOG_ERROR("open file %s failed : %s",file_name,strerror(errno));
        return;
    }
    fwrite(buffer, sizeof(char),buffer_len,fp);
    fclose(fp);
}


/**
 * 往网络写2字节长度
 * @param length_value
 * @param output
 */
void write_two_byte_length(uint16_t length_value, uchar * output){
    uint16_t value = htons(length_value);
    uchar * temp = (uchar *)&value;
    *output     = *temp;
    *(output+1) = *(temp+1);
}

/**
 * 往网络写4字节长度
 * @param length_value
 * @param output
 */
void write_four_byte_length(uint32_t length_value, uchar * output){
    uint32_t value = htonl(length_value);
    uchar * temp = (uchar *)&value;
    *output     = *temp;
    *(output+1) = *(temp+1);
    *(output+2) = *(temp+2);
    *(output+3) = *(temp+3);
}

/**
 * 从网络读2字节长度
 * @param input
 * @return
 */
uint16_t read_two_byte_length(uchar * input){
    uint16_t * temp = (uint16_t *)input;
    uint16_t value = ntohs(*temp);
    return value;
}

/**
 * 从网络读4字节长度
 * @param input
 * @return
 */
uint32_t read_four_byte_length(uchar * input){
    uint32_t * temp = (uint32_t *)input;
    uint32_t value = ntohl(*temp);
    return value;
}
