//CODE-7 the one taken from mediatek source
#include <stdio.h>
#include <string.h>
#include <tee_client_api.h>

static const TEEC_UUID UUIDS[] = {
    { 0x38ba0cdc, 0xdf0e, 0x11e4, { 0x98, 0x69, 0x23, 0x3f, 0xb6, 0xae, 0x47, 0x95 } },
    { 0x989850bf, 0x4663, 0x9dcd, { 0x39, 0x4c, 0x07, 0xa4, 0x5f, 0x46, 0x33, 0xd2 } },
    { 0xf50ebeef, 0x5a8c, 0x490a, { 0xa2, 0x64, 0xc1, 0xb8, 0x6a, 0xa2, 0x34, 0x78 } }
};

#define TA_CMD_GET_RANDOM  0x0

int main(void) {
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_Result res;
    uint32_t err_origin;
    char random_buf[16];
    int i;

    TEEC_InitializeContext(NULL, &ctx);

    for (i = 0; i < 3; i++) {
        printf("Testing UUID %d...\n", i);

        res = TEEC_OpenSession(&ctx, &sess, &UUIDS[i], TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
        if (res != TEEC_SUCCESS) {
            printf("  OpenSession failed (res=0x%x origin=0x%x)\n", res, err_origin);
            continue;
        }

        memset(&op, 0, sizeof(op));
        op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
        op.params[0].tmpref.buffer = random_buf;
        op.params[0].tmpref.size = sizeof(random_buf);

        res = TEEC_InvokeCommand(&sess, TA_CMD_GET_RANDOM, &op, &err_origin);
        if (res == TEEC_SUCCESS) {
            printf("  This is the trusted_keys TA! Random data received.\n");
        } else {
            printf("  InvokeCommand failed (res=0x%x origin=0x%x)\n", res, err_origin);
        }

        TEEC_CloseSession(&sess);
    }

    TEEC_FinalizeContext(&ctx);
    return 0;
}
