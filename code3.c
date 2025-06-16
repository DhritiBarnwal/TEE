//CODE-3
#include <stdio.h>
#include <string.h>
#include <tee_client_api.h>
#include <tee_shared_data_types.h>

#define TA_UUID_1 { 0x38ba0cdc, 0xdf0e, 0x11e4, { 0x98, 0x69, 0x23, 0x3f, 0xb6, 0xae, 0x47, 0x95 } }
#define CMD_CRYPTO_OPERATION 1

int main(void)
{
    TEEC_Context context;
    TEEC_Session session;
    TEEC_Operation op;
    TEEC_Result res;
    TEEC_UUID uuid = TA_UUID_1;
    TEEC_SharedMemory shm;
    uint32_t origin;

    // Initialize context
    res = TEEC_InitializeContext(NULL, &context);
    if (res != TEEC_SUCCESS) {
        printf("Failed to initialize context: 0x%x\n", res);
        return 1;
    }
    printf("Context initialized successfully\n");

    // Open session to the TA
    res = TEEC_OpenSession(&context, &session, &uuid,
                           TEEC_LOGIN_USER, NULL, NULL, &origin);
    if (res != TEEC_SUCCESS) {
        printf("TEEC_OpenSession failed: 0x%x, origin 0x%x\n", res, origin);
        TEEC_FinalizeContext(&context);
        return 1;
    }
    printf("Session opened successfully\n");

    // Allocate shared memory for input/output
    shm.size = 4096;
    shm.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
    printf("Attempting to allocate shared memory of size %zu\n", shm.size);
    res = TEEC_AllocateSharedMemory(&context, &shm);
    if (res != TEEC_SUCCESS) {
        printf("Failed to allocate shared memory: 0x%x\n", res);
        TEEC_CloseSession(&session);
        TEEC_FinalizeContext(&context);
        return 1;
    }

    // Write input data to shared memory
    memset(shm.buffer, 0, shm.size);
    strcpy((char *)shm.buffer, "Hello from normal world");

    // Prepare operation struct
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    op.params[0].memref.parent = &shm;
    op.params[0].memref.size = shm.size;

    // Invoke crypto operation command (CMD 1)
    printf("Invoking Command 1...\n");
    res = TEEC_InvokeCommand(&session, CMD_CRYPTO_OPERATION, &op, &origin);
    if (res != TEEC_SUCCESS) {
        printf("InvokeCommand failed: 0x%x, origin 0x%x\n", res, origin);
    } else {
        printf("TA response: %s\n", (char *)shm.buffer);
    }

    // Cleanup
    TEEC_ReleaseSharedMemory(&shm);
    TEEC_CloseSession(&session);
    TEEC_FinalizeContext(&context);

    return 0;
}
