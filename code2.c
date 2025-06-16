//CODE-2
#include <stdio.h>
#include <tee_client_api.h>
#include <string.h>

// Define the UUIDs of known TAs
static const TEEC_UUID ta_uuids[] = {
    // 38ba0cdc-df0e-11e4-9869-233fb6ae4795
    { 0x38ba0cdc, 0xdf0e, 0x11e4, { 0x98, 0x69, 0x23, 0x3f, 0xb6, 0xae, 0x47, 0x95 } },
    // 989850bf-4663-9dcd-394c-07a45f4633d2
    { 0x989850bf, 0x4663, 0x9dcd, { 0x39, 0x4c, 0x07, 0xa4, 0x5f, 0x46, 0x3d, 0x2 } },
    // f50ebeef-5a8c-490a-a264-c1b86aa23478
    { 0xf50ebeef, 0x5a8c, 0x490a, { 0xa2, 0x64, 0xc1, 0xb8, 0x6a, 0xa2, 0x34, 0x78 } }
};

int main(void) {
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Result res;
    TEEC_Operation op;
    uint32_t origin;
    size_t i;

    printf("Initializing context...\n");
    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS) {
        printf("TEEC_InitializeContext failed: 0x%x\n", res);
        return 1;
    }

    for (i = 0; i < sizeof(ta_uuids) / sizeof(TEEC_UUID); i++) {
        printf("\n--- Testing TA UUID %zu ---\n", i + 1);
        res = TEEC_OpenSession(
            &ctx, &sess, &ta_uuids[i],
            TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);

        if (res != TEEC_SUCCESS) {
            printf("TEEC_OpenSession failed: 0x%x, origin 0x%x\n", res, origin);
            continue;
        }

        printf("Session opened successfully.\n");

        memset(&op, 0, sizeof(op));
        op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);

        printf("Invoking command 0...\n");
        res = TEEC_InvokeCommand(&sess, 0, &op, &origin);

        if (res == TEEC_SUCCESS) {
            printf("Invoke command 0 succeeded.\n");
        } else {
            printf("Invoke command 0 failed: 0x%x, origin 0x%x\n", res, origin);
        }

        TEEC_CloseSession(&sess);
    }

    TEEC_FinalizeContext(&ctx);
    printf("Done.\n");
    return 0;
}
