#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 5240
#define TIME_THRESHOLD_MS 200

static long long get_current_timestamp_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

int main(void)
{
    char buffer[BUFFER_SIZE];

    memset(buffer, 'A', BUFFER_SIZE - 1);
    buffer[BUFFER_SIZE - 1] = '\0';

    while (1) {
        long long start_time;
        long long end_time;
        long long duration;

        start_time = get_current_timestamp_ms();

        printf("%s", buffer);

        end_time = get_current_timestamp_ms();
        duration = end_time - start_time;

        if (duration > TIME_THRESHOLD_MS) {
            printf(" [error: printf time  %lldms > 200ms]", duration);
        }

        printf("\n");
    }

    return 0;
}
