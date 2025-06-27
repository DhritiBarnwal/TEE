//CODE-11
#include <stdio.h>
#include <string.h>
#include <tee_client_api.h>
#include <pkcs11_ta.h>

// pkcs11 TA UUID: f50ebeef-5a8c-490a-a264-c1b86aa23478
#define PKCS11_UUID { 0xf50ebeef, 0x5a8c, 0x490a, { 0xa2, 0x64, 0xc1, 0xb8, 0x6a, 0xa2, 0x34, 0x78 } }
#define PKCS11_TA_CMD_INVOKE 0

// struct pkcs11_invoke {
//     uint32_t func_id;
//     uint32_t session_handle;
//     uint32_t ctrl_size;
//     uint32_t data_size;
// };
struct __attribute__((__packed__)) pkcs11_invoke {
    uint32_t func_id;
    uint32_t session_handle;
    uint32_t ctrl_size;
    uint32_t data_size;
};

#define PKCS11_CMD_C_GENERATERANDOM 0x00000150

int main(void) {
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_UUID uuid = PKCS11_UUID;
    TEEC_Result res;
    TEEC_Operation op;
    uint32_t origin;

    struct pkcs11_invoke cmd_buf;
    uint8_t random_buf[16]; // 16 random bytes

    printf("Initializing context...\n");
    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS) {
        printf("TEEC_InitializeContext failed: 0x%x\n", res);
        return 1;
    }

    printf("Opening session with pkcs11 TA...\n");
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
                           TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
    if (res != TEEC_SUCCESS) {
        printf("TEEC_OpenSession failed: 0x%x origin 0x%x\n", res, origin);
        TEEC_FinalizeContext(&ctx);
        return 1;
    }

    printf("Session opened successfully.\n");

    memset(&cmd_buf, 0, sizeof(cmd_buf));
    cmd_buf.func_id = PKCS11_CMD_C_GENERATERANDOM;
    cmd_buf.session_handle = 0;  // 0 because C_GenerateRandom is stateless
    cmd_buf.ctrl_size = 0;
    cmd_buf.data_size = sizeof(random_buf);

    memset(&random_buf, 0, sizeof(random_buf));
    memset(&op, 0, sizeof(op));

    op.paramTypes = TEEC_PARAM_TYPES(
        TEEC_MEMREF_TEMP_INOUT, // command struct
        TEEC_MEMREF_TEMP_OUTPUT, // random data output
        TEEC_NONE, TEEC_NONE);

    op.params[0].tmpref.buffer = &cmd_buf;
    op.params[0].tmpref.size   = sizeof(cmd_buf);

    op.params[1].tmpref.buffer = random_buf;
    op.params[1].tmpref.size   = sizeof(random_buf);

    printf("Invoking C_GenerateRandom command...\n");
    res = TEEC_InvokeCommand(&sess, PKCS11_TA_CMD_INVOKE, &op, &origin);
    if (res != TEEC_SUCCESS) {
        printf("TEEC_InvokeCommand failed: 0x%x origin 0x%x\n", res, origin);
    } else {
        printf("Random data generated:\n");
        for (int i = 0; i < sizeof(random_buf); i++)
            printf("%02x ", random_buf[i]);
        printf("\n");
    }

    printf("Closing session...\n");
    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);
    return 0;
}
