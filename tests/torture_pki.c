#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "torture_pki.h"
#include <libssh/priv.h>

char *torture_pki_read_file(const char *filename)
{
    char *key;
    int fd;
    int size;
    int rc;
    struct stat sb;

    if (filename == NULL || filename[0] == '\0') {
        return NULL;
    }

    fd = open(filename, O_RDONLY);// | _O_BINARY);
    if (fd < 0) {
        return NULL;
    }

    rc = fstat(fd, &sb);
    if (rc != 0) {
        close(fd);
        return NULL;
    }

    key = malloc(sb.st_size + 1);
    if (key == NULL) {
        close(fd);
        return NULL;
    }

    size = sb.st_size;
    do
    {
      rc = read(fd, key + (sb.st_size - size), size);
      if (rc < 0)
      {
        key[sb.st_size - size] = 0;
        fprintf(stderr, "Can't read full size of pki file, %d remaining, got:\r\n%s", size, key);
        free(key);
        return NULL;
      }
      size -= rc;
    } while (size && rc); //on windows the the size might be to great
    close(fd);

    key[sb.st_size - size] = '\0';
    return key;
}

int torture_read_one_line(const char *filename, char *buffer, size_t len)
{
    FILE *fp;
    size_t nmemb;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1;
    }

    nmemb = fread(buffer, len - 2, 1, fp);
    if (nmemb != 0 || ferror(fp)) {
        fclose(fp);
        return -1;
    }
    buffer[len - 1] = '\0';

    fclose(fp);

    return 0;
}

/**
 * @internal
 *
 * Returns the character len of a public key string, omitting the comment part
 */
size_t torture_pubkey_len(const char *pubkey)
{
    const char *ptr;

    ptr = strchr(pubkey, ' ');
    if (ptr != NULL) {
        ptr = strchr(ptr + 1, ' ');
        if (ptr != NULL) {
            return ptr - pubkey;
        }
    }

    return 0;
}
