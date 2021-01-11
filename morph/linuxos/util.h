/*!
 * Linux OS specific utilities for morphologica.
 */
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

namespace morph {
    namespace linuxos {

        int parseProcLine (char* line)
        {
            // This assumes that a digit will be found and the line ends in " Kb".
            int i = strlen (line);
            const char* p = line;
            while (*p <'0' || *p > '9') { p++; }
            line[i-3] = '\0';
            i = atoi(p);
            return i;
        }

        //! Get VmSize from /proc/self/status
        int getVmemKb()
        {
            FILE* file = fopen ("/proc/self/status", "r");
            int result = -1;
            char line[128];
            while (fgets (line, 128, file) != NULL) {
                if (strncmp (line, "VmSize:", 7) == 0) {
                    result = parseProcLine (line);
                    break;
                }
            }
            fclose (file);
            return result;
        }

        //! Get VmSize from /proc/self/status
        int getVmSizeKb()
        {
            FILE* file = fopen ("/proc/self/status", "r");
            int result = -1;
            char line[128];
            while (fgets (line, 128, file) != NULL) {
                if (strncmp (line, "VmSize:", 7) == 0) {
                    result = parseProcLine (line);
                    break;
                }
            }
            fclose (file);
            return result;
        }

        //! Get VmData from /proc/self/status
        int getVmDataKb()
        {
            FILE* file = fopen ("/proc/self/status", "r");
            int result = -1;
            char line[128];
            while (fgets (line, 128, file) != NULL) {
                if (strncmp (line, "VmData:", 7) == 0) {
                    result = parseProcLine (line);
                    break;
                }
            }
            fclose (file);
            return result;
        }

        //! Get VmStk from /proc/self/status
        int getVmStkKb()
        {
            FILE* file = fopen ("/proc/self/status", "r");
            int result = -1;
            char line[128];
            while (fgets (line, 128, file) != NULL) {
                if (strncmp (line, "VmStk:", 6) == 0) {
                    result = parseProcLine (line);
                    break;
                }
            }
            fclose (file);
            return result;
        }
    } // namespace linux
} // namespace morph
