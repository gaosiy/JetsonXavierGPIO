// jetsonGPIO.c
// update 202603 optimized

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "jetsonGPIO.h"

static int buildGpioPath(char *buf, size_t size, jetsonGPIO gpio, const char *fileName)
{
    if (buf == NULL || fileName == NULL) {
        errno = EINVAL;
        return -1;
    }

    int ret = snprintf(buf, size, SYSFS_GPIO_DIR "/gpio%d/%s", gpio, fileName);
    if (ret < 0 || (size_t)ret >= size) {
        errno = ENAMETOOLONG;
        return -1;
    }

    return 0;
}

static int writeSysfsFile(const char *path, const char *value)
{
    if (path == NULL || value == NULL) {
        errno = EINVAL;
        return -1;
    }

    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        return -1;
    }

    size_t len = strlen(value);
    ssize_t written = write(fd, value, len);
    int savedErrno = errno;

    close(fd);

    if (written < 0) {
        errno = savedErrno;
        return -1;
    }

    if ((size_t)written != len) {
        errno = EIO;
        return -1;
    }

    return 0;
}

static int writeSysfsIntFile(const char *path, unsigned int value)
{
    return writeSysfsFile(path, value ? "1" : "0");
}

static int writeGpioAttr(jetsonGPIO gpio, const char *attr, const char *value)
{
    char path[MAX_BUF];

    if (buildGpioPath(path, sizeof(path), gpio, attr) < 0) {
        perror("buildGpioPath");
        return -1;
    }

    if (writeSysfsFile(path, value) < 0) {
        char errbuf[128];
        snprintf(errbuf, sizeof(errbuf), "writeGpioAttr gpio%d/%s", gpio, attr);
        perror(errbuf);
        return -1;
    }

    return 0;
}

//
// gpioExport
// Export the given gpio to userspace
// Return: Success = 0 ; Failure = -1
int gpioExport(jetsonGPIO gpio)
{
    char path[] = SYSFS_GPIO_DIR "/export";
    char buf[32];

    int len = snprintf(buf, sizeof(buf), "%d", gpio);
    if (len < 0 || (size_t)len >= sizeof(buf)) {
        errno = EINVAL;
        perror("gpioExport snprintf");
        return -1;
    }

    if (writeSysfsFile(path, buf) < 0) {
        char errbuf[128];
        snprintf(errbuf, sizeof(errbuf), "gpioExport unable to export gpio%d", gpio);
        perror(errbuf);
        return -1;
    }

    return 0;
}

//
// gpioUnexport
// Unexport the given gpio from userspace
// Return: Success = 0 ; Failure = -1
int gpioUnexport(jetsonGPIO gpio)
{
    char path[] = SYSFS_GPIO_DIR "/unexport";
    char buf[32];

    int len = snprintf(buf, sizeof(buf), "%d", gpio);
    if (len < 0 || (size_t)len >= sizeof(buf)) {
        errno = EINVAL;
        perror("gpioUnexport snprintf");
        return -1;
    }

    if (writeSysfsFile(path, buf) < 0) {
        char errbuf[128];
        snprintf(errbuf, sizeof(errbuf), "gpioUnexport unable to unexport gpio%d", gpio);
        perror(errbuf);
        return -1;
    }

    return 0;
}

//
// gpioSetDirection
// Set the direction of the GPIO pin
// Return: Success = 0 ; Failure = -1
int gpioSetDirection(jetsonGPIO gpio, unsigned int out_flag)
{
    return writeGpioAttr(gpio, "direction", out_flag ? "out" : "in");
}

//
// gpioSetValue
// Set the value of the GPIO pin to 1 or 0
// Return: Success = 0 ; Failure = -1
int gpioSetValue(jetsonGPIO gpio, unsigned int value)
{
    return writeGpioAttr(gpio, "value", value ? "1" : "0");
}

//
// gpioGetValue
// Get the value of the requested GPIO pin ; value return is 0 or 1
// Return: Success = 0 ; Failure = -1
int gpioGetValue(jetsonGPIO gpio, unsigned int *value)
{
    if (value == NULL) {
        errno = EINVAL;
        perror("gpioGetValue null pointer");
        return -1;
    }

    int fd;
    char path[MAX_BUF];
    char ch;

    if (buildGpioPath(path, sizeof(path), gpio, "value") < 0) {
        perror("gpioGetValue build path");
        return -1;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        char errbuf[128];
        snprintf(errbuf, sizeof(errbuf), "gpioGetValue unable to open gpio%d", gpio);
        perror(errbuf);
        return -1;
    }

    ssize_t rd = read(fd, &ch, 1);
    int savedErrno = errno;
    close(fd);

    if (rd < 0) {
        errno = savedErrno;
        perror("gpioGetValue read");
        return -1;
    }

    if (rd != 1) {
        errno = EIO;
        perror("gpioGetValue short read");
        return -1;
    }

    *value = (ch == '0') ? 0U : 1U;
    return 0;
}

//
// gpioSetEdge
// Set the edge of the GPIO pin
// Valid edges: "none", "rising", "falling", "both"
// Return: Success = 0 ; Failure = -1
int gpioSetEdge(jetsonGPIO gpio, const char *edge)
{
    if (edge == NULL) {
        errno = EINVAL;
        perror("gpioSetEdge null edge");
        return -1;
    }

    if (strcmp(edge, "none") != 0 &&
        strcmp(edge, "rising") != 0 &&
        strcmp(edge, "falling") != 0 &&
        strcmp(edge, "both") != 0) {
        errno = EINVAL;
        perror("gpioSetEdge invalid edge");
        return -1;
    }

    return writeGpioAttr(gpio, "edge", edge);
}

//
// gpioOpen
// Open the given pin for reading
// Returns: file descriptor on success, -1 on failure
int gpioOpen(jetsonGPIO gpio)
{
    int fd;
    char path[MAX_BUF];

    if (buildGpioPath(path, sizeof(path), gpio, "value") < 0) {
        perror("gpioOpen build path");
        return -1;
    }

    fd = open(path, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        char errbuf[128];
        snprintf(errbuf, sizeof(errbuf), "gpioOpen unable to open gpio%d", gpio);
        perror(errbuf);
        return -1;
    }

    return fd;
}

//
// gpioClose
// Close the given file descriptor
int gpioClose(int fileDescriptor)
{
    return close(fileDescriptor);
}

//
// gpioActiveLow
// Set the active_low attribute of the GPIO pin to 1 or 0
// Return: Success = 0 ; Failure = -1
int gpioActiveLow(jetsonGPIO gpio, unsigned int value)
{
    return writeGpioAttr(gpio, "active_low", value ? "1" : "0");
}
