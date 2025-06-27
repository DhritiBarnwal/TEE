// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2017-2020, Linaro Limited
 */

/* BINARY_PREFIX is expected by teec_trace.h */
// #ifndef BINARY_PREFIX
// #define BINARY_PREFIX		"ckteec"
// #endif

// #include <errno.h>
// #include <inttypes.h>
// #include <pkcs11.h>
// #include <pkcs11_ta.h>
// #include <pthread.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/types.h>
// #include <tee_client_api.h>
// #include <teec_trace.h>
// #include <unistd.h>

// #include "ck_helpers.h"
// #include "invoke_ta.h"
// #include "local_utils.h"

// struct ta_context {
// 	pthread_mutex_t init_mutex;
// 	bool initiated;
// 	TEEC_Context context;
// 	TEEC_Session session;
// };

// static struct ta_context ta_ctx = {
// 	.init_mutex = PTHREAD_MUTEX_INITIALIZER,
// };

// bool ckteec_invoke_initiated(void)
// {
// 	return ta_ctx.initiated;
// }

// TEEC_SharedMemory *ckteec_alloc_shm(size_t size, enum ckteec_shm_dir dir)
// {
// 	TEEC_SharedMemory *shm = NULL;

// 	switch (dir) {
// 	case CKTEEC_SHM_IN:
// 	case CKTEEC_SHM_OUT:
// 	case CKTEEC_SHM_INOUT:
// 		break;
// 	default:
// 		return NULL;
// 	}

// 	shm = calloc(1, sizeof(TEEC_SharedMemory));
// 	if (!shm)
// 		return NULL;

// 	shm->size = size;

// 	if (dir == CKTEEC_SHM_IN || dir == CKTEEC_SHM_INOUT)
// 		shm->flags |= TEEC_MEM_INPUT;
// 	if (dir == CKTEEC_SHM_OUT || dir == CKTEEC_SHM_INOUT)
// 		shm->flags |= TEEC_MEM_OUTPUT;

// 	if (TEEC_AllocateSharedMemory(&ta_ctx.context, shm)) {
// 		free(shm);
// 		return NULL;
// 	}

// 	return shm;
// }

// TEEC_SharedMemory *ckteec_register_shm(void *buffer, size_t size,
// 				       enum ckteec_shm_dir dir)
// {
// 	TEEC_SharedMemory *shm = NULL;

// 	switch (dir) {
// 	case CKTEEC_SHM_IN:
// 	case CKTEEC_SHM_OUT:
// 	case CKTEEC_SHM_INOUT:
// 		break;
// 	default:
// 		return NULL;
// 	}

// 	shm = calloc(1, sizeof(TEEC_SharedMemory));
// 	if (!shm)
// 		return NULL;

// 	shm->buffer = buffer;
// 	shm->size = size;

// 	if (dir == CKTEEC_SHM_IN || dir == CKTEEC_SHM_INOUT)
// 		shm->flags |= TEEC_MEM_INPUT;
// 	if (dir == CKTEEC_SHM_OUT || dir == CKTEEC_SHM_INOUT)
// 		shm->flags |= TEEC_MEM_OUTPUT;

// 	if (TEEC_RegisterSharedMemory(&ta_ctx.context, shm)) {
// 		free(shm);
// 		return NULL;
// 	}

// 	return shm;
// }

// void ckteec_free_shm(TEEC_SharedMemory *shm)
// {
// 	TEEC_ReleaseSharedMemory(shm);
// 	free(shm);
// }

// static bool is_output_shm(TEEC_SharedMemory *shm)
// {
// 	return shm && (shm->flags & TEEC_MEM_OUTPUT);
// }

// CK_RV ckteec_invoke_ta(unsigned long cmd, TEEC_SharedMemory *ctrl,
// 		       TEEC_SharedMemory *io1,
// 		       TEEC_SharedMemory *io2, size_t *out2_size,
// 		       TEEC_SharedMemory *io3, size_t *out3_size)
// {
// 	uint32_t command = (uint32_t)cmd;
// 	TEEC_Operation op;
// 	uint32_t origin = 0;
// 	TEEC_Result res = TEEC_ERROR_GENERIC;
// 	uint32_t ta_rc = PKCS11_CKR_GENERAL_ERROR;

// 	if ((is_output_shm(io2) && !out2_size) ||
// 	    (is_output_shm(io3) && !out3_size))
// 		return CKR_ARGUMENTS_BAD;

// 	memset(&op, 0, sizeof(op));

// 	if (ctrl && !(ctrl->flags & TEEC_MEM_INPUT &&
// 		      ctrl->flags & TEEC_MEM_OUTPUT))
// 		return CKR_ARGUMENTS_BAD;

// 	if (ctrl) {
// 		op.paramTypes |= TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, 0, 0, 0);
// 		op.params[0].memref.parent = ctrl;
// 	} else {
// 		/* TA mandates param#0 as in/out memref for output status */
// 		op.paramTypes |= TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT,
// 						  0, 0, 0);
// 		op.params[0].tmpref.buffer = &ta_rc;
// 		op.params[0].tmpref.size = sizeof(ta_rc);
// 	}

// 	if (io1) {
// 		op.paramTypes |= TEEC_PARAM_TYPES(0, TEEC_MEMREF_WHOLE, 0, 0);
// 		op.params[1].memref.parent = io1;
// 	}

// 	if (io2) {
// 		op.paramTypes |= TEEC_PARAM_TYPES(0, 0, TEEC_MEMREF_WHOLE, 0);
// 		op.params[2].memref.parent = io2;
// 	}

// 	if (io3) {
// 		op.paramTypes |= TEEC_PARAM_TYPES(0, 0, 0, TEEC_MEMREF_WHOLE);
// 		op.params[3].memref.parent = io3;
// 	}

// 	res = TEEC_InvokeCommand(&ta_ctx.session, command, &op, &origin);
// 	switch (res) {
// 	case TEEC_SUCCESS:
// 		/* Get PKCS11 TA return value from ctrl buffer */
// 		if (ctrl) {
// 			if (op.params[0].memref.size == sizeof(ta_rc))
// 				memcpy(&ta_rc, ctrl->buffer, sizeof(ta_rc));
// 		} else {
// 			if (op.params[0].tmpref.size != sizeof(ta_rc))
// 				ta_rc = PKCS11_CKR_GENERAL_ERROR;
// 		}
// 		break;
// 	case TEEC_ERROR_SHORT_BUFFER:
// 		ta_rc = CKR_BUFFER_TOO_SMALL;
// 		break;
// 	case TEEC_ERROR_OUT_OF_MEMORY:
// 		return CKR_DEVICE_MEMORY;
// 	default:
// 		return CKR_GENERAL_ERROR;
// 	}

// 	if (ta_rc == CKR_OK || ta_rc == CKR_BUFFER_TOO_SMALL) {
// 		if (is_output_shm(io2))
// 			*out2_size = op.params[2].memref.size;
// 		if (is_output_shm(io3))
// 			*out3_size = op.params[3].memref.size;
// 	}

// 	return ta_rc;
// }

// static CK_RV ping_ta(void)
// {
// 	TEEC_Operation op = { 0 };
// 	uint32_t origin = 0;
// 	TEEC_Result res = TEEC_SUCCESS;
// 	uint32_t ta_version[3] = { 0 };
// 	uint32_t status = 0;

// 	memset(&op, 0, sizeof(op));
// 	op.params[0].tmpref.buffer = &status;
// 	op.params[0].tmpref.size = sizeof(status);
// 	op.params[2].tmpref.buffer = ta_version;
// 	op.params[2].tmpref.size = sizeof(ta_version);
// 	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT, TEEC_NONE,
// 					 TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE);

// 	res = TEEC_InvokeCommand(&ta_ctx.session, PKCS11_CMD_PING, &op,
// 				 &origin);

// 	if (res != TEEC_SUCCESS ||
// 	    origin != TEEC_ORIGIN_TRUSTED_APP ||
// 	    op.params[0].tmpref.size != sizeof(status) ||
// 	    status != PKCS11_CKR_OK)
// 		return CKR_DEVICE_ERROR;

// 	if (ta_version[0] != PKCS11_TA_VERSION_MAJOR &&
// 	    ta_version[1] > PKCS11_TA_VERSION_MINOR) {
// 		EMSG("PKCS11 TA version mismatch: %"PRIu32".%"PRIu32".%"PRIu32,
// 		     ta_version[0], ta_version[1], ta_version[2]);

// 		return CKR_DEVICE_ERROR;
// 	}

// 	DMSG("PKCS11 TA version %"PRIu32".%"PRIu32".%"PRIu32,
// 	     ta_version[0], ta_version[1], ta_version[2]);

// 	return CKR_OK;
// }

// CK_RV ckteec_invoke_init(void)
// {
// 	TEEC_UUID uuid = PKCS11_TA_UUID;
// 	uint32_t origin = 0;
// 	TEEC_Result res = TEEC_SUCCESS;
// 	CK_RV rv = CKR_CRYPTOKI_ALREADY_INITIALIZED;
// 	const char *login_type_env = NULL;
// 	const char *login_gid_env = NULL;
// 	uint32_t login_method = TEEC_LOGIN_PUBLIC;
// 	void *login_data = NULL;
// 	gid_t login_gid = 0;
// 	unsigned long tmpconv = 0;
// 	char *endp = NULL;
// 	int e = 0;

// 	login_type_env = getenv("CKTEEC_LOGIN_TYPE");

// 	if (login_type_env) {
// 		if (strcmp(login_type_env, "public") == 0) {
// 			login_method = TEEC_LOGIN_PUBLIC;
// 		} else if (strcmp(login_type_env, "user") == 0) {
// 			login_method = TEEC_LOGIN_USER;
// 		} else if (strcmp(login_type_env, "group") == 0) {
// 			login_gid_env = getenv("CKTEEC_LOGIN_GID");
// 			if (!login_gid_env || !strlen(login_gid_env)) {
// 				EMSG("missing CKTEEC_LOGIN_GID");
// 				rv = CKR_ARGUMENTS_BAD;
// 				goto out;
// 			}

// 			login_method = TEEC_LOGIN_GROUP;
// 			tmpconv = strtoul(login_gid_env, &endp, 10);
// 			if (errno == ERANGE || tmpconv > (gid_t)-1 ||
// 			    (login_gid_env + strlen(login_gid_env) != endp)) {
// 				EMSG("failed to convert CKTEEC_LOGIN_GID");
// 				rv = CKR_ARGUMENTS_BAD;
// 				goto out;
// 			}

// 			login_gid = (gid_t)tmpconv;
// 			login_data = &login_gid;
// 		} else {
// 			EMSG("invalid value for CKTEEC_LOGIN_TYPE");
// 			rv = CKR_ARGUMENTS_BAD;
// 			goto out;
// 		}
// 	}

// 	e = pthread_mutex_lock(&ta_ctx.init_mutex);
// 	if (e)
// 		return CKR_CANT_LOCK;

// 	if (ta_ctx.initiated) {
// 		rv = CKR_CRYPTOKI_ALREADY_INITIALIZED;
// 		goto out;
// 	}

// 	res = TEEC_InitializeContext(NULL, &ta_ctx.context);
// 	if (res != TEEC_SUCCESS) {
// 		EMSG("TEEC init context failed\n");
// 		rv = CKR_DEVICE_ERROR;
// 		goto out;
// 	}

// 	res = TEEC_OpenSession(&ta_ctx.context, &ta_ctx.session, &uuid,
// 			       login_method, login_data, NULL, &origin);
// 	if (res != TEEC_SUCCESS) {
// 		EMSG("TEEC open session failed %x from %d\n", res, origin);
// 		TEEC_FinalizeContext(&ta_ctx.context);
// 		rv = CKR_DEVICE_ERROR;
// 		goto out;
// 	}

// 	rv = ping_ta();

// 	if (rv == CKR_OK) {
// 		ta_ctx.initiated = true;
// 	} else {
// 		TEEC_CloseSession(&ta_ctx.session);
// 		TEEC_FinalizeContext(&ta_ctx.context);
// 	}

// out:
// 	e = pthread_mutex_unlock(&ta_ctx.init_mutex);
// 	if (e) {
// 		EMSG("pthread_mutex_unlock: %s", strerror(e));
// 		EMSG("terminating...");
// 		exit(EXIT_FAILURE);
// 	}

// 	return rv;
// }

// CK_RV ckteec_invoke_terminate(void)
// {
// 	CK_RV rv = CKR_CRYPTOKI_NOT_INITIALIZED;
// 	int e = 0;

// 	e = pthread_mutex_lock(&ta_ctx.init_mutex);
// 	if (e) {
// 		EMSG("pthread_mutex_lock: %s", strerror(e));
// 		EMSG("terminating...");
// 		exit(EXIT_FAILURE);
// 	}

// 	if (!ta_ctx.initiated)
// 		goto out;

// 	ta_ctx.initiated = false;
// 	TEEC_CloseSession(&ta_ctx.session);
// 	TEEC_FinalizeContext(&ta_ctx.context);

// 	rv = CKR_OK;

// out:
// 	e = pthread_mutex_unlock(&ta_ctx.init_mutex);
// 	if (e) {
// 		EMSG("pthread_mutex_unlock: %s", strerror(e));
// 		EMSG("terminating...");
// 		exit(EXIT_FAILURE);
// 	}

// 	return rv;
// }

// int main(void)
// {
//     CK_RV rv;

//     printf("Initializing CKTEEC context...\n");
//     rv = ckteec_invoke_init();
//     if (rv != CKR_OK) {
//         printf("ckteec_invoke_init failed: 0x%lx\n", rv);
//         return 1;
//     }
//     printf("CKTEEC context initialized successfully.\n");

//     // Here you could add code to allocate shared memory, invoke TA, etc.
//     // For demonstration, just terminate.

//     printf("Terminating CKTEEC context...\n");
//     rv = ckteec_invoke_terminate();
//     if (rv != CKR_OK) {
//         printf("ckteec_invoke_terminate failed: 0x%lx\n", rv);
//         return 1;
//     }
//     printf("CKTEEC context terminated successfully.\n");

//     return 0;
// }

//CODE-11
// #include <stdio.h>
// #include <string.h>
// #include <tee_client_api.h>
// #include <pkcs11_ta.h>

// // pkcs11 TA UUID: f50ebeef-5a8c-490a-a264-c1b86aa23478
// #define PKCS11_UUID { 0xf50ebeef, 0x5a8c, 0x490a, { 0xa2, 0x64, 0xc1, 0xb8, 0x6a, 0xa2, 0x34, 0x78 } }
// #define PKCS11_TA_CMD_INVOKE 0

// // struct pkcs11_invoke {
// //     uint32_t func_id;
// //     uint32_t session_handle;
// //     uint32_t ctrl_size;
// //     uint32_t data_size;
// // };
// struct __attribute__((__packed__)) pkcs11_invoke {
//     uint32_t func_id;
//     uint32_t session_handle;
//     uint32_t ctrl_size;
//     uint32_t data_size;
// };

// #define PKCS11_CMD_C_GENERATERANDOM 0x00000150

// int main(void) {
//     TEEC_Context ctx;
//     TEEC_Session sess;
//     TEEC_UUID uuid = PKCS11_UUID;
//     TEEC_Result res;
//     TEEC_Operation op;
//     uint32_t origin;

//     struct pkcs11_invoke cmd_buf;
//     uint8_t random_buf[16]; // 16 random bytes

//     printf("Initializing context...\n");
//     res = TEEC_InitializeContext(NULL, &ctx);
//     if (res != TEEC_SUCCESS) {
//         printf("TEEC_InitializeContext failed: 0x%x\n", res);
//         return 1;
//     }

//     printf("Opening session with pkcs11 TA...\n");
//     res = TEEC_OpenSession(&ctx, &sess, &uuid,
//                            TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
//     if (res != TEEC_SUCCESS) {
//         printf("TEEC_OpenSession failed: 0x%x origin 0x%x\n", res, origin);
//         TEEC_FinalizeContext(&ctx);
//         return 1;
//     }

//     printf("Session opened successfully.\n");

//     memset(&cmd_buf, 0, sizeof(cmd_buf));
//     cmd_buf.func_id = PKCS11_CMD_C_GENERATERANDOM;
//     cmd_buf.session_handle = 0;  // 0 because C_GenerateRandom is stateless
//     cmd_buf.ctrl_size = 0;
//     cmd_buf.data_size = sizeof(random_buf);

//     memset(&random_buf, 0, sizeof(random_buf));
//     memset(&op, 0, sizeof(op));

//     op.paramTypes = TEEC_PARAM_TYPES(
//         TEEC_MEMREF_TEMP_INOUT, // command struct
//         TEEC_MEMREF_TEMP_OUTPUT, // random data output
//         TEEC_NONE, TEEC_NONE);

//     op.params[0].tmpref.buffer = &cmd_buf;
//     op.params[0].tmpref.size   = sizeof(cmd_buf);

//     op.params[1].tmpref.buffer = random_buf;
//     op.params[1].tmpref.size   = sizeof(random_buf);

//     printf("Invoking C_GenerateRandom command...\n");
//     res = TEEC_InvokeCommand(&sess, PKCS11_TA_CMD_INVOKE, &op, &origin);
//     if (res != TEEC_SUCCESS) {
//         printf("TEEC_InvokeCommand failed: 0x%x origin 0x%x\n", res, origin);
//     } else {
//         printf("Random data generated:\n");
//         for (int i = 0; i < sizeof(random_buf); i++)
//             printf("%02x ", random_buf[i]);
//         printf("\n");
//     }

//     printf("Closing session...\n");
//     TEEC_CloseSession(&sess);
//     TEEC_FinalizeContext(&ctx);
//     return 0;
// }

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <tee_client_api.h>
#include <pthread.h>

// #define PTA_SCP03_UUID { 0xbe0e5821, 0xe718, 0x4f77, { 0xab, 0x3e, 0x8e, 0x6c, 0x73, 0xa9, 0xc7, 0x35 } }
#define PTA_SCP03_UUID { 0x989850bf, 0x4663, 0x9dcd, { 0x39, 0x4c, 0x07, 0xa4, 0x5f, 0x46, 0x33, 0xd2 } }

#define PTA_CMD_ENABLE_SCP03		0
/* Enable the session using the current keys in the Secure Element */
#define PTA_SCP03_SESSION_CURRENT_KEYS	0
/* Enable the session after replacing the current keys in the Secure Element */
#define PTA_SCP03_SESSION_ROTATE_KEYS	1


struct ta_context {
	pthread_mutex_t lock;
	TEEC_Context context;
	TEEC_Session session;
	TEEC_UUID uuid;
	bool open;
};

static struct ta_context scp03_ta_ctx = {
	.lock = PTHREAD_MUTEX_INITIALIZER,
	.uuid = PTA_SCP03_UUID,
};


static bool open_session(struct ta_context *ctx)
{
	TEEC_Result res = TEEC_SUCCESS;
    printf("open_session called\n");
    printf("ctx->open: %d\n", ctx->open);
	// if (pthread_mutex_lock(&ctx->lock))
	// 	return false;

	if (!ctx->open) {
		res = TEEC_InitializeContext(NULL, &ctx->context);
        if (res) {
            printf("TEEC_InitializeContext failed: 0x%x\n", res);
            return false;
        }
        
		if (!res) {
			res = TEEC_OpenSession(&ctx->context, &ctx->session,
					       &ctx->uuid, TEEC_LOGIN_PUBLIC,
					       NULL, NULL, NULL);
            if (res) {
                printf("TEEC_OpenSession failed: 0x%x\n", res);
                TEEC_FinalizeContext(&ctx->context);
            } else {
                ctx->open = true;
                printf("Session opened successfully.\n");
            }
			// if (!res)
			// 	ctx->open = true;
		}
	}
    else{
        printf("Already open\n");
    }
	return !res;
    
}



unsigned long Test_do_scp03(uint32_t cmd)
{
	TEEC_Operation op = { 0 };

	if (!open_session(&scp03_ta_ctx))
		// return SER_CANT_OPEN_SESSION;
        return 6000;

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, 0, 0, 0);
	op.params[0].value.a = cmd;

	if (TEEC_InvokeCommand(&scp03_ta_ctx.session,
			       PTA_CMD_ENABLE_SCP03, &op, NULL))
		// return SER_ERROR_GENERIC;
        return 6001;

	// return SER_OK;
    return 6002;
}

int main(){
	unsigned long rv = 0;
	rv = Test_do_scp03(PTA_SCP03_SESSION_CURRENT_KEYS);
    printf("Test_do_scp03 returned: %lu\n", rv);
}