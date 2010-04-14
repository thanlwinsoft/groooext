/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "UnixEnvironmentSetter.hxx"
namespace org { namespace sil { namespace graphite {

    bool UnixEnvironmentSetter::parseFile(const char * var, const char * value)
    {
        const char * home = getenv("HOME");
        if (!home) return false;
        size_t pos = 0;
        size_t varPos = 0;
        size_t varEndPos = 0;
        const static size_t BUFFER_SIZE = 1024;
        char path[BUFFER_SIZE];
        char buffer[BUFFER_SIZE];
        char search[BUFFER_SIZE];
        snprintf(path, BUFFER_SIZE, "%s/.bashrc", home);
        snprintf(search, BUFFER_SIZE, "export %s=", var);
        FILE * bashrc = fopen(path, "r+");
        bool varCorrect = false;
        size_t bytesRead = fread(buffer, 1, BUFFER_SIZE-1, bashrc);
        while (bytesRead)
        {
            buffer[bytesRead] = '\0';// Null terminate
            char * envPos = strstr(buffer, search);
            if (envPos)
            {
                // found the variable
                envPos += strlen(search);
                varPos = pos + (envPos - buffer);
                if (strncmp(envPos + strlen(search), value, strlen(value)) == 0)
                {
                    // found and has correct value
                    varCorrect = true;
                }
                else
                {
                    // found with wrong value
                    varCorrect = false;
                    // find end of line
                    char *endOfLine = envPos;
                    while (*endOfLine != '\n' && *endOfLine != '\0')
                    {
                        endOfLine++;
                    }
                    if (strlen(value) < (endOfLine - envPos))
                    {
                        varEndPos = pos + (endOfLine - buffer);
                    }
                }
            }
            // find position of last complete line
            size_t overlap = 0;
            while (overlap < bytesRead && buffer[bytesRead-overlap] != '\n')
            {
                overlap++;
            }
            strncpy(buffer, buffer + bytesRead - overlap, overlap);
            pos += bytesRead - overlap;
            bytesRead = fread(buffer, 1, BUFFER_SIZE-1-overlap, bashrc);
        }
        if (!varCorrect)
        {
            if (varPos > 0 && varEndPos > varPos)
            {
                fseek(bashrc, varPos, SEEK_SET);
                varPos += fwrite(value, 1, strlen(value), bashrc);
                while (varPos < varEndPos)
                    varPos += fwrite("#", 1, 1, bashrc); // pad to end of line with #
            }
            else
            {
                fseek(bashrc, 0, SEEK_END);
                if (bytesRead > 0 && buffer[bytesRead-1] != '\n')
                    fwrite("\n", 1, 1, bashrc);
                snprintf(search, BUFFER_SIZE, "export %s=%s\n", var, value);
                fwrite(search, 1, strlen(search), bashrc);
            }
        }
        fclose(bashrc);
    }

}}}
