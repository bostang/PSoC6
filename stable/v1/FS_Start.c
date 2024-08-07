/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2021  SEGGER Microcontroller GmbH                 *
*                                                                    *
*       www.segger.com     Support: support_emfile@segger.com        *
*                                                                    *
**********************************************************************
*                                                                    *
*       emFile * File system for embedded applications               *
*                                                                    *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product for in-house use.         *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       emFile version: V5.6.1                                       *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
Licensing information
Licensor:                 SEGGER Microcontroller Systems LLC
Licensed to:              Cypress Semiconductor Corp, 198 Champion Ct., San Jose, CA 95134, USA
Licensed SEGGER software: emFile
License number:           FS-00227
License model:            Cypress Services and License Agreement, signed November 17th/18th, 2010
                          and Amendment Number One, signed December 28th, 2020 and February 10th, 2021
Licensed platform:        Any Cypress platform (Initial targets are: PSoC3, PSoC5, PSoC6)
----------------------------------------------------------------------
Support and Update Agreement (SUA)
SUA period:               2010-12-01 - 2022-07-27
Contact to extend SUA:    sales@segger.com
-------------------------- END-OF-HEADER -----------------------------

File    : FS_Start.c
Purpose : Start application for file system.

Additional information:
  Preparations:
    Works out-of-the-box with any storage device.

  Expected behavior:
    The application performs the following actions:
    - Formats the storage device if required
    - Determines the available free space
    - Opens a file
    - Writes some data to the opened file
    - Closes the opened file
    - Determines the available free space
    If the storage device is not formatted the file system formats
    the storage device and generates the following message:
    "Signature invalid or no signature. High-level format required."
    This message is generated each time the sample application is started
    when the system memory is used as storage via the RAMDisk driver.
    The reason for this behavior is that the format information is lost
    as the system memory is reinitialized at startup.

  Sample output:
    Start
    Running sample on ""
      Free space: 7674 MBytes
      Write test data to file \File.txt
      Free space: 7674 MBytes
    Finished
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/
#include <stdio.h>
#include "FS.h"
#include "SEGGER.h"
#include <stdlib.h>

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       MainTask
*/
#ifdef __cplusplus
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
void MainTask(void);
char* ReadFileToString(const char* fileName);
#ifdef __cplusplus
}
#endif
char* ReadFileToString(const char* fileName) {
    FS_FILE* pFile;
    char* buffer = NULL;

    pFile = FS_FOpen(fileName, "r");
    if (pFile != NULL) {
        // Determine the file size
        FS_FSeek(pFile, 0, FS_SEEK_END);
        U32 fileSize = FS_FTell(pFile);
        FS_FSeek(pFile, 0, FS_SEEK_SET);

        buffer = (char*)malloc(fileSize + 1); // Allocate buffer to hold file contents
        if (buffer != NULL) {
            int bytesRead = FS_Read(pFile, buffer, fileSize); // Read the entire file
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0'; // Null terminate the string
            } else {
                free(buffer); // Free the buffer if read failed
                buffer = NULL;
            }
        }
        (void)FS_FClose(pFile);
    }

    return buffer;
}

void MainTask(void) {
    U32 v;
    char ac[256];
    char acFileName[32];
    const char* sVolumeName = "";

    FS_X_Log("Start\n");
    //
    // Initialize file system
    //
    FS_Init();
    //
    // Check if low-level format is required
    //
    (void)FS_FormatLLIfRequired(sVolumeName);
    //
    // Check if volume needs to be high level formatted.
    //
    if (FS_IsHLFormatted(sVolumeName) == 0) {
        FS_X_Log("High-level format\n");
        (void)FS_Format(sVolumeName, NULL);
    }
    SEGGER_snprintf(ac, (int)sizeof(ac), "Running sample on \"%s\"\n", sVolumeName);
    FS_X_Log(ac);
    v = FS_GetVolumeFreeSpaceKB(sVolumeName);
    if (v < 0x8000u) {
        SEGGER_snprintf(ac, (int)sizeof(ac), "  Free space: %lu KBytes\n", v);
    } else {
        v >>= 10;
        SEGGER_snprintf(ac, (int)sizeof(ac), "  Free space: %lu MBytes\n", v);
    }
    FS_X_Log(ac);
    SEGGER_snprintf(acFileName, (int)sizeof(acFileName), "%s\\File.txt", sVolumeName);
    SEGGER_snprintf(ac, (int)sizeof(ac), "  Read test data from file %s\n", acFileName);
    FS_X_Log(ac);

    char* fileContent = ReadFileToString(acFileName);
    if (fileContent != NULL) {
        SEGGER_snprintf(ac, (int)sizeof(ac), "Read data:\n%s\n", fileContent);
        FS_X_Log(ac);
        free(fileContent); // Free the allocated memory
    } else {
        SEGGER_snprintf(ac, (int)sizeof(ac), "ERROR: Could not read file \"%s\"\n", acFileName);
        FS_X_Log(ac);
    }

    v = FS_GetVolumeFreeSpaceKB(sVolumeName);
    if (v < 0x8000u) {
        SEGGER_snprintf(ac, (int)sizeof(ac), "  Free space: %lu KBytes\n", v);
    } else {
        v >>= 10;
        SEGGER_snprintf(ac, (int)sizeof(ac), "  Free space: %lu MBytes\n", v);
    }
    FS_X_Log(ac);
    FS_Unmount(sVolumeName);
    FS_X_Log("Finished\n");
    for (;;) {
        ;
    }
}




/*************************** End of file ****************************/
