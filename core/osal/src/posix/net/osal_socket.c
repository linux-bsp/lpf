/************************************************************************
 * OSAL - socket系统调用封装实现（POSIX）
 ************************************************************************/

#include "osal.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

/*===========================================================================
 * Socket基本操作
 *===========================================================================*/

int32_t OSAL_socket(int32_t domain, int32_t type, int32_t protocol)
{
	int32_t result = socket(domain, type, protocol);
	return result;
}

int32_t OSAL_bind(int32_t sockfd, const osal_sockaddr_t *addr,
				  osal_size_t addrlen)
{
	union {
		const osal_sockaddr_t *osal_addr;
		const struct sockaddr *posix_addr;
	} addr_union;

	addr_union.osal_addr = addr;
	return bind(sockfd, addr_union.posix_addr, (socklen_t)addrlen);
}

int32_t OSAL_listen(int32_t sockfd, int32_t backlog)
{
	return listen(sockfd, backlog);
}

int32_t OSAL_accept(int32_t sockfd, osal_sockaddr_t *addr, osal_size_t *addrlen)
{
	union {
		osal_sockaddr_t *osal_addr;
		struct sockaddr *posix_addr;
	} addr_union;
	socklen_t len;

	len = addrlen ? (socklen_t)(*addrlen) : 0;
	addr_union.osal_addr = addr;
	int result = accept(sockfd, addr_union.posix_addr, addrlen ? &len : NULL);
	if (addrlen) {
		*addrlen = (osal_size_t)len;
	}
	return result;
}

int32_t OSAL_connect(int32_t sockfd, const osal_sockaddr_t *addr,
					 osal_size_t addrlen)
{
	union {
		const osal_sockaddr_t *osal_addr;
		const struct sockaddr *posix_addr;
	} addr_union;

	addr_union.osal_addr = addr;
	return connect(sockfd, addr_union.posix_addr, (socklen_t)addrlen);
}

osal_ssize_t OSAL_send(int32_t sockfd, const void *buf, osal_size_t len,
					   int32_t flags)
{
	return send(sockfd, buf, len, flags);
}

osal_ssize_t OSAL_recv(int32_t sockfd, void *buf, osal_size_t len,
					   int32_t flags)
{
	return recv(sockfd, buf, len, flags);
}

osal_ssize_t OSAL_sendto(int32_t sockfd, const void *buf, osal_size_t len,
						 int32_t flags, const osal_sockaddr_t *dest_addr,
						 osal_size_t addrlen)
{
	union {
		const osal_sockaddr_t *osal_addr;
		const struct sockaddr *posix_addr;
	} addr_union;

	addr_union.osal_addr = dest_addr;
	return sendto(sockfd, buf, len, flags, addr_union.posix_addr,
				  (socklen_t)addrlen);
}

osal_ssize_t OSAL_recvfrom(int32_t sockfd, void *buf, osal_size_t len,
						   int32_t flags, osal_sockaddr_t *src_addr,
						   osal_size_t *addrlen)
{
	socklen_t slen;
	union {
		osal_sockaddr_t *osal_addr;
		struct sockaddr *posix_addr;
	} addr_union;
	ssize_t result;

	slen = addrlen ? (socklen_t)(*addrlen) : 0;
	addr_union.osal_addr = src_addr;
	result = recvfrom(sockfd, buf, len, flags, addr_union.posix_addr,
					  addrlen ? &slen : NULL);
	if (addrlen) {
		*addrlen = (osal_size_t)slen;
	}
	return result;
}

int32_t OSAL_shutdown(int32_t sockfd, int32_t how)
{
	int32_t result = shutdown(sockfd, how);
	return result;
}

/*===========================================================================
 * Socket选项操作
 *===========================================================================*/

int32_t OSAL_setsockopt(int32_t sockfd, int32_t level, int32_t optname,
						const void *optval, osal_size_t optlen)
{
	return setsockopt(sockfd, level, optname, optval, (socklen_t)optlen);
}

int32_t OSAL_getsockopt(int32_t sockfd, int32_t level, int32_t optname,
						void *optval, osal_size_t *optlen)
{
	socklen_t len = (socklen_t)(*optlen);
	int result = getsockopt(sockfd, level, optname, optval, &len);
	*optlen = (osal_size_t)len;
	return result;
}

/*===========================================================================
 * 网络接口操作
 *===========================================================================*/

uint32_t OSAL_if_nametoindex(const char *ifname)
{
	uint32_t result = if_nametoindex(ifname);
	return result;
}

char *OSAL_if_indextoname(uint32_t ifindex, char *ifname)
{
	uint32_t index;
	char *result;

	index = ifindex;
	result = if_indextoname(index, ifname);
	return result;
}

/*===========================================================================
 * 字节序转换
 *===========================================================================*/

uint16_t OSAL_htons(uint16_t hostshort)
{
	return htons(hostshort);
}

uint32_t OSAL_htonl(uint32_t hostlong)
{
	return htonl(hostlong);
}

uint16_t OSAL_ntohs(uint16_t netshort)
{
	return ntohs(netshort);
}

uint32_t OSAL_ntohl(uint32_t netlong)
{
	return ntohl(netlong);
}

/*===========================================================================
 * IP地址转换
 *===========================================================================*/

int32_t OSAL_inet_pton(int32_t af, const char *src, void *dst)
{
	int32_t result = inet_pton(af, src, dst);
	return result;
}

const char *OSAL_inet_ntop(int32_t af, const void *src, char *dst,
						   uint32_t size)
{
	socklen_t len;

	len = (socklen_t)size;
	return inet_ntop(af, src, dst, len);
}
