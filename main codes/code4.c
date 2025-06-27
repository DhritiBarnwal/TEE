//CODE-4
#include <stdio.h>
#include <string.h>
#include <tee_client_api.h>
#include <tee_shared_data_types.h>

#define TA_UUID_1 { 0xf50ebeef, 0x5a8c, 0x490a, { 0xa2, 0x64, 0xc1, 0xb8, 0x6a, 0xa2, 0x34, 0x78 } }

int main(void)
{
    TEEC_Context context;
    TEEC_Session session;
    TEEC_Operation op;
    TEEC_Result res;
    TEEC_UUID uuid = TA_UUID_1;
    uint32_t origin;
    uint32_t cmd_id;

    // Initialize context
    res = TEEC_InitializeContext(NULL, &context);
    if (res != TEEC_SUCCESS) {
        printf("Failed to initialize context: 0x%x\n", res);
        return 1;
    }

    // Open session to the TA
    res = TEEC_OpenSession(&context, &session, &uuid,
                           TEEC_LOGIN_USER, NULL, NULL, &origin);
    if (res != TEEC_SUCCESS) {
        printf("TEEC_OpenSession failed: 0x%x, origin 0x%x\n", res, origin);
        TEEC_FinalizeContext(&context);
        return 1;
    }

    // Zero out operation struct
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);

    // Test command IDs 0-10
    for (cmd_id = 0; cmd_id <= 10; cmd_id++) {
        printf("\nInvoking Command ID %u...\n", cmd_id);

        res = TEEC_InvokeCommand(&session, cmd_id, &op, &origin);

        if (res == TEEC_SUCCESS) {
            printf("Command %u succeeded!\n", cmd_id);
        } else {
            printf("Command %u failed: 0x%x, origin 0x%x\n", cmd_id, res, origin);
        }
    }

    // Cleanup
    TEEC_CloseSession(&session);
    TEEC_FinalizeContext(&context);

    return 0;
}
