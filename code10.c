//CODE-10
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tee_client_api.h>
#include <pkcs11_ta.h>

// UUID for pkcs11 TA
static const TEEC_UUID pkcs11_uuid = {
    0xf50ebeef, 0x5a8c, 0x490a,
    { 0xa2, 0x64, 0xc1, 0xb8, 0x6a, 0xa2, 0x34, 0x78 }
};

// Command ID for initializing token — defined in pkcs11_ta.h as CKS_INITIALIZE (example: 0x00000001)
#define PKCS11_CMD_INITIALIZE 0x00000001

int main(void)
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    uint32_t err_origin;

    // Initialize context
    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS) {
        printf("TEEC_InitializeContext failed with code 0x%x\n", res);
        return 1;
    }

    // Open a session with pkcs11 TA
    res = TEEC_OpenSession(&ctx, &sess, &pkcs11_uuid,
                           TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS) {
        printf("TEEC_OpenSession failed with code 0x%x, origin 0x%x\n", res, err_origin);
        TEEC_FinalizeContext(&ctx);
        return 1;
    }

    printf("Session opened successfully with pkcs11 TA.\n");

    // Prepare an operation (no params for initialize)
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(
        TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);

    // Invoke CKS_Initialize command
    res = TEEC_InvokeCommand(&sess, PKCS11_CMD_INITIALIZE, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        printf("TEEC_InvokeCommand (INITIALIZE) failed with code 0x%x, origin 0x%x\n", res, err_origin);
    } else {
        printf("PKCS11 TA initialized successfully.\n");
    }

    // Close session and finalize context
    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);

    return 0;
}
