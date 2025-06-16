//CODE-1

#include <stdio.h>
#include <string.h>
#include "tee_client_api.h"

#define TA_HELLO_WORLD_UUID \
    { 0x38ba0cdc, 0xdf0e, 0x11e4, \
        { 0x98, 0x69, 0x23, 0x3f, 0xb6, 0xae, 0x47, 0x95 } }

#define CMD_HELLO_WORLD 0  // Assuming command ID 0 exists

int main(void) {
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Result res;
    TEEC_UUID uuid = TA_HELLO_WORLD_UUID;
    TEEC_Operation op;
    uint32_t err_origin;

    printf("Initializing context...\n");
    res = TEEC_InitializeContext(NULL, &ctx);
    printf("Result: 0x%x\n", res);
    if (res != TEEC_SUCCESS) {
        printf("TEEC_InitializeContext failed with code 0x%x\n", res);
        return 1;
    }

    printf("Opening session to TA...\n");
    res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    printf("Result: 0x%x\n", res);
    if (res != TEEC_SUCCESS) {
        printf("TEEC_OpenSession failed with code 0x%x, origin 0x%x\n", res, err_origin);
        TEEC_FinalizeContext(&ctx);
        return 1;
    }

    printf("Session opened successfully.\n");

    // Set up operation (test value exchange)
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    op.params[0].value.a = 1234;

    printf("Invoking command CMD_HELLO_WORLD...\n");
    res = TEEC_InvokeCommand(&sess, CMD_HELLO_WORLD, &op, &err_origin);
    printf("Invoke Result: 0x%x\n", res);

    if (res == TEEC_SUCCESS) {
        printf("Command succeeded. Returned value: %d\n", op.params[0].value.a);
    } else {
        printf("Invoke failed with code 0x%x, origin 0x%x\n", res, err_origin);
    }

    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);
    return 0;
}
