#include "error.h"
#include "http_net.h"
#include "imgfs_server_service.h" // for DEFAULT_LISTENING_PORT
#include <stdio.h>

int main(void)
{
    int err = http_init(DEFAULT_LISTENING_PORT, NULL);
    if (err < 0) {
        fprintf(stderr, "http_init() failed\n");
        fprintf(stderr, "%s\n", ERR_MSG(err));
        return err;
    }
    printf("ImgFS server started on http://localhost:%u\n",
           DEFAULT_LISTENING_PORT);

    while ((err = http_receive()) == ERR_NONE);

    fprintf(stderr, "http_receive() failed\n");
    fprintf(stderr, "%s\n", ERR_MSG(err));

    return err;
}
