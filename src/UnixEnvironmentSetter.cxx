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
#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "UnixEnvironmentSetter.hxx"
namespace org { namespace sil { namespace graphite {

    const char * UnixEnvironmentSetter::BASHRC = ".bashrc";
    /**
    * Parses the .bashrc file looking for the specified variable
    * if it finds, it it modifes its value, if not it appends it to the end
    * of the file. If the variable is embeded in complicated logic involving
    *  if, while etc it will probably fail to do the correct thing.
    * @param var environment variable name
    * @param value environment variable value
    * @return true if .bashrc was successfully modfied
    */
    bool UnixEnvironmentSetter::parseFile(const char * file, const char * var, const char * value)
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
        snprintf(path, BUFFER_SIZE, "%s/%s", home, file);
        snprintf(search, BUFFER_SIZE, "export %s=", var);
        printf("Searching for %s\n", search);
        FILE * bashrc = fopen(path, "r+");
        if (bashrc == NULL)
        {
            bashrc = fopen(path, "w+");
        }
        // Failed to read file
        if (bashrc == NULL) return false;
        bool varCorrect = false;
        size_t bytesRead = fread(buffer, 1, BUFFER_SIZE-1, bashrc);
        buffer[bytesRead] = '\0';// Null terminate
        while (bytesRead > 0)
        {
            // search for existing set statements, there could be more than one, so only the last is
            // important assuming that there is no complicated logic involved
            char * envPos = strstr(buffer, search);
            char *endOfLine = NULL;
            while (envPos)
            {
                // found the variable
#ifdef GROOO_DEBUG
                printf("Existing value for %s found at %u\n", var, pos + (envPos - buffer));
#endif
                // check that it isn't commented
                bool commented = false;
                for (int i = (envPos - buffer); i >=0; i--)
                {
                    if (buffer[i] == '#')
                    {
#ifdef GROOO_DEBUG
                        printf("Ignoring - it is commented\n");
#endif
                        commented = true;
                        break;
                    }
                    else if (buffer[i] == '\n')
                    {
                        break;
                    }
                }
                if (commented == false)
                {
                    envPos += strlen(search);
                    varPos = pos + (envPos - buffer);
                    if (strncmp(envPos, value, strlen(value)) == 0)
                    {
                        // found and has correct value
#ifdef GROOO_DEBUG
                        printf("Value is correct\n");
#endif
                        varCorrect = true;
                    }
                    else
                    {
                        // found with wrong value
                        varCorrect = false;
#ifdef GROOO_DEBUG
                        printf("Value is wrong\n");
#endif
                    }
                    // find end of line
                    endOfLine = envPos;
                    while (*endOfLine != '\n' && *endOfLine != '\0')
                    {
                        endOfLine++;
                    }
                    if (strlen(value) <= (endOfLine - envPos))
                    {
                        varEndPos = pos + (endOfLine - buffer);
#ifdef GROOO_DEBUG
                        printf("Found old value at %d-%d\n", varPos, varEndPos);
#endif
                    }
                }
                else // commented line, skip to end
                {
                    endOfLine = strchr(envPos, '\n');
                }
                if (endOfLine && *endOfLine != '\0')
                    envPos = strstr(endOfLine, search);
                else
                    envPos = NULL;
            }
            // find position of last complete line
            size_t overlap = 0;
            while (overlap < bytesRead && buffer[bytesRead-overlap-1] != '\n')
            {
                overlap++;
            }
            if (overlap > 0)
            {
                strncpy(buffer, buffer + bytesRead - overlap, overlap);
                assert(buffer[0] != '\n');
                assert(buffer[0] == buffer[bytesRead-overlap]);
                buffer[overlap] = '\0';
#ifdef GROOO_DEBUG
                printf("overlap=%d %s\n", overlap, buffer);
#endif
            }
            pos += bytesRead - overlap;
            bytesRead = fread(buffer + overlap, 1, BUFFER_SIZE-1-overlap, bashrc);
            if (bytesRead == 0) break;
            buffer[bytesRead+overlap] = '\0';// Null terminate
            bytesRead += overlap;
        }
        if (!varCorrect)
        {
            if (varPos > 0 && varEndPos > varPos)
            {
#ifdef GROOO_DEBUG
                printf("Overwriting old value\n");
#endif
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
                snprintf(search, BUFFER_SIZE, "export %s=%s\n\n", var, value);
                fwrite(search, 1, strlen(search), bashrc);
            }
        }
        fflush(bashrc);
        fclose(bashrc);
        return true;
    }

}}}