#ifndef __LIBPSARDUMPER_H__
#define __LIBPSARDUMPER_H__

/**
 * Init psar decoding.
 *
 * @param dataPSAR The buffer with the psar data.
 * @param dataOut The buffer that will receive data 1.
 * @param dataOut2 The buffer that will receive data 2.
 *
 * @return `0` on success, `< 0` on error.
*/
int pspPSARInit(u8 *dataPSAR, u8 *dataOut, u8 *dataOut2);

/**
 * Gets the next psar file.
 *
 * @param dataPSAR The buffer with the psar data.
 * @param cbFile The size of the psar.
 * @param dataOut The buffer for internal temporal use.
 * @param dataOut2 The buffer that receives the file data.
 * @param name The buffer that receives file name.
 * @param retSize A pointer to an integer that receives the file size.
 * @param retPos A pointer to an integer that receives the current position within the psar.
 * @param signcheck A pointer to an integer that receives wether the file should be sign checked or not.
 *
 * @return `0` if there is no more files, `1` if there are more files, `< 0` on error.
*/
int pspPSARGetNextFile(u8 *dataPSAR, int cbFile, u8 *dataOut, u8 *dataOut2, char *name, int *retSize, int *retPos);

/**
 * Sets the buffer position.
 *
 * @param position The position to set.
 *
 * @return Always `0`.
*/
int pspPSARSetBufferPosition(int position);

int pspDecryptTable(u8 *buf1, u8 *buf2, int size, int mode);

#endif