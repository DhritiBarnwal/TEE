//CODE-9
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tee_client_api.h>

#define MAX_CANDIDATE_UUIDS 3
#define MAX_BUFFER_SIZE 512

// Your candidate UUIDs
TEEC_UUID uuids[MAX_CANDIDATE_UUIDS] = {
    { 0x38ba0cdc, 0xdf0e, 0x11e4, { 0x98, 0x69, 0x23, 0x3f, 0xb6, 0xae, 0x47, 0x95 } },
    { 0x989850bf, 0x4663, 0x9dcd, { 0x39, 0x4c, 0x07, 0xa4, 0x5f, 0x46, 0x33, 0xd2 } },
    { 0xf50ebeef, 0x5a8c, 0x490a, { 0xa2, 0x64, 0xc1, 0xb8, 0x6a, 0xa2, 0x34, 0x78 } }
};

void test_uuid(TEEC_Context *ctx, TEEC_UUID uuid)
{
    TEEC_Session sess;
    TEEC_Result res;
    TEEC_Operation op;
    uint32_t origin;
    uint8_t out_buf[MAX_BUFFER_SIZE] = {0};

    printf("\n== Testing UUID: %08x-%04x-%04x-...\n",
           uuid.timeLow, uuid.timeMid, uuid.timeHiAndVersion);

    res = TEEC_OpenSession(ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
    if (res != TEEC_SUCCESS) {
        printf("  OpenSession failed (res=0x%08x origin=0x%x)\n", res, origin);
        return;
    }
    printf("  Session opened successfully.\n");

    // Test command 0 with no parameters
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);

    res = TEEC_InvokeCommand(&sess, 0, &op, &origin);
    printf("  InvokeCommand(0) no params: res=0x%08x origin=0x%x\n", res, origin);

    // Test command 0 with an output memref (for GetRandom)
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    op.params[0].tmpref.buffer = out_buf;
    op.params[0].tmpref.size = 16;

    res = TEEC_InvokeCommand(&sess, 0, &op, &origin);
    printf("  InvokeCommand(0) with output memref: res=0x%08x origin=0x%x\n", res, origin);

    // Test command 1 with input/output memrefs (for Seal)
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT,
                                     TEEC_NONE, TEEC_NONE);

    char test_key[] = "test_key_data";
    op.params[0].tmpref.buffer = test_key;
    op.params[0].tmpref.size = sizeof(test_key);

    uint8_t seal_out_buf[MAX_BUFFER_SIZE] = {0};
    op.params[1].tmpref.buffer = seal_out_buf;
    op.params[1].tmpref.size = sizeof(seal_out_buf);

    res = TEEC_InvokeCommand(&sess, 1, &op, &origin);
    printf("  InvokeCommand(1) with in/out memref: res=0x%08x origin=0x%x\n", res, origin);

    // Close session
    TEEC_CloseSession(&sess);
    printf("  Session closed.\n");
}

int main(void)
{
    TEEC_Context ctx;
    TEEC_Result res;

    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS) {
        printf("TEEC_InitializeContext failed with code 0x%08x\n", res);
        return 1;
    }

    for (int i = 0; i < MAX_CANDIDATE_UUIDS; i++) {
        test_uuid(&ctx, uuids[i]);
    }

    TEEC_FinalizeContext(&ctx);
    return 0;
}
