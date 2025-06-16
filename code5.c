//CODE-5
#include <stdio.h>
#include <string.h>
#include <tee_client_api.h>

#define TA_UUID_1 { 0x38ba0cdc, 0xdf0e, 0x11e4, { 0x98, 0x69, 0x23, 0x3f, 0xb6, 0xae, 0x47, 0x95 } }
#define CMD_ENCRYPT_INIT 1

int main(void)
{
    TEEC_Context context;
    TEEC_Session session;
    TEEC_Operation op;
    TEEC_Result res;
    TEEC_UUID uuid = TA_UUID_1;
    uint32_t origin;

    // IV: 16-byte aligned buffer
    unsigned char iv[16] = "123456789012345";

    // Initialize context
    res = TEEC_InitializeContext(NULL, &context);
    if (res != TEEC_SUCCESS) {
        printf("Failed to initialize context: 0x%x\n", res);
        return 1;
    }

    // Open session
    res = TEEC_OpenSession(&context, &session, &uuid,
                           TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
    if (res != TEEC_SUCCESS) {
        printf("Failed to open session: 0x%x, origin 0x%x\n", res, origin);
        TEEC_FinalizeContext(&context);
        return 1;
    }

    // Setup operation
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(
        TEEC_VALUE_INPUT,     // key ID as value param
        TEEC_MEMREF_TEMP_INPUT, // IV
        TEEC_NONE,
        TEEC_NONE);

    op.params[0].value.a = 1; // Key ID 1

    op.params[1].tmpref.buffer = iv;
    op.params[1].tmpref.size = sizeof(iv);

    // Invoke ENCRYPT_INIT
    printf("Invoking CMD_ENCRYPT_INIT...\n");
    res = TEEC_InvokeCommand(&session, CMD_ENCRYPT_INIT, &op, &origin);
    if (res != TEEC_SUCCESS) {
        printf("InvokeCommand failed: 0x%x, origin 0x%x\n", res, origin);
    } else {
        printf("ENCRYPT_INIT succeeded.\n");
    }

    // Cleanup
    TEEC_CloseSession(&session);
    TEEC_FinalizeContext(&context);

    return 0;
}
