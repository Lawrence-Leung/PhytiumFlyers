/*
 * Phytium's Remote Processor Control Driver
 *
 * Copyright (C) 2022 Phytium Technology Co., Ltd. - All Rights Reserved
 * Author: Shaojun Yang <yangshaojun@phytium.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms 
 * of the GNU General Public License version 2 as published by the Free Software Foundation.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>	//Offer POSIX OS API access
#include <poll.h>
#include <linux/rpmsg.h>
#include <errno.h>

static ssize_t write_full(int fd, void *buf, size_t count)
{
	ssize_t ret = 0;
	ssize_t total = 0;

	while (count) {
		ret = write(fd, buf, count);
		if (ret < 0) {
			if (errno == EINTR)
				continue;

			break;
		}

		count -= ret;
		buf += ret;
		total += ret;
	}   

	return total;
}

static ssize_t read_full(int fd, void *buf, size_t count)
{
	ssize_t res;

	do {
		res = read(fd, buf, count);
	} while (res < 0 && errno == EINTR);

	if ((res < 0 && errno == EAGAIN))
		return 0;

	if (res < 0)
		return -1; 

	return res;
}

static int count = 100, no; 

int main(int argc, char **argv)
{
	int ctrl_fd0, ctrl_fd1, rpmsg_fd0, rpmsg_fd1, ret;
	struct rpmsg_endpoint_info eptinfo0, eptinfo1;
	struct pollfd fds0, fds1;
	uint8_t buf0[32], buf1[32], r_buf0[32], r_buf1[32];

	//Opening endpoint0 controller
	ctrl_fd0 = open("/dev/rpmsg_ctrl_lawrence_10", O_RDWR | O_NONBLOCK);
	if (ctrl_fd0 < 0) {
		perror("open rpmsg_ctrl0 failed.\n");
		return -1;
	}

	//Opening endpoint1 controller
	ctrl_fd1 = open("/dev/rpmsg_ctrl_lawrence_0", O_RDWR | O_NONBLOCK);
	if (ctrl_fd1 < 0) {
		perror("open rpmsg_ctrl1 failed.\n");
		return -1;
	}
	
	//Name Inserting
	memcpy(eptinfo0.name, "hello", 32);
	eptinfo0.src = 11;
	eptinfo0.dst = 0;

	//Another Name's Insertation
	memcpy(eptinfo1.name, "channel1", 32);
	eptinfo1.src = 1;	//Attention!
	eptinfo1.dst = 0;

	ret = ioctl(ctrl_fd0, RPMSG_CREATE_EPT_IOCTL, eptinfo0);	
	if (ret != 0) {
		perror("ioctl RPMSG_CREATE_EPT_IOCTL failed.\n");
		goto err0;
	}

	ret = ioctl(ctrl_fd1, RPMSG_CREATE_EPT_IOCTL, eptinfo1); 
        if (ret != 0) {
                perror("ioctl RPMSG_CREATE_EPT_IOCTL failed.\n");
                goto err0;
        }	

	//Opening rpmsg file descriptor
	rpmsg_fd0 = open("/dev/rpmsg_lawrence_10", O_RDWR | O_NONBLOCK);
	if (rpmsg_fd0 < 0) {
		perror("open rpmsg0 failed.\n");
		goto err1;
	}

	rpmsg_fd1 = open("/dev/rpmsg_lawrence_0", O_RDWR | O_NONBLOCK);
        if (rpmsg_fd1 < 0) {
                perror("open rpmsg1 failed.\n");
                goto err1;
        }	

	//fds
	memset(&fds0, 0, sizeof(struct pollfd));
	fds0.fd = rpmsg_fd0;
	fds0.events |= POLLIN; 

	memset(&fds1, 0, sizeof(struct pollfd));
	fds1.fd = rpmsg_fd1;
	fds1.events |= POLLIN;

	/* receive message from remote processor. */
	while (count) {

		usleep(1000);
		snprintf(buf0, sizeof(buf0), "%s%d \r\n", "ENDPOINT0 No:", no);
		snprintf(buf1, sizeof(buf1), "%s%d \r\n", "ENDPOINT1 No:", ++no);
	
		/* send message to remote processor. */
	
		ret = write_full(rpmsg_fd0, buf0, sizeof(buf0));
		if (ret < 0) {
			perror("write_full failed.\n");
			goto err1;
		}
			
		ret = write_full(rpmsg_fd1, buf1, sizeof(buf1));
		if (ret < 0) {
				perror("write_full failed.\n");
				goto err1;
		}
		
		ret = poll(&fds0, 1, 100); //Timeout is set to 0 msecs.
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			goto err1;
		}


		ret = poll(&fds1, 1, 100); //Timeout is set to 0 msecs.
		if (ret < 0) {
				if (errno == EINTR)
						continue;
				goto err1;
		}

		usleep(1000);	
		
		memset(r_buf0, 0, 32);
		ret = read_full(rpmsg_fd0, r_buf0, 32);
		if (ret < 0) {
			perror("read_full failed.\n");
			goto err1;
		}

		memset(r_buf1, 0, 32);
                ret = read_full(rpmsg_fd1, r_buf1, 32);
                if (ret < 0) {
                        perror("read_full failed.\n");
                        goto err1;
                }

		/* output message */
		printf("Receive BUF0: %s\n", r_buf0);
		printf("Receive BUF1: %s\n\r\n", r_buf1);

		usleep(5000);
		count--;
	}

err1:
	close(rpmsg_fd0);
	close(rpmsg_fd1);
	/*
	//Unlink dynamically created dev files, for test.
	if (unlink("/dev/rpmsg0") == -1) {
		perror("unlink failed");
		return 1;
	}
        if (unlink("/dev/rpmsg1") == -1) {
                perror("unlink failed");
                return 1;
        }
	printf("All devs are successfully removed. \r\n");
	*/
err0:
	close(ctrl_fd0);
	close(ctrl_fd1);

	return 0;
}
