/*
 *  ======== dnn_model.c ========
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <float.h>
#include <math.h>

/*
 *  ======== DNN Model ========
 */
typedef struct _DNN_MODEL {
    float weight1[60][128];
    float bias1[128];
    float weight2[128][9];
    float bias2[9];
}DNN_MODEL;


const char* dnn_path = "E:\\Work\\CS6501EOS\\Power-traces-analysis-on-CC2642R1\\dnn.dat";

DNN_MODEL *dnn;

int init_model()
{
    //printf("Read dnn from path: %s\n", dnn_path);
    /* Read DNN weights */
    FILE *infile;
    infile = fopen(dnn_path, "rb");

    if(infile == NULL) {
        return -1;
    }

    dnn = (DNN_MODEL*) malloc(sizeof(DNN_MODEL));

    fread(dnn, sizeof(DNN_MODEL), 1, infile);
    fclose(infile);

    //puts("Finish loading DNN model.\n");
    return 0;
}

float relu(float x) {
    return x > 0 ? x : 0;
}

void softmax(float *input, int input_len) {
    double m = -FLT_MAX;
    int i;
    for (i = 0; i < input_len; i++) {
        if (input[i] > m) {
            m = input[i];
        }
    }

    double sum = 0.0;
    for (i = 0; i < input_len; i++) {
        sum += expf(input[i] - m);
    }

    double offset = m + logf(sum);
    for (i = 0; i < input_len; i++) {
        input[i] = expf(input[i] - offset);
    }
}

int predict(float *value_buf, int buf_size)
{
    int i, j;

    float *layer1, *layer2;
    layer1 = (float *)malloc(sizeof(float) * 128);
    layer2 = (float *)malloc(sizeof(float) * 9);

    /* Layer 1 */
    // bias
    for (j = 0; j < 128; j++) {
        layer1[j] = dnn->bias1[j];
    }

    // weight
    for (i = 0; i < buf_size; i++) {
        for (j = 0; j < 128; j++) {
            layer1[j] += dnn->weight1[i][j] * value_buf[i];
        }
    }

    // relu activation
    for (j = 0; j < 128; j++) {
        layer1[j] = relu(layer1[j]);
    }

    /* Layer 2 */
    // bias
    for (j = 0; j < 9; j++) {
        layer2[j] = dnn->bias2[j];
    }

    // weight
    for (i = 0; i < 128; i++) {
        for (j = 0; j < 9; j++) {
            layer2[j] += dnn->weight2[i][j] * layer1[i];
        }
    }

    // softmax activation
    softmax(layer2, 9);
    int max_idx = 0;
    for (j = 1; j < 9; j++) {
        if (layer2[j] > layer2[max_idx]) {
            max_idx = j;
        }
    }

    /* Free layer1 and layer2 memory */
    free(layer1);
    free(layer2);

    return max_idx;
}

