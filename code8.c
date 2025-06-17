//CODE-8
#include <stdio.h>
#include <string.h>
#include <tee_client_api.h>

// Candidate UUID to test — replace with other candidates to probe them
TEEC_UUID test_uuid = { 0x38ba0cdc, 0xdf0e, 0x11e4,
                        { 0x98, 0x69, 0x23, 0x3f, 0xb6, 0xae, 0x47, 0x95 } };

int main(void) {
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_Result res;
    uint32_t err_origin;

    // Initialize a context connecting us to the TEE
    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS) {
        printf("TEEC_InitializeContext failed with code 0x%x\n", res);
        return 1;
    }

    printf("Context initialized.\n");

    // Open a session to the Trusted Application
    res = TEEC_OpenSession(&ctx, &sess, &test_uuid,
                           TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS) {
        printf("TEEC_OpenSession failed with code 0x%x origin 0x%x\n", res, err_origin);
        TEEC_FinalizeContext(&ctx);
        return 1;
    }

    printf("Session opened successfully.\n");

    // Prepare operation struct (no params needed for PKCS11_TA_CMD_INITIALIZE)
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);

    // Invoke Command ID 0 (could be PKCS11_TA_CMD_INITIALIZE)
    res = TEEC_InvokeCommand(&sess, 0x00000000, &op, &err_origin);
    if (res == TEEC_SUCCESS) {
        printf("TEEC_InvokeCommand (CMD 0) succeeded.\n");
    } else {
        printf("TEEC_InvokeCommand (CMD 0) failed with code 0x%x origin 0x%x\n", res, err_origin);
    }

    // Close the session
    TEEC_CloseSession(&sess);
    printf("Session closed.\n");

    // Finalize the context
    TEEC_FinalizeContext(&ctx);
    printf("Context finalized.\n");

    return 0;
}
