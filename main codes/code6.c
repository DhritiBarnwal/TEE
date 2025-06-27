//CODE-6
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "tee_client_api.h"

// TA UUIDs on your MediaTek Genio 1200 kit
static const TEEC_UUID ta_uuids[] = {
    { 0x38ba0cdc, 0xdf0e, 0x11e4, { 0x98, 0x69, 0x23, 0x3f, 0xb6, 0xae, 0x47, 0x95 } },
    { 0x989850bf, 0x4663, 0x9dcd, { 0x39, 0x4c, 0x07, 0xa4, 0x5f, 0x46, 0x33, 0xd2 } },
    { 0xf50ebeef, 0x5a8c, 0x490a, { 0xa2, 0x64, 0xc1, 0xb8, 0x6a, 0xa2, 0x34, 0x78 } }
};

// Command definitions
#define CMD_ENCRYPT_INIT   1
#define CMD_ENCRYPT_UPDATE 2
#define CMD_ENCRYPT_FINAL  3
#define CMD_DIGEST_INIT    4
#define CMD_DIGEST_UPDATE  5
#define CMD_DIGEST_FINAL   6

// Cryptographic operation function implementation
TEEC_Result libraryFunction(
    const TEEC_UUID* ta_uuid,
    uint8_t const * inputBuffer,
    uint32_t inputSize,
    uint8_t* outputBuffer,
    uint32_t outputSize,
    uint8_t* digestBuffer
) {
    TEEC_Context context;
    TEEC_Session session;
    TEEC_Operation operation;
    TEEC_Result result;
    TEEC_SharedMemory commsSM, inputSM, outputSM;
    uint8_t* ivPtr;

    result = TEEC_InitializeContext(NULL, &context);
    if (result != TEEC_SUCCESS) {
        printf("Failed to initialize context: 0x%08X\n", result);
        return result;
    }

    result = TEEC_OpenSession(
        &context, &session, ta_uuid,
        TEEC_LOGIN_USER, NULL, NULL, NULL
    );
    if (result != TEEC_SUCCESS) {
        printf("Failed to open session: 0x%08X\n", result);
        TEEC_FinalizeContext(&context);
        return result;
    }

    // [a] Communication buffer
    commsSM.size = 20;
    commsSM.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
    result = TEEC_AllocateSharedMemory(&context, &commsSM);
    if (result != TEEC_SUCCESS) goto cleanup_session;

    // [b] Input buffer
    // inputSM.size = inputSize;
    // inputSM.flags = TEEC_MEM_INPUT;
    // inputSM.buffer = (void*)inputBuffer;
    // result = TEEC_RegisterSharedMemory(&context, &inputSM);
    // if (result != TEEC_SUCCESS) goto cleanup_comms;
    inputSM.size = inputSize;
    inputSM.flags = TEEC_MEM_INPUT;
    result = TEEC_AllocateSharedMemory(&context, &inputSM);
    if (result != TEEC_SUCCESS) goto cleanup_comms;
    memcpy(inputSM.buffer, inputBuffer, inputSize);


    // [c] Output buffer
    // outputSM.size = outputSize;
    // outputSM.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
    // outputSM.buffer = outputBuffer;
    // result = TEEC_RegisterSharedMemory(&context, &outputSM);
    // if (result != TEEC_SUCCESS) goto cleanup_input;
    outputSM.size = outputSize;
    outputSM.flags = TEEC_MEM_OUTPUT;
    result = TEEC_AllocateSharedMemory(&context, &outputSM);
    if (result != TEEC_SUCCESS) goto cleanup_input;


    // [4] Encrypt Init
    memset(&operation, 0, sizeof(operation));
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_PARTIAL_INPUT, TEEC_NONE, TEEC_NONE);
    operation.params[0].value.a = 1; // key_id = 1
    operation.params[1].memref.parent = &commsSM;
    operation.params[1].memref.offset = 0;
    operation.params[1].memref.size = 16;
    ivPtr = (uint8_t*)commsSM.buffer;
    memset(ivPtr, 0, 16);

    result = TEEC_InvokeCommand(&session, CMD_ENCRYPT_INIT, &operation, NULL);
    if (result != TEEC_SUCCESS) goto cleanup_output;

    // Digest Init
    result = TEEC_InvokeCommand(&session, CMD_DIGEST_INIT, NULL, NULL);
    if (result != TEEC_SUCCESS) goto cleanup_output;

    // [5] Encrypt Update
    memset(&operation, 0, sizeof(operation));
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_MEMREF_PARTIAL_OUTPUT, TEEC_NONE, TEEC_NONE);
    operation.params[0].memref.parent = &inputSM;
    operation.params[1].memref.parent = &outputSM;
    operation.params[1].memref.offset = 0;
    operation.params[1].memref.size = outputSize;

    result = TEEC_InvokeCommand(&session, CMD_ENCRYPT_UPDATE, &operation, NULL);
    if (result != TEEC_SUCCESS) goto cleanup_output;

    // [5b] Digest Update
    memset(&operation, 0, sizeof(operation));
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    operation.params[0].memref.parent = &outputSM;
    operation.params[0].memref.offset = 0;
    operation.params[0].memref.size = operation.params[1].memref.size;

    result = TEEC_InvokeCommand(&session, CMD_DIGEST_UPDATE, &operation, NULL);
    if (result != TEEC_SUCCESS) goto cleanup_output;

    // [6] Finalize Encrypt
    result = TEEC_InvokeCommand(&session, CMD_ENCRYPT_FINAL, NULL, NULL);
    if (result != TEEC_SUCCESS) goto cleanup_output;

    // [6b] Finalize Digest
    memset(&operation, 0, sizeof(operation));
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_OUTPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    operation.params[0].memref.parent = &commsSM;
    operation.params[0].memref.offset = 0;
    operation.params[0].memref.size = 20;

    result = TEEC_InvokeCommand(&session, CMD_DIGEST_FINAL, &operation, NULL);
    if (result != TEEC_SUCCESS) goto cleanup_output;

    memcpy(digestBuffer, commsSM.buffer, 20);

cleanup_output:
    TEEC_ReleaseSharedMemory(&outputSM);
cleanup_input:
    TEEC_ReleaseSharedMemory(&inputSM);
cleanup_comms:
    TEEC_ReleaseSharedMemory(&commsSM);
cleanup_session:
    TEEC_CloseSession(&session);
    TEEC_FinalizeContext(&context);

    return result;
}

// Main program
int main() {
    const char *inputMessage = "Hello TEE Crypto!";
    uint8_t inputBuffer[64] = {0};
    uint8_t encryptedBuffer[64] = {0};
    uint8_t digestBuffer[20] = {0};

    size_t inputLen = strlen(inputMessage);
    memcpy(inputBuffer, inputMessage, inputLen);

    printf("Invoking TA UUID: %08X-%04X-%04X-XXXXXXXXXXXX\n",
           ta_uuids[0].timeLow, ta_uuids[0].timeMid, ta_uuids[0].timeHiAndVersion);

    TEEC_Result res = libraryFunction(
        &ta_uuids[0],
        inputBuffer,
        inputLen,
        encryptedBuffer,
        sizeof(encryptedBuffer),
        digestBuffer
    );

    if (res == TEEC_SUCCESS) {
        printf("Encryption & Digest successful!\n");
        printf("Encrypted data:\n");
        for (int i = 0; i < inputLen; i++)
            printf("%02X ", encryptedBuffer[i]);
        printf("\nDigest (SHA-1):\n");
        for (int i = 0; i < 20; i++)
            printf("%02X ", digestBuffer[i]);
        printf("\n");
    } else {
        printf("Operation failed: 0x%08X\n", res);
    }

    return 0;
}
