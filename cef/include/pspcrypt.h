/**
 * File for native encrypt/decrypt functions
*/

#ifndef PSP_CRYPT_H
#define PSP_CRYPT_H

#include <psptypes.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef __KERNEL__

/**
 * Sends a command to the KIRK encryption/decryption engine.
 *
 * @param inbuf - The input buffer
 * @param outbuf - The output buffer
 * @param cmd - The commands to send to KIRK engine.
 *
 * @returns < 0 on error
 *
 * @attention Needs to link to `pspsemaphore` stub.
 */
int sceUtilsBufferCopy(u8 *inbuf, u8* outbuf, int cmd);

/**
 * Sends a command to the KIRK encryption/decryption engine, by polling.
 *
 * @param inbuf - The input buffer
 * @param outbuf - The output buffer
 * @param cmd - The commands to send to KIRK engine.
 *
 * @returns < 0 on error
 *
 * @attention Needs to link to `pspsemaphore` stub.
 */
int sceUtilsBufferCopyByPolling(u8 *inbuf, u8* outbuf, int cmd);

/**
 * Sends a command to the KIRK encryption/decryption engine.
 *
 * @param inbuf - The input buffer
 * @param insize - The size of input buffer
 * @param outbuf - The output buffer
 * @param outsize - The size of output buffer
 * @param cmd - The commands to send to KIRK engine.
 *
 * @returns < 0 on error
 *
 * @attention Needs to link to `pspsemaphore` stub.
 */
int sceUtilsBufferCopyWithRange(void *inbuf, SceSize insize, void *outbuf, int outsize, int cmd);

/**
 * Sends a command to the KIRK encryption/decryption engine, by polling
 *
 * @param inbuf - The input buffer
 * @param insize - The size of input buffer
 * @param outbuf - The output buffer
 * @param outsize - The size of output buffer
 * @param cmd - The commands to send to KIRK engine.
 *
 * @returns < 0 on error
 *
 * @attention Needs to link to `pspsemaphore` stub.
 */
int sceUtilsBufferCopyByPollingWithRange(void *inbuf, SceSize insize, void *outbuf, int outsize, int cmd);

/**
 * Used for PSAR decoding (1.00 bogus)
 *
 * @param buf - The in/out buffer to decode.
 * @param bufsize - The size of the buffer pointed by buf
 * @param retSize - Pointer to an integer that receives the size of
 * the decoded data.
 *
 * @returns < 0 on error
 *
 * @attention Needs to link to `pspnwman_driver` stub.
*/
int sceNwman_driver_9555D68D(void* buf, SceSize bufsize, int* retSize);

/**
 * Used for PSAR decoding
 *
 * @param buf - The in/out buffer to decode.
 * @param bufsize - The size of the buffer pointed by buf
 * @param retSize - Pointer to an integer that receives the size of
 * the decoded data.
 *
 * @returns < 0 on error
 *
 * @attention Needs to link to `pspmesgd_driver` stub.
*/
int sceMesgd_driver_102DC8AF(void* buf, SceSize bufsize, int* retSize);

#endif // __KERNEL__

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PSP_CRYPT_H