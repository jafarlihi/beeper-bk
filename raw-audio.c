#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#include <fftw3.h>

static ssize_t loop_write(uint8_t *data, size_t size) {
    fftw_complex *out;
    double *in;
    fftw_plan p;
    in = (double*) malloc(sizeof(double) * size);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * size);

    for (int i = 0; i < size; i++) {
        in[i] = data[i];
    }

    p = fftw_plan_dft_r2c_1d(size, in, out, FFTW_ESTIMATE);
    fftw_execute(p);

    double max; int maxi;
    for (int i = 1; i < size / 4; i++) {
        printf("%d %f\n", i, sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]));
        if (sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]) > max) {
            max = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
            maxi = i;
        }
    }

    printf("MAX %d %f\n", maxi, max);

    fftw_destroy_plan(p);
    fftw_free(in); fftw_free(out);
}

int main(int argc, char*argv[]) {
    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 1
    };
    pa_simple *s = NULL;
    int error;

    if (!(s = pa_simple_new(NULL, argv[0], PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        goto finish;
    }

    uint8_t buf[88200];
    if (pa_simple_read(s, buf, sizeof(buf), &error) < 0) {
        fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
        goto finish;
    }

    loop_write(buf, sizeof(buf));
finish:
    if (s)
        pa_simple_free(s);
    return 0;
}
